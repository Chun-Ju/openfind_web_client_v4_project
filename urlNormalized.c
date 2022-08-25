#include "urlNormalized.h"

_Bool href2url(char *aHrefStr, char hostname[MAX_URL_SIZE], char cur_url[MAX_CONVERT_URL_SIZE],_Bool protocolTypeHttps){//SUCCESS:continue following steps, else SKIP:skip this url
   _Bool complete = 0;//complete:0 need to search for prev layer or can ignore, complete:1 means don't need search for prev layer
   int concat_direction = 0;//neg:prev, 0:means the same ignore, pos:concat after current url
   int which = 0;
prev_justify:
   switch(aHrefStr[which++]){
      case '#':
         concat_direction = 0;
      break;
      case '.':
         if(which < strlen(aHrefStr)){
            switch(aHrefStr[which++]){
               case '/': //after current
                  if(which == 1){
                     complete = 1;
                     concat_direction = 1;
                  }
               break;
               case '.': //while previous justify
                  if(which < strlen(aHrefStr) && aHrefStr[which++] == '/'){
                     concat_direction -= 1;
                     goto prev_justify;
                  }
               break;
            }
         }
      break;
      case '/': //after host name
         complete = 1;
         concat_direction = -200;
      break;
      default:
         if(strlen(aHrefStr) >= strlen(HTTP_PROTOCOL_STR)){
            complete = !strncmp(aHrefStr, HTTP_PROTOCOL_STR, strlen(HTTP_PROTOCOL_STR));
            if(!complete){
               complete = !strncmp(aHrefStr, HTTPS_PROTOCOL_STR, strlen(HTTPS_PROTOCOL_STR));
            }
         }
         if(!complete){//need forward search for 1 layer
            concat_direction = -1;
         }
      break;
   }
   char tmpStrSearch[MAX_CONVERT_URL_SIZE];
   if(!complete){
      if(concat_direction == 0){
         return SKIP;
      }else{//searching prev layer
         strncpy(tmpStrSearch, cur_url, strlen(cur_url) + 1);
         for(int i = concat_direction; i < 0; i++){
            char *pos = strrchr(tmpStrSearch, '/');
            if(!pos){//this is error url so skip it
               return SKIP;
            }
            *pos = '\0';
         }
         sprintf(tmpStrSearch, "%s/%s\0", tmpStrSearch, aHrefStr);
         strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
      }
      return !SKIP;
   }
   //don't need to search forwardly
   switch(concat_direction){//case 0 has already processed completetly
      case 1:
         strncpy(tmpStrSearch, cur_url, strlen(cur_url) + 1);
         strncat(tmpStrSearch, aHrefStr + 2, strlen(aHrefStr) - 1);
         strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
         break;
      case -200:
         if(protocolTypeHttps){
            strncpy(tmpStrSearch, HTTPS_PROTOCOL_STR, strlen(HTTPS_PROTOCOL_STR)+ 1);
         }else{
            strncpy(tmpStrSearch, HTTP_PROTOCOL_STR, strlen(HTTP_PROTOCOL_STR)+ 1);
         }
         strncat(tmpStrSearch, hostname, strlen(hostname) + 1);
         strncat(tmpStrSearch, aHrefStr, strlen(aHrefStr) + 1);
         strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
         break;
   }
   return !SKIP;
}

/* ---------------------------------------------------------------------------------------------- *
 * determine the last part is needed or not, ex:last part of url[https://aa.com/asd/#a] means: #a *
 * and the last part that begins with # or mailto or tel will be ignored.like[https://aa.com/asd/]*
 * ---------------------------------------------------------------------------------------------- */
void urlDeleteRedundantInf(char *aHrefStr){
   char tmpStrSearch[MAX_CONVERT_URL_SIZE] = "\0";
   strncpy(tmpStrSearch, aHrefStr, strlen(aHrefStr) + 1);
   if(tmpStrSearch[strlen(tmpStrSearch) - 1] == '/'){
      tmpStrSearch[strlen(tmpStrSearch) - 1] == '\0';
   }
   char *lastPos = strrchr(tmpStrSearch, '/');//continue above example, lastPos = /#a,so what we want is (lastPos + 1)
   if(lastPos && (lastPos+1) &&(*(lastPos+1) == '#') || (strncmp((lastPos+1), "mailto:", 7) == 0) || (strncmp((lastPos+1), "tel:", 4) == 0) || (strncmp((lastPos+1), "javascript:", 11) == 0)){//if #.. mailto: tel: then skip last part
      *(lastPos+1) = '\0';
   }else if(lastPos && (lastPos + 1)){
      char *ignoredStr = strchr(lastPos + 1, '#');
      if(ignoredStr){
         *(ignoredStr) = '\0';
      }
   }
   strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch)+1);
}

void webUrlNormalized(char *href, char *webUrl){

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
