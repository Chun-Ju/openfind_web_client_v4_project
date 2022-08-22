#include "socket.h"

/* ---------------------------------------------------------- *
 * First we need to make a standard TCP socket connection.    *
 * create_socket() creates a socket & TCP-connects to server. *
 * ---------------------------------------------------------- */
int create_socket(char[], _Bool);
int get_host(char *, char *, int);
char *parsingHerf(char *, char *);
_Bool equalDomain(char *, char *);

char hostname[MAX_URL_SIZE] = "\0";
char curhostname[MAX_URL_SIZE] = "\0";
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
   strncpy(dest_url, def_url, strlen(def_url) + 1);
   get_host(dest_url, host, MAX_URL_SIZE);
   get_host(dest_url, curhostname, MAX_URL_SIZE);

   if(strcmp(hostname, "") != 0){
      if(!equalDomain(hostname, curhostname)){
         return result;
      }
   }
   /* ---------------------------------------------------------- *
    * Add to solve the sub directory of the website. by Felicia  *
    * ---------------------------------------------------------- */
   char *subDirPtr = strstr(def_url, host) + strlen(host);
   char subDir[MAX_URL_SIZE];
   strncpy(subDir, subDirPtr, strlen(subDirPtr));
   subDir[strlen(subDirPtr)] = '\0';
   if(strcmp("", subDirPtr)==0){
      subDir[0] = '/';
      subDir[1] = '\0';
   }
   /* ---------------------------------------------------------- *
    * These function calls initialize openssl for correct work.  *
    * ---------------------------------------------------------- */
   OpenSSL_add_all_algorithms();
   ERR_load_crypto_strings();
   SSL_load_error_strings();

   /* ---------------------------------------------------------- *
    * initialize SSL library and register algorithms             *
    * ---------------------------------------------------------- */
   if(SSL_library_init() < 0){
      goto ssl_fail_error_handle;
   }
   /* ---------------------------------------------------------- *
    * SSLv23_client_method is deprecated function                *
    * Set TLS client hello by andric                             *
    * ---------------------------------------------------------- */
   method = TLS_client_method();

   /* ---------------------------------------------------------- *
    * Try to create a new SSL context                            *
    * ---------------------------------------------------------- */
   if ( (ctx = SSL_CTX_new(method)) == NULL){
      goto ssl_fail_error_handle;
   }
   /* ---------------------------------------------------------- *
    * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
    * ---------------------------------------------------------- */
   SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

   /* ---------------------------------------------------------- *
    * Create new SSL connection state object                     *
    * ---------------------------------------------------------- */
   ssl = SSL_new(ctx);

   /* ---------------------------------------------------------- *
    * Make the underlying TCP socket connection                  *
    * ---------------------------------------------------------- */
   server = create_socket(dest_url, parent);
   if(server == 0){
      goto ssl_fail_error_handle;
   }
   /* ---------------------------------------------------------- *
    * Attach the SSL session to the socket descriptor            *
    * ---------------------------------------------------------- */
   SSL_set_fd(ssl, server);

   /* ---------------------------------------------------------- *
    * Set to Support TLS SNI extension by Andric                 *
    * ---------------------------------------------------------- */
   SSL_ctrl(ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)host);

   /* ---------------------------------------------------------- *
    * Try to SSL-connect here, returns 1 for success             *
    * ---------------------------------------------------------- */
   if ((ret = SSL_connect(ssl)) != 1 ) {
      int err;
      /* ---------------------------------------------------------- *
       * Print SSL-connect error message by andric                  *
       * ---------------------------------------------------------- */
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

   /* ---------------------------------------------------------- *
    * Get the remote certificate into the X509 structure         *
    * ---------------------------------------------------------- */
   cert = SSL_get_peer_certificate(ssl);
   if (cert == NULL){
      goto ssl_fail_error_handle;
   }
   /* ---------------------------------------------------------- *
    * extract various certificate information                    *
    * -----------------------------------------------------------*/
   certname = X509_NAME_new();
   certname = X509_get_subject_name(cert);

   /* ---------------------------------------------------------- *
    * recv the message or the webpage structure by Felicia       *
    * -----------------------------------------------------------*/
   char str[MAX_URL_SIZE + 0x30];
   sprintf(str, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n\0" , subDir, hostname);
   int bytes_w = SSL_write(ssl, str, strlen(str));
   /*if(bytes_w < 0){
     int err = SSL_get_error(ssl, bytes_w);
     printf("%d\n", err);
     }*/
   char printBuf[MAX_WEB_SIZE];
   memset(printBuf, '\0', MAX_WEB_SIZE);
   char buf[MAX_PER_SIZE];
   int bytes = 1;
   while(bytes > 0){
      int retry = 0;
read_again:
      bytes = SSL_read(ssl, buf, sizeof(buf));
      int err = SSL_get_error(ssl, bytes);
      if(bytes < 0){
         if(err == SSL_ERROR_WANT_READ){
            if(retry++ < RECONNECT_LIMIT){
               goto read_again;
            }else{
#ifdef _TEST_
               printf("retry read:%d times so give up.\n", RECONNECT_LIMIT);
#endif
               break;
            }
         }
      }
      buf[bytes] = '\0';
      if((bytes > 0) && (strcmp(buf, "0\r\n\r\n") != 0 && strcmp(buf, "\r\n\r\n") != 0)){
         strncat(printBuf, buf, bytes+1);
      }
   }
   /* --------------------------------------------------------  *
    * parsing the web struct and analyze the message by Felicia *
    * --------------------------------------------------------- */
   char statusCode[STATUS_CODE_LEN+1];//HTTP/1.1 STATUS_CODE ...
   strncpy(statusCode, printBuf + 9, STATUS_CODE_LEN);
   statusCode[STATUS_CODE_LEN] = '\0';
   char nextUrl[MAX_URL_SIZE] = "https://\0";
   _Bool findNext = 0;
   if(strcmp(statusCode, "200") == 0){
      char *CRLF = "\r\n\r\n";
      char *body = strstr(printBuf, CRLF);
      if(!body){
         printf("no web struct found\n");
         goto ssl_fail_error_handle;
      }
      body += strlen(CRLF);

      char *chunkedStr = strstr(printBuf, "Transfer-Encoding: chunked");
      if(chunkedStr){
         if(chunkedStr < body){
            body = strchr(body, '\n');
            body += 1;
            body[strlen(body)-5] = '\0';
         }
      }
      if(body[strlen(body)-4] == '\r'){
         body[strlen(body)-4] = '\0';
      }
      //weburl file
      char pathName[PATH_MAX + TABLE_SIZE + NUM_LEN];
      ret = searchHash(def_url, 1);
      if(ret <= 0){//0 means search before so can skip it
         printf("have some error:%s\n", def_url);
         goto ssl_fail_error_handle;
      }
      sprintf(pathName, "%s%03d%08x\0", outputDir, hash_func(def_url), ret);

      //write and wrlock
      struct flock lock;
      char buffer[1024];
      int fd = open(pathName, O_RDWR|O_CREAT|O_EXCL, 0644);
      if(fd == -1){
         printf("fopen fail: %s %s\n", pathName, def_url);
         goto ssl_fail_error_handle;
      }
      int res = fcntl(fd, F_SETLKW, &lock);
      int n = read(fd, buffer, sizeof(buffer));
      if(n != 0){
         goto ssl_fail_error_handle;
      }
      write(fd, body, strlen(body) + 1);
      close(fd);

      result = parsingHerf(body, outputDir);
   }else{//according status code go to corresponding address
      //catch the location
      char location[MAX_URL_SIZE];
      char *location_p = strstr(printBuf, "Location: ");
      if(location_p){
         char * location_end = strstr(location_p, "\r\n");
         if(location_end){
            findNext = 1;
            int countTmp = location_end - location_p - 10;
            strncpy(location, location_p + 10, countTmp + 1);
            location[countTmp] = '\0';
         }
         if(strcmp(statusCode, "301") == 0){
            strncpy(nextUrl, location, strlen(location) + 1);
         } else if(strcmp(statusCode, "302") == 0){
            strncat(nextUrl, hostname, strlen(hostname) + 1);
            strncat(nextUrl, location, strlen(location) + 1);
         }
      }
   }
   if(findNext){
      char webUrl[MAX_CONVERT_URL_SIZE];
      webUrlProcessed(nextUrl, webUrl);

      char pathName[PATH_MAX + NUM_LEN];
      ret = insertHash(webUrl);
      if(ret > 0){//others has enrolled this url
         findNext = 0;
      }

      if(findNext){
         strncpy(nextUrl, webUrl, strlen(webUrl) + 1);
      }
   }
   /* ---------------------------------------------------------- *
    * Free the structures we don't need anymore                  *
    * -----------------------------------------------------------*/
ssl_fail_error_handle:
   SSL_free(ssl);
   close(server);
   X509_free(cert);
   SSL_CTX_free(ctx);
   if(!findNext){
      return result;
   }else{
      return requestWeb(nextUrl, outputDir, parent);
   }

}


/* ---------------------------------------------------------- *
 *           parsing all the href include in webpage          *
 * ---------------------------------------------------------- */
char* parsingHerf(char * web, char *outputDir){

   char *printBuf = (char *)malloc(MAX_WEB_SIZE * sizeof(char));
   if(!printBuf){
      return NULL;
   }
   char *ahref = "<a href=\"\0";
   char *ahrefEnd = "\"";
   char *ahrefStr;
   while((ahrefStr = strstr(web, ahref))){
      int length = strlen(web) - (int)(ahrefStr-web);
      strncpy(web, ahrefStr + strlen(ahref), length - strlen(ahref));
      web[length-strlen(ahref)] = '\0';

      char *ahrefStrEnd = strstr(web, ahrefEnd);
      if(!ahrefStrEnd){
         continue;
      }
      length = (int)(ahrefStrEnd - web);
      char *aHrefStr = (char *)malloc(MAX_CONVERT_URL_SIZE * sizeof(char));
      strncpy(aHrefStr, web, length);
      aHrefStr[length] = '\0';

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
            if(strlen(aHrefStr) >= 8){//complete and concat == 0 means don't need to search
               complete = !strncmp(aHrefStr, "https://", 8);
            }
            if(!complete){
               complete = 1;
               concat_direction = 2;
            }
            break;
      }

      if(!complete){
         if(concat_direction == 0){
            //printf("many %s\n", aHrefStr);
            continue;
         }else{
            //searching prev layer
            printf("not searching prev layer yet");
         }
      }else{
         char tmpStrSearch[MAX_CONVERT_URL_SIZE];
         switch(concat_direction){//case 0 has already processed completetly
            case 1:
               strncpy(tmpStrSearch, cur_url, strlen(cur_url) + 1);
               strncat(tmpStrSearch, aHrefStr + 2, strlen(aHrefStr) - 1);
               strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
               break;
            case 2:
               strncpy(tmpStrSearch, cur_url, strlen(cur_url) + 1);
               strncat(tmpStrSearch, aHrefStr, strlen(aHrefStr) + 1);
               strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
               break;
            case -200:
               strncpy(tmpStrSearch, "https://\0", 9);
               strncat(tmpStrSearch, hostname, strlen(hostname) + 1);
               strncat(tmpStrSearch, aHrefStr, strlen(aHrefStr) + 1);
               strncpy(aHrefStr, tmpStrSearch, strlen(tmpStrSearch) + 1);
               break;
         }
      }
      char tmpStr[7 + MAX_CONVERT_URL_SIZE];
      char webUrl[MAX_CONVERT_URL_SIZE];
      webUrlProcessed(aHrefStr, webUrl);
      //write to the hash file
      int ret;
      if((ret = insertHash(webUrl)) != 0){
         continue;
      }
      /* TO COMPARE IS IT THE SAME HOST */
      char tmphost[MAX_URL_SIZE];
      get_host(webUrl, tmphost, MAX_URL_SIZE);
      if(!equalDomain(hostname, tmphost)){
         continue;
      }

      sprintf(tmpStr, "%04x\t%s\t\0", strlen(webUrl), webUrl);
      strncat(printBuf, tmpStr, strlen(tmpStr));
      free(aHrefStr);
   }
   return printBuf;

}

/* ---------------------------------------------------------- *
 * create_socket() creates the socket & TCP-connect to server *
 * ---------------------------------------------------------- */
int create_socket(char url_str[], _Bool parent) {

   int sockfd;
   char portnum[6] = "443";
   char proto[6] = "";
   char *tmp_ptr = NULL;
   int  port;
   struct hostent *host;
   struct sockaddr_in dest_addr;
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
      //printf("%s %s\n", url_str, tmpHost);
      /* ---------------------------------------------------------- *
       * Remove the final / from url_str, if there is one           *
       * ---------------------------------------------------------- */
      if (url_str[strlen(url_str)] == '/')
         url_str[strlen(url_str)] = '\0';

      /* ---------------------------------------------------------- *
       * the first : ends the protocol string, i.e. http            *
       * ---------------------------------------------------------- */
      if(strchr(url_str, ':')){
         strncpy(proto, url_str, (strchr(url_str, ':')-url_str));

         /* ---------------------------------------------------------- *
          * the hostname starts after the "://" part                   *
          * ---------------------------------------------------------- */
         strncpy(hostname, strstr(url_str, "://")+3, sizeof(hostname));
      }else{
         strncpy(hostname, url_str, strlen(url_str));
      }
      /* ---------------------------------------------------------- *
       * if the hostname contains a colon :, we got a port number   *
       * ---------------------------------------------------------- */
      if (strchr(hostname, ':')) {
         tmp_ptr = strchr(hostname, ':');
         /* the last : starts the port number, if avail, i.e. 8443 */
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

   /* about timeout by Felicia */
   struct timeval tv;
   tv.tv_sec  = 10;
   tv.tv_usec = 0;
   setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(struct timeval));
   setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
   /* end of timeout */

   /* ---------------------------------------------------------- *
    * Zeroing the rest of the struct                             *
    * ---------------------------------------------------------- */
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
