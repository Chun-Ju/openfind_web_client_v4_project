#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#define URL_FILE "url.txt"//for saving the file of url's order to dispatch
#define urlPerProcess 5//the max amount of url expected tp dispatch to per child process

#define SUCCESS 0
#define ERR_BASE 0
#define ERR_ARGS (ERR_BASE-1)
#define ERR_SOCKET (ERR_BASE-2)
#define ERR_FOPEN (ERR_BASE-3)
#define ERR_MALLOC (ERR_BASE-4)
#define ERR_FREAD (ERR_BASE-5)
#define ERR_MKDIR (ERR_BASE-6)

#define ERR_MKDIR_EACCES (ERR_MKDIR-1)
#define ERR_MKDIR_EEXIST (ERR_MKDIR-2)
#define ERR_MKDIR_ENAMETOOLONG (ERR_MKDIR-3)

#ifndef MAX_URL_SIZE
#define MAX_URL_SIZE 0x2000
#endif

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
   outputDir = (char *)malloc(strlen(argv[2] + 1) * sizeof(char));
   if(!outputDir){
      return ERR_MALLOC;
   }
   strncpy(outputDir, argv[2], strlen(argv[2] + 1));

   free(outputDir);
   return SUCCESS;
}
