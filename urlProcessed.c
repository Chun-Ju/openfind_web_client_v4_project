#include "urlProcessed.h"

#ifndef _DEFINEURLFILELEN_H_
#define _DEFINEURLFILELEN_H_
#include "defineUrlFileLen.h"
#endif

void charConvertHex(char *originalHref, char *href, int which[MAX_URL_SIZE], int count){
   int last = 0;
   href[0] = '\0';
   for(int i = 0; i < count; i++){ // start to processing
      strncat(href, originalHref + last, which[i] - last);
      href[strlen(href)] = '\0';
      sprintf(href, "%s%%%x\0", href, originalHref[which[i]]);
      last = which[i] + 1;
   }
   if(last < strlen(originalHref)){
      strncat(href, originalHref + last, which[count - 1] - last);
   }
}

void webUrlProcessed(char *href, char *webUrl) {
   int hrefProcessedIndex[MAX_URL_SIZE];
   int hrefProcessedCount = 0;

   for(int i = 0; i < strlen(href); i++){
      if(!isalnum(href[i]) && !strchr(URL_SAVED_CHARSET, href[i])){
         hrefProcessedIndex[hrefProcessedCount++] = i;
      }
   }

   if(hrefProcessedCount == 0){//nothing need to be converted
      strncpy(webUrl, href, strlen(href) + 1);
   }else{
      charConvertHex(href, webUrl, hrefProcessedIndex, hrefProcessedCount);
   }
}
