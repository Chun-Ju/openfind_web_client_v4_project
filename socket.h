#define MAX_WEB_SIZE (0xfffff)
#define MAX_PER_SIZE (0xffff)

#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#define SOCKET_FAIL 0
#define SSL_FAIL NULL

#define STATUS_CODE_LEN 3

#ifndef _URLPROCESSED_H_
#define _URLPROCESSED_H_
#include "urlProcessed.h"
#endif

#ifndef _DEFINEURLFILELEN_H_
#define _DEFINEURLFILELEN_H_
#include "defineUrlFileLen.h"
#endif

#ifndef _SIMPLEDISKHASH_H_
#define _SIMPLEDISKHASH_H_
#include "simpleDiskHash.h"
#endif

char *requestWeb(char *def_url, char *outputDir);
