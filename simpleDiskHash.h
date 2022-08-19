#ifndef _STDIO_H_
#define _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#define _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _UNISTD_H_
#define _UNISTD_H_
#include <unistd.h>
#endif

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#ifndef _FCNTL_H_
#define _FCNTL_H_
#include <fcntl.h>
#endif

#ifndef _ERRNO_H_
#define _ERRNO_H_
#include <errno.h>
#endif

#define TABLE_SIZE 500
#define TABLE_SIZE_MAX_LEN 3//in this case table_size max : 499,so its max_len is 3
#define FNV_PRIME 16777619
#define OFFSET_BASIS 2166136261

#define K 1024
#define READ_SIZE (16*K)

#ifndef _ERRDEFINE_H_
#define _ERRDEFINE_H_
#include "errDefine.h"
#endif

#ifndef _DEFINEURLFILELEN_H_
#define _DEFINEURLFILELEN_H_
#include "defineUrlFileLen.h"
#endif

unsigned hash_func(char *key);
int searchHash(char *key, int externalCaller);
int insertHash(char *key);
