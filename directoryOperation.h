#ifndef _DIRECTORYOPERATION_H_
#define _DIRECTORYOPERATION_H_

#include <dirent.h>

#ifndef _ERRNO_H_
#define _ERRNO_H_
#include <errno.h>
#endif

#ifndef _SYS_WAIT_H_
#define _SYS_WAIT_H_
#include <sys/wait.h>
#endif

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_
#include <sys/stat.h>
#endif

#ifndef _STDIO_H_
#define _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#define _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#ifndef _UNISTD_H_
#define _UNISTD_H_
#include <unistd.h>
#endif

#include "errDefine.h"
#include "defineUrlFileLen.h"

int removeExistedDirFile(char *dir);
int createDir(char *dir);

#endif
