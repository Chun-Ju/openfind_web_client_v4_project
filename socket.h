#ifndef _SOCKET_H_
#define _SOCKET_H_

#define MAX_WEB_SIZE (0xfffff)
#define MAX_PER_SIZE (0xffff)

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#ifndef _UNISTD_H_
#define _UNISTD_H_
#include <unistd.h>
#endif

#ifndef _FCNTL_H_
#define _FCNTL_H_
#include <fcntl.h>
#endif

#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#define SOCKET_FAIL 0
#define SSL_FAIL NULL

#define SAME 1
#define NOT_SAME 0

#define STATUS_CODE_LEN 3

#include "urlNormalized.h"
#include "defineUrlFileLen.h"
#include "simpleDiskHash.h"

char *requestWeb(char *def_url, char *outputDir, _Bool parent);

#endif
