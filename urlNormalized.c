#include "urlNormalized.h"

void webUrlNormalized(char *href, char *webUrl) {

   webUrl[0] = '\0';
   int last = 0;
   for(int i = 0; i < strlen(href); i++){
      if(!isalnum(href[i]) && !strchr(URL_SAVED_CHARSET, href[i])){
         strncat(webUrl, href + last, i - last);
         webUrl[strlen(webUrl)] = '\0';
         sprintf(webUrl, "%s%%%x\0", webUrl, href[i]);
         last = i + 1;
      }
   }

   if(last < strlen(href)){//for lefting string
      strncat(webUrl, href + last, strlen(href) - last);
   }

}
