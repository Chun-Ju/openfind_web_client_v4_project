#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef _STDIO_H_
#define _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#include "urlProcessed.h"

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

#ifndef PATH_MAX
#define PATH_MAX 0x1000
#endif
