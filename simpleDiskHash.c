#include "simpleDiskHash.h"

#ifndef _ERRDEFINE_H_
#define _ERRDEFINE_H_
#include "errDefine.h"
#endif

#ifndef _DEFINEURLFILELEN_H_
#define _DEFINEURLFILELEN_H_
#include "defineUrlFileLen.h"
#endif

unsigned hash_func(char *key){

   unsigned hash = OFFSET_BASIS;
   const char *str = key;
   while (*str){
      hash = hash * FNV_PRIME;
      hash = hash ^ *str++;
   }
   hash %= TABLE_SIZE;
   return hash;

}

int fd;
struct flock lock;

//>0: its number
//0:  no found
//<0: error code
int searchHash(char *key, int externalCaller){//caller 0:insertHash 1:other program
   int result = 0;
   _Bool first = 0;
   unsigned index = hash_func(key);
   char pathName[HASH_PATH_LEN + TABLE_SIZE_MAX_LEN + strlen(FILE_EXTENSION_TXT) + 1];//1 for \0
   sprintf(pathName, "%s%d%s\0", HASH_PATH, index, FILE_EXTENSION_TXT);
   fd = open(pathName, O_RDWR|O_CREAT|O_EXCL|O_APPEND, 0644);
   if(fd == -1){
      if(errno == EEXIST){
         fd = open(pathName, O_RDWR|O_APPEND, 0644);
         if(fd == -1){
            result = ERR_FOPEN;
            goto error_handling_search_0;
         }
      }
   }else{//first one
      first = 1;
   }

   /* ---------------- *
    * setting the lock *
    * ---------------- */
   lock.l_type = F_WRLCK;
   lock.l_whence = SEEK_SET;
   lock.l_start = 0;
   lock.l_len = 0;
   int res = fcntl(fd, F_SETLKW, &lock);//get the wrlck
   if(first){
      return result;
   }
   /* ----------------------------- *
    * read file and search(compare) *
    * ----------------------------- */
   char buffer[READ_SIZE + 1];
   int n;
   int nextStart = 0;
   int curStart = 0;
   int numberFile = 0;

   while((n = read(fd, buffer, READ_SIZE)) != 0){//read it and check until nothing can read

      if(n < 0){
         result = ERR_FREAD;
         goto error_handling_search_1;
      }
      curStart = nextStart;//starting index of this time read in file
      nextStart += n;//next start index;
      int bufIndex = 0;
      while(bufIndex < n){
         /* ------------------------------------------------------------------------------ *
          * capture the pre information[%8x\t%4x\t]%s\t about its number and strlen of url *
          * ------------------------------------------------------------------------------ */
         if((bufIndex + FORMAT_PRE_INFO_LEN - 1) >= n){//preinformation is seperate so need to read begin form this data again
            nextStart -= (n - bufIndex);
            break;
         }

         char urlLenStr[MAX_CONVERT_URL_SIZE_LEN + 1];//1 for \0
         strncpy(urlLenStr, buffer + bufIndex, MAX_CONVERT_URL_SIZE_LEN);
         urlLenStr[MAX_CONVERT_URL_SIZE_LEN] = '\0';
         unsigned url_len = strtol(urlLenStr, NULL, 16);
         if((bufIndex + FORMAT_PRE_INFO_LEN + url_len) >= n){//url is seperate so need to read begin form this data again
            nextStart -= (n - bufIndex);
            break;
         }

         //capture the url
         char url[MAX_CONVERT_URL_SIZE + 1];//1 for \0
         strncpy(url, buffer + bufIndex + FORMAT_PRE_INFO_LEN, url_len);
         url[url_len] = '\0';
         //compare
         if(strcmp(key, url) == 0){
            result = ++numberFile;
            goto error_handling_search_1;
         }
         bufIndex += (FORMAT_PRE_INFO_LEN + url_len + 1);
         ++numberFile;
#ifdef _TEST_
         printf("start index:%d hash:%d  len:%d url:%s\n", bufIndex, index, FORMAT_PRE_INFO_LEN + url_len + 1, url);
#endif
      }
      //clear the buffer
      memset(buffer, '\0', READ_SIZE);
      //set the pos for next read
      lseek(fd, nextStart, SEEK_SET);
   }


   if(externalCaller){
error_handling_search_1:
      lock.l_type = F_UNLCK;
      fcntl(fd, F_SETLKW, &lock);
error_handling_search_0:
      close(fd);
   }
   return result;
}

//>0: already enrolled by others
//=0: need to put into url pool
//<0: error appeared
int insertHash(char *key){
   unsigned index = hash_func(key);

   int searchResult = searchHash(key, 0);
   if(searchResult != 0){
      return searchResult;
   }

   //when no found, write into file in format: %8x\t%4x\t%s\t
   char tmpStr[MAX_CONVERT_URL_SIZE + 16];
   char tmpStrFormat[FORMAT_STR_LEN + 1];//1 for \0
   sprintf(tmpStrFormat, "%%0%dx\t%%s\t\0", MAX_CONVERT_URL_SIZE_LEN);
   sprintf(tmpStr, tmpStrFormat, strlen(key), key);

   write(fd, tmpStr, strlen(tmpStr));
   //release lock
   lock.l_type = F_UNLCK;
   fcntl(fd, F_SETLKW, &lock);
   close(fd);
   return searchResult;
}
