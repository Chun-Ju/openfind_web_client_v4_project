#include "main.h"

#ifndef _URLPROCESSED_H_
#define _URLPROCESSED_H_
#include "urlProcessed.h"
#endif

#include "socket.h"

int processCount = 5;
char *outputDir;
char *urlBuf;

int main(int argc, char *argv[]){
   //pid_t parentPid = getpid();

   if(argc == 4){
      processCount = atoi(argv[3]);
   }else if (argc != 3){
      return ERR_ARGS;
   }

   //create directory
   errno = 0;
   int ret = mkdir(argv[2], 0777);
   if(ret == -1){
      switch(errno){
         case EACCES:
#ifdef _TEST_
            printf("the parent dir doesn't allow write");
#endif
            return ERR_MKDIR_EACCES;
         case EEXIST:
#ifdef _TEST_
            printf("pathname already exists");
#endif
            return ERR_MKDIR_EEXIST;
         case ENAMETOOLONG:
#ifdef _TEST_
            printf("pathname is too long");
#endif
            return ERR_MKDIR_ENAMETOOLONG;
         default:
#ifdef _TEST_
            printf("mkdir");
#endif
            return ERR_MKDIR;
      }
   }
   outputDir = (char *)malloc((strlen(argv[2]) + 1) * sizeof(char));
   if(!outputDir){
      return ERR_MALLOC;
   }
   strncpy(outputDir, argv[2], strlen(argv[2]) + 1);
   char webUrl[3 * MAX_URL_SIZE];
   char webUrlFile[3 * MAX_URL_SIZE];
   webUrlProcessed(argv[1], webUrl, webUrlFile);

   char tmpPathName[3 * MAX_URL_SIZE + PATH_MAX];
   sprintf(tmpPathName, "%s%s\0", outputDir, webUrlFile);
   FILE *fd = fopen(tmpPathName, "w+");
   if(!fd){
      free(outputDir);
      return ERR_FOPEN;
   }
   fclose(fd);

   char *result = requestWeb(webUrl, outputDir);
   fd = fopen(URL_FILE, "w+");
   if(!fd){
      free(outputDir);
      return ERR_FOPEN;
   }
   fprintf(fd, "%s", result);
   fclose(fd);

   free(outputDir);
   return SUCCESS;
}
