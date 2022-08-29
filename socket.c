#include "socket.h"

/* ---------------------------------------------------------- *
 * First we need to make a standard TCP socket connection.    *
 * create_socket() creates a socket & TCP-connects to server. *
 * ---------------------------------------------------------- */
_Bool write2File(char *, char *, char *, int, char *);
int create_socket(char[], _Bool, _Bool);
int get_host(char *, char *, int);
char *parsingHerf(char *, long, char *, _Bool);
_Bool equalDomain(char *, char *);

char hostname[MAX_URL_SIZE];
char curhostname[MAX_URL_SIZE];
char cur_url[MAX_CONVERT_URL_SIZE];

char *requestWeb(char *def_url, char *outputDir, _Bool parent) {
   memset(cur_url, '\0', MAX_CONVERT_URL_SIZE);
   strncpy(cur_url, def_url, strlen(def_url) + 1);

   char dest_url[MAX_URL_SIZE];
   char host[MAX_URL_SIZE];
   X509 *cert = NULL;
   X509_NAME *certname = NULL;
   const SSL_METHOD *method;
   SSL_CTX *ctx;
   SSL *ssl;
   int server = 0;
   int ret, i;

   char *result = SSL_FAIL;

   _Bool protocolTypeHttps;//0:others(http) 1:https
   if(strncmp(def_url, HTTP_PROTOCOL_STR, strlen(HTTP_PROTOCOL_STR)) == 0){
      protocolTypeHttps = 0;
      printf("still not serve about http server in branch pdf version\n");
      return result;
   }else if(strncmp(def_url, HTTPS_PROTOCOL_STR, strlen(HTTPS_PROTOCOL_STR)) == 0){
      protocolTypeHttps = 1;
   }else{
      return result;
   }

   strncpy(dest_url, def_url, strlen(def_url) + 1);
   get_host(dest_url, hostname, MAX_URL_SIZE);
   get_host(dest_url, curhostname, MAX_URL_SIZE);

   if(strcmp(hostname, "") != 0){
      if(!equalDomain(hostname, curhostname)){
         return result;
      }
   }
   /* ----------------------------------------------- *
    * Add to solve the sub directory of the website.  *
    * ----------------------------------------------- */
   char *subDirPtr = strstr(def_url, host) + strlen(host);
   char subDir[MAX_URL_SIZE];
   strncpy(subDir, subDirPtr, strlen(subDirPtr));
   subDir[strlen(subDirPtr)] = '\0';
   if(strcmp("", subDirPtr)==0){
      subDir[0] = '/';
      subDir[1] = '\0';
   }

   /* ------------------------------------------------------------------- *
    * Connecting with ssl or just make the underlying TCP socket connect  *
    * and send GET request then recv the response message                 *
    * ------------------------------------------------------------------- */
   //initialize
   char str[MAX_URL_SIZE + 0x30];
   sprintf(str, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n\0" , subDir, hostname);
   int bytes_w;
   int retry = 0;

   char printBuf[MAX_WEB_SIZE];
   //char *printBuf = (char *)calloc(MAX_WEB_SIZE, sizeof(char));
   memset(printBuf, '\0', MAX_WEB_SIZE);
   char *contentStart;

   char buf[MAX_WEB_SIZE];
   int bytes = 1;

   int totalBytes = 0;
   _Bool header = FALSE;
   char headerContent[MAX_WEB_SIZE];
   char statusCode[STATUS_CODE_LEN+1];
   char *chunkedStr;
   long long contentLength;
   _Bool HTML = FALSE;
   //https->ssl and TCP socket
   //connect
   OpenSSL_add_all_algorithms();
   ERR_load_crypto_strings();
   SSL_load_error_strings();
   if(SSL_library_init() < 0){
      goto ssl_fail_error_handle;
   }
   method = TLS_client_method();
   if ( (ctx = SSL_CTX_new(method)) == NULL){
      goto ssl_fail_error_handle;
   }
   SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
   ssl = SSL_new(ctx);

   server = create_socket(dest_url, parent, protocolTypeHttps);
   if(server == 0){
      goto ssl_fail_error_handle;
   }

   SSL_set_fd(ssl, server);
   SSL_ctrl(ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)host);
   if ((ret = SSL_connect(ssl)) != 1 ) {
      int err;
      err = SSL_get_error(ssl, ret);
      if(err == SSL_ERROR_SSL){
         ERR_load_crypto_strings();
         SSL_load_error_strings(); // just once
         char msg[1024];
         ERR_error_string_n(ERR_get_error(), msg, sizeof(msg));
         printf("%s %s %s %s\n", msg, ERR_lib_error_string(0), ERR_func_error_string(0), ERR_reason_error_string(0));
      }
      goto ssl_fail_error_handle;
   }
   cert = SSL_get_peer_certificate(ssl);
   if (cert == NULL){
      goto ssl_fail_error_handle;
   }
   certname = X509_NAME_new();
   certname = X509_get_subject_name(cert);

   //send the request message
   retry = 0;
   while(retry < RETRY_LIMIT){
      bytes_w = SSL_write(ssl, str, strlen(str));
      if(bytes_w < 0){
         int err = SSL_get_error(ssl, bytes_w);
         if((err == SSL_ERROR_WANT_WRITE) && (retry++ < RETRY_LIMIT)){
            sleep(1);
         }else{
            goto ssl_fail_error_handle;
         }
      }else{
         break;
      }
   }

   //recv the message or the webpage structure until header finish
   //only do string processing with header msg.
   while(bytes > 0 && !header){
      retry = 0;
      while(retry < RETRY_LIMIT && !header){
         bytes = SSL_read(ssl, buf, sizeof(buf));
         if(bytes > 0){
            memmove(printBuf + totalBytes, buf, bytes);
            totalBytes += bytes;
            char *headerEndPtr = strstr(printBuf, CRLF);
            if(!header && headerEndPtr){//find the whole header msg
               int headerEnd = headerEndPtr - printBuf;
               strncpy(headerContent, printBuf, headerEnd);
               headerContent[headerEnd] = '\0';
               header = TRUE;
               //do header parsing:
               char *statusCodePtr = strchr(headerContent, ' ');
               if(statusCodePtr){
                  statusCodePtr += 1;
               }else{
                  goto ssl_fail_error_handle;
               }
               strncpy(statusCode, statusCodePtr, STATUS_CODE_LEN);
               chunkedStr = strstr(headerContent, CHUNKED);//null : not chunk, other : chunked
               char *strAfterContentLengthStr = strstr(headerContent, CONTENT_LENGTH_STR);
               if(strAfterContentLengthStr){
                  char *contentLengthEndPtr = strstr(strAfterContentLengthStr, "\r\n");
                  if(contentLengthEndPtr){
                     char contentLengthStr[MAX_WEB_SIZE];
                     int length = contentLengthEndPtr - strAfterContentLengthStr - strlen(CONTENT_LENGTH_STR);
                     strncpy(contentLengthStr, strAfterContentLengthStr + strlen(CONTENT_LENGTH_STR), length);
                     contentLengthStr[strlen(contentLengthStr)] = '\0';
                     contentLength = strtol(contentLengthStr,NULL, 10);
                  }else{
                     contentLength = -1;//means no supply
                  }
               }else{
                  contentLength = -1;//means no supply
               }

               char *contentTypeStart = strstr(headerContent, CONTENT_TYPE_STR);
               char textType[MAX_WEB_SIZE];
               if(contentTypeStart){
                  char *contentTypeEnd = strstr(contentTypeStart, "\r\n");
                  if(contentTypeEnd){
                     char contentType[MAX_WEB_SIZE];
                     int length = contentTypeEnd - (contentTypeStart + strlen(CONTENT_TYPE_STR));
                     strncpy(contentType, contentTypeStart + strlen(CONTENT_TYPE_STR), length);
                     contentType[length] = '\0';
                     char *textTypePrefix = strstr(contentType, "text/");
                     if(textTypePrefix){
                        char *textTypePostfix = strchr(textTypePrefix, ';');
                        if(textTypePostfix){
                           strncpy(textType, textTypePrefix, textTypePostfix - textTypePrefix);
                           textType[textTypePrefix - textTypePostfix] = '\0';
                        }else{
                           strncpy(textType ,textTypePrefix, strlen(textTypePrefix));
                        }
                     }
                     if(strcmp(textType, "text/html") == 0 || strcmp(textType, "text/plain") == 0 || strcmp(textType, "text/xml") == 0){
                        HTML = TRUE;
                     }
                  }
               }
               totalBytes -= (headerEnd + strlen(CRLF));
               memmove(printBuf, headerEndPtr + strlen(CRLF), totalBytes);
            }
            break;
         }else if(bytes < 0){
            int err = SSL_get_error(ssl, bytes);
            if((err != SSL_ERROR_WANT_READ) && (retry++ < RETRY_LIMIT)){
               sleep(1);
            }else{
               goto ssl_fail_error_handle;
            }
         }else{
            int err = SSL_get_error(ssl, bytes);
            if(err != SSL_ERROR_ZERO_RETURN){//bytes == 0, and SSL_ERROR_ZERO_RETURN means close normally, others mean error
               goto ssl_fail_error_handle;
            }else{
               break;
            }
         }
      }
   }
   /* ----------------------------------------------------  *
    * wirte the web struct to file  and analyze the message *
    * ----------------------------------------------------- */
   char nextUrl[MAX_URL_SIZE] = HTTPS_PROTOCOL_STR;
   if(!protocolTypeHttps){
      strncpy(nextUrl, HTTP_PROTOCOL_STR, strlen(HTTP_PROTOCOL_STR) + 1);
   }
   _Bool findNext = 0;

   //convert url to file name by combine hash value and order in hashfile,
   //ex:this url's hash = 3, and is the first url in 3.txt, its filename will become 00300000001
   char pathName[PATH_MAX + TABLE_SIZE + NUM_LEN + 1];//1 for \0
   ret = searchHash(def_url, 1);
   if(ret <= 0){//0 means has searched before so can skip it
      printf("have some error:%s\n", def_url);
      return FALSE;
   }
   sprintf(pathName, "%s%03d%08x\0", outputDir, hash_func(def_url), ret);
   if(strcmp(statusCode, "200")!= 0){//according status code go to corresponding address
      //catch the location
      char location[MAX_URL_SIZE];

      char *strAfterLocation = strstr(headerContent, LOCATION);
      if(strAfterLocation){
         char * location_end = strstr(strAfterLocation, "\r\n");
         if(location_end){
            findNext = 1;
            int countTmp = location_end - strAfterLocation - strlen(LOCATION);
            strncpy(location, strAfterLocation + strlen(LOCATION), countTmp + 1);
            location[countTmp] = '\0';
         }
         if(strcmp(statusCode, "301") == 0){
            strncpy(nextUrl, location, strlen(location) + 1);
         }else if(strcmp(statusCode, "302") == 0){
            if(strncmp(location, HTTP_PROTOCOL_STR, strlen(HTTP_PROTOCOL_STR)) == 0 || strncmp(location, HTTPS_PROTOCOL_STR, strlen(HTTPS_PROTOCOL_STR)) == 0){
               strncpy(nextUrl, location, strlen(location) + 1);
            }else{
               strncat(nextUrl, hostname, strlen(hostname) + 1);
               strncat(nextUrl, location, strlen(location) + 1);
            }
         }
      }
   }else{
      //continue receive message
      if(printBuf){
         if(!write2File(outputDir, def_url, printBuf, totalBytes, pathName)){
            goto ssl_fail_error_handle;
         }
      }
      bytes = 1;
      //recv the message or the webpage structure until nothing can read
      while(bytes > 0){
         retry = 0;
         for(int i = 0; i < RETRY_LIMIT; i++){
            bytes = SSL_read(ssl, buf, sizeof(buf));
            if(bytes > 0){
               totalBytes += bytes;
               write2File(outputDir, def_url, buf, bytes, pathName);
               break;
            }else if(bytes < 0){
               int err = SSL_get_error(ssl, bytes);
               if((err != SSL_ERROR_WANT_READ) && (retry++ < RETRY_LIMIT)){
                  sleep(1);
               }else{
                  goto ssl_fail_error_handle;
               }
            }else{
               int err = SSL_get_error(ssl, bytes);
               if(err != SSL_ERROR_ZERO_RETURN){//bytes == 0, and SSL_ERROR_ZERO_RETURN means close normally, others mean error
                  goto ssl_fail_error_handle;
               }else{
                  break;
               }
            }
         }
      }
      result = SSL_SUCCESS;
      //parsing temperorally removed id one page pdf version
      //read file and parsing
      if(HTML){
         //char *readBuffer = (char *)malloc(totalBytes * sizeof(char));
         char readBuffer[MAX_WEB_SIZE];
         if(!readBuffer){
            goto ssl_fail_error_handle;
         }

         FILE *fptr = fopen(pathName, "r");
         if(!fptr){
            //free(readBuffer);
            goto ssl_fail_error_handle;
         }
         fread(readBuffer, totalBytes, 1, fptr);
         fclose(fptr);
         result = parsingHerf(readBuffer, MAX_WEB_SIZE, outputDir, protocolTypeHttps);
         //TODO > MAX_WEB_SIZE  still need process
         if(totalBytes > MAX_WEB_SIZE){
            //special processing
         }
         int fd = open(URL_FILE, O_WRONLY|O_APPEND);
         if(fd == -1){
#ifdef _TEST_
            printf("\nreason: err_open\n\n");
#endif
             goto ssl_fail_error_handle;
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
         //free(readBuffer);
      }
   }

   //response might give the Location info which is the accurate address
   if(findNext){
      char webUrl[MAX_CONVERT_URL_SIZE];
      webUrlNormalized(nextUrl, webUrl);

      char pathName[PATH_MAX + NUM_LEN];
      ret = insertHash(webUrl);
      if(ret > 0){//others has enrolled this url
         findNext = 0;
      }else{
         strncpy(nextUrl, webUrl, strlen(webUrl) + 1);
      }
   }

   /* ---------------------------------------------------------- *
    * Free the structures we don't need anymore                  *
    * -----------------------------------------------------------*/
ssl_fail_error_handle:
   if(protocolTypeHttps){
      SSL_free(ssl);
      X509_free(cert);
      SSL_CTX_free(ctx);
   }
   close(server);
   /* ----------------------------------------------------------------------------- *
    * Return the result and see whether moves to the location response given or not *
    * ----------------------------------------------------------------------------- */
   if(!findNext){
      return result;
   }else{
      return requestWeb(nextUrl, outputDir, parent);
   }

}

_Bool write2File(char *outputDir, char *def_url, char *printBuf, int totalBytes, char *pathName){
   struct flock lock;
   char buffer[MAX_WEB_SIZE];
   int fd = open(pathName, O_RDWR|O_CREAT|O_EXCL|O_APPEND, 0644);
   if(fd == -1){
      int err = errno;
      if(errno == EEXIST){
         fd = open(pathName, O_RDWR|O_APPEND);
      }else{
         printf("fopen fail: %s %s\n", pathName, def_url);
         return FALSE;
      }
   }
   int res = fcntl(fd, F_SETLKW, &lock);
   write(fd, printBuf, totalBytes + 1);
   lock.l_type = F_UNLCK;
   close(fd);
   return TRUE;
}

/* ---------------------------------------------------------- *
 *           parsing all the href include in webpage          *
 * ---------------------------------------------------------- */
char* parsingHerf(char * web, long totalBytes, char *outputDir, _Bool protocolTypeHttps){
   char *printBuf = (char *)calloc(MAX_WEB_SIZE, sizeof(char));
   if(!printBuf){
      return NULL;
   }

   char *tagStart = "<a";
   char *hrefStart = "href";
   char *tagEnd = "\"";
   char *strAfterTag;
   char *strAfterComment;
   while(1){
      //printf("%s\n", web);
find_again:
      /*strAfterTag = strstr(web, tagStart);
      printf("%p\n", strAfterTag);*/
      strAfterTag = memmem(web, totalBytes, tagStart, strlen(tagStart));
      if(!strAfterTag){
         break;
      }
      strAfterComment = memmem(web, totalBytes, HTML_COMMENT_START_STR, strlen(HTML_COMMENT_END_STR));
      if(strAfterComment){
         if(strAfterTag > strAfterComment){
            //sprintf(web, "%s\0", memmem(web, totalBytes, HTML_COMMENT_END_STR, strlen(HTML_COMMENT_END_STR)) + strlen(HTML_COMMENT_END_STR));
            memmove(web, strAfterComment + strlen(HTML_COMMENT_END_STR), totalBytes - (strAfterComment - web));
            goto find_again;
         }
      }
      
      strAfterTag = memmem(strAfterTag, totalBytes - (strAfterTag - web), hrefStart, strlen(hrefStart));
      //strAfterTag = strstr(strAfterTag, hrefStart);
      //strAfterTag = strchr(strAfterTag, '=');
      strAfterTag = memmem(strAfterTag, totalBytes - (strAfterTag - web), "=", 1);

      int length = totalBytes - (int)(strAfterTag - web);
      //strncpy(web, strAfterTag + strlen(tagStart), length - strlen(tagStart));
      memmove(web, strAfterTag + strlen(tagStart), length - strlen(tagStart));
      //web[length-strlen(tagStart)] = '\0';
      char *ahrefStrEnd = memmem(web, totalBytes, tagEnd, strlen(tagEnd));
      if(!ahrefStrEnd){
         continue;
      }
      length = (int)(ahrefStrEnd - web);

      char aHrefStr[MAX_CONVERT_URL_SIZE];
      //strncpy(aHrefStr, web, length);
      memmove(aHrefStr, web, length);
      aHrefStr[length] = '\0';

      if(href2url(aHrefStr, hostname, cur_url, protocolTypeHttps)){
         continue;
      }

      urlDeleteRedundantInf(aHrefStr);

      /* normalized and check is it the same hostname, if not then skip the following step. */
      //URL normalized
      char tmpStr[7 + MAX_CONVERT_URL_SIZE];
      char webUrl[MAX_CONVERT_URL_SIZE];
      webUrlNormalized(aHrefStr, webUrl);

      /* TO COMPARE IS IT THE SAME HOST */
      char tmphost[MAX_URL_SIZE];
      get_host(webUrl, tmphost, MAX_URL_SIZE);
      if(!equalDomain(hostname, tmphost)){
         continue;
      }

      /* Next,insert to the diskhash, if it already exist skip this url. */
      //write to the hash file
      int ret;
      if((ret = insertHash(webUrl)) != 0){
         continue;
      }
      sprintf(tmpStr, "%04x\t%s\t\0", strlen(webUrl), webUrl);
      strncat(printBuf, tmpStr, strlen(tmpStr) + 1);
   }
   return printBuf;

}

/* ---------------------------------------------------------- *
 * create_socket() creates the socket & TCP-connect to server *
 * ---------------------------------------------------------- */
int create_socket(char url_str[], _Bool parent, _Bool protocolTypeHttps) {

   int sockfd;
   char portnum[6] = "443";
   if(!protocolTypeHttps){
      strncpy(portnum, "80\0", 3);
   }
   char proto[6] = "";
   char *tmp_ptr = NULL;
   int  port;
   struct hostent *host;
   struct sockaddr_in dest_addr;

   //record the hostname and domain, user want to search for
   if(parent){
      memset(hostname, '\0', MAX_URL_SIZE);
      //tmphost
      char tmpHost[MAX_URL_SIZE];
      get_host(url_str, tmpHost, MAX_URL_SIZE);
      if(strstr(url_str, tmpHost)){
         int a = (strstr(url_str, tmpHost) + strlen(tmpHost)) - url_str;
         url_str[a]  = '\0';
      }else{
         return SOCKET_FAIL;
      }

      if (url_str[strlen(url_str)] == '/'){//Remove the final / from url_str, if there is one
         url_str[strlen(url_str)] = '\0';
      }
      if(strchr(url_str, ':')){//the first : ends the protocol string, i.e. http
         strncpy(proto, url_str, (strchr(url_str, ':')-url_str));
         //the hostname starts after the "://" part
         strncpy(hostname, strstr(url_str, "://")+3, sizeof(hostname));
      }else{
         strncpy(hostname, url_str, strlen(url_str));
      }
      if (strchr(hostname, ':')){//if the hostname contains a colon :, we got a port number
         tmp_ptr = strchr(hostname, ':');
         //the last : starts the port number, if avail, i.e. 8443
         strncpy(portnum, tmp_ptr+1,  sizeof(portnum));
         *tmp_ptr = '\0';
      }
   }

   port = atoi(portnum);
   if(strcmp(curhostname, hostname) != 0){
      strncpy(hostname, curhostname, strlen(curhostname) + 1);
   }
   if ((host = gethostbyname(hostname)) == NULL) {
      //abort();
      return SOCKET_FAIL;
   }
   /* ---------------------------------------------------------- *
    * create the basic TCP socket                                *
    * ---------------------------------------------------------- */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   dest_addr.sin_family=AF_INET;
   dest_addr.sin_port=htons(port);
   dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);

   //about timeout
   struct timeval tv;
   tv.tv_sec  = 10;
   tv.tv_usec = 0;
   setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(struct timeval));
   setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

   memset(&(dest_addr.sin_zero), '\0', 8);

   tmp_ptr = inet_ntoa(dest_addr.sin_addr);
   /* ---------------------------------------------------------- *
    * Try to make the host connect here                          *
    * ---------------------------------------------------------- */
   connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr));
   return sockfd;

}

int get_host(char *url, char *host, int host_size){

   char *pch;
   pch = strstr(url, "//");
   if (pch == NULL) {
      pch = url;
   }
   else {
      pch = pch + 2;
   }
   strncpy(host, pch, host_size - 1);
   host[host_size - 1] = '\0';

   pch = strchr(host, '/');
   if (pch) {
      *pch = '\0';
   }
   pch = strchr(host, ':');
   if (pch) {
      *pch = '\0';
   }
   return 0;

}

//1:same domain 0:not same domain
_Bool equalDomain(char* dmn1, char *dmn2){

   /* TO COMPARE IS IT THE SAME HOST */
   if(strcmp(dmn1, dmn2) == 0){
      return SAME;
   }
   if(strlen(dmn2) > strlen(dmn1)){
      char *tmphostRoot = strchr(dmn2, '.');
      if(tmphostRoot){
         if(strcmp((tmphostRoot + 1), dmn1) == 0){
            return SAME;
         }
      }
   }else if(strlen(dmn2) < strlen(dmn1)){//dmn2 shorter than dmn1
      char *tmphostRoot = strchr(dmn1, '.');
      if(tmphostRoot){
         if(strcmp((tmphostRoot + 1), dmn2) == 0){
            return SAME;
         }
      }
   }
   return NOT_SAME;

}
