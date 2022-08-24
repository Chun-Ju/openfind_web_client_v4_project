#include "directoryOperation.h"

int removeExistedDirFile(char *dir){
   DIR* dirPtr = opendir(dir);
   if(!dirPtr){
      if(errno == ENOTDIR){//if is file
         char *tmpDir = (char *)calloc(strlen(dir),sizeof(char));
         if(!tmpDir){
            return ERR_MALLOC;
         }
         strncpy(tmpDir, dir , strlen(dir) + 1);
         while(tmpDir[strlen(tmpDir)-1] == '/'){
            tmpDir[strlen(tmpDir)-1] = '\0';//remove the ending /
         }
         remove(tmpDir);
         free(tmpDir);
         return createDir(dir);
      }else{
         return ERR_DOPEN;
      }
   }else{
      struct dirent* entity;
      entity = readdir(dirPtr);
      while(entity != NULL){
         char path[PATH_MAX];
         sprintf(path, "%s%s\0", dir, entity->d_name);
         remove(path);
         entity = readdir(dirPtr);
      }
      closedir(dirPtr);
   }
   return SUCCESS;
}

int createDir(char *dir){
   int ret = mkdir(dir, 0777);
   if(ret == -1){
      ret = errno;
      if(ret == EEXIST){
         ret = removeExistedDirFile(dir);
         if(ret != SUCCESS){
            return ret;
         }
      }else{
#ifdef _TEST_
         printf("MKDIR ERROR: %s\n", strerror(ret));
#endif
         return ERR_MKDIR;
      }
   }
   return SUCCESS;
}
