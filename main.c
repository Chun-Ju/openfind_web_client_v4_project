#include "main.h"

int processCount = 5;
char *outputDir;
char *urlBuf;

//error code of child process cannot exceed -256
int forkProcess(int time){
   if(time == 0){
      return SUCCESS;
   }
   if(fork() != 0){//parent 負責生產 並決定讀那一行數
      time--;
      return forkProcess(time);
   }else{ //all need to see once
      for(int i = 0; i < urlPerProcess; i++){ //read the whole page of web and download it
         char *tmpUrlStr = urlBuf + ((processCount - time) * urlPerProcess + i) * MAX_CONVERT_URL_SIZE;
         if(strcmp(tmpUrlStr, "") == 0){
            break;
         }
         char *result = requestWeb(tmpUrlStr, outputDir, 0);
         if(!result){
            continue;
         }

         int fd = open(URL_FILE, O_WRONLY|O_APPEND);
         if(fd == -1){
#ifdef _TEST_
            printf("\nreason: err_open\n\n");
#endif
            exit(ERR_FOPEN);
         }
         struct flock lock;
         char buffer[1024];
         lock.l_len = 0;
         lock.l_pid = getpid();
         lock.l_start = 0;
         lock.l_type = F_WRLCK;
         lock.l_whence = SEEK_SET;
         //NEED TO BLOCKING UNTIL CAN WRITE
         fcntl(fd, F_SETLKW, &lock);
         write(fd, result, strlen(result));

         lock.l_type = F_UNLCK;
         fcntl(fd, F_SETLK, &lock);

         close(fd);
         free(result);

      }
      exit(SUCCESS);
   }
}

int readUrlFromFile(FILE *fd){
   char *urlBufTmp = (char *)malloc(processCount * urlPerProcess * MAX_CONVERT_URL_SIZE * sizeof(char));
   if(!urlBufTmp){
      return ERR_MALLOC;
   }
   int count = 0;
   for(count = 0; count < processCount * urlPerProcess; count++){
      int tmpCount;
      if(fscanf(fd, "%x\t", &tmpCount) == EOF){//end of file
         break;
      }

      char url[MAX_CONVERT_URL_SIZE];
      fread(url, tmpCount + 1, 1, fd);
      url[tmpCount] = '\0';
      strncpy(urlBufTmp + count * MAX_CONVERT_URL_SIZE, url, tmpCount + 1);
   }
   const int diffCount = count % processCount;
   //if == 0 , represent everyone has the same amount to count
   //else means the first [diffcount] will more one than others
   int which = 0;
   //notice: the first [diffcount] will be [urlPerProcessCount] + 1
   const int urlPerProcessCount_tmp = count / processCount;
   for(int i = 0; i < processCount && which < count; i++){
      int i_count = urlPerProcessCount_tmp;
      if(diffCount != 0 && (i < diffCount)){
         i_count++;
      }
      for(int j = 0; j < i_count; j++){
         char *urlBufPointer = urlBuf + (i * urlPerProcess + j) * MAX_CONVERT_URL_SIZE;
         char *urlBufTmpPointer = urlBufTmp + which * MAX_CONVERT_URL_SIZE;
         int urlBufTmpPointer_len = strlen(urlBufTmpPointer);
         strncpy(urlBufPointer, urlBufTmp + which * MAX_CONVERT_URL_SIZE, urlBufTmpPointer_len+1);
         if(++which >= count){
            break;
         }
      }
   }
   free(urlBufTmp);
   return SUCCESS;
}

int createDir(char *dir){
   int ret = mkdir(dir, 0777);
   if(ret == -1){
      ret = errno;
#ifdef _TEST_
      printf("MKDIR ERROR: %s\n", strerror(ret));
#endif
      return ERR_MKDIR;
   }
   return SUCCESS;
}

int main(int argc, char *argv[]){

   if(argc < 3){
      return ERR_ARGS;
   }else if(argc == 4){
      processCount = atoi(argv[3]);
   }

   if(strcmp(argv[2], "") == 0){
      outputDir = (char *)malloc((strlen(OUTPUT_DIR) + 1) * sizeof(char));
      if(!outputDir){
         return ERR_MALLOC;
      }
      strncpy(outputDir, OUTPUT_DIR, strlen(OUTPUT_DIR) + 1);
   }else{
      outputDir = (char *)malloc((strlen(argv[2]) + 1) * sizeof(char));
      if(!outputDir){
         return ERR_MALLOC;
      }
      strncpy(outputDir, argv[2], strlen(argv[2]));
   }
   outputDir[strlen(outputDir)] = '\0';

   int ret;
   if((ret = createDir(outputDir)) != SUCCESS){
      free(outputDir);
      return ret;
   }
   if((ret = createDir(HASH_PATH)) != SUCCESS){
      free(outputDir);
      return ret;
   }

   if(strcmp(argv[1], "") == 0){//empty url is quite serious, so retrun with error arguments
      return ERR_ARGS;
   }
   char webUrl[MAX_CONVERT_URL_SIZE];
   webUrlProcessed(argv[1], webUrl);
   insertHash(webUrl);

   char *result;
   for(int i = 0; i < RETRY_LIMIT; i++){
      result = requestWeb(webUrl, outputDir, 1);
      if(result){
         break;
      }
      if(i == RETRY_LIMIT - 1){
         return ERR_SOCKET;
      }
   }
   FILE *fd = fopen(URL_FILE, "w+");
   if(!fd){
      free(outputDir);
      return ERR_FOPEN;
   }
   fprintf(fd, "%s", result);
   fclose(fd);
   //read and dispatch
   urlBuf = (char *)malloc(processCount * urlPerProcess * MAX_CONVERT_URL_SIZE * sizeof(char));
   if(!urlBuf){
      free(outputDir);
      return ERR_MALLOC;
   }
   int lastPos = 0;
   while(1){
      memset(urlBuf, '\0', processCount * urlPerProcess * MAX_CONVERT_URL_SIZE * sizeof(char));
      fd = fopen(URL_FILE, "r");
      if(!fd){
         free(urlBuf);
         free(outputDir);
         return ERR_FOPEN;
      }
      fseek(fd, lastPos, SEEK_SET);
      ret = readUrlFromFile(fd);
      if(ret != SUCCESS){
         free(urlBuf);
         fclose(fd);
         free(outputDir);
         return ret;
      }
      lastPos = ftell(fd);
      fclose(fd);
      //call the child process to work
      forkProcess(processCount);
      int status;
      while(wait(&status) > 0){
         if(status < 0){ //child process過程中有error發生
            free(urlBuf);
            free(outputDir);
            return status/256;//error code的設定絕對不可小於-256
         }
      }
      //check
      fd = fopen(URL_FILE, "r");
      if(!fd){
         free(urlBuf);
         free(outputDir);
         return ERR_FOPEN;
      }
      fseek(fd, 0, SEEK_END);
      if(lastPos == ftell(fd)){//already read to the end
#ifdef _TEST_
         printf("it's the end, position is: %d\n", lastPos);
#endif
         break;
      }
      fclose(fd);
   }
   free(urlBuf);
   free(outputDir);
   return SUCCESS;
}
