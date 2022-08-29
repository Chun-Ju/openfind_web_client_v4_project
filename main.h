#ifndef _MAIN_H_
#define _MAIN_H_

#ifndef _SYS_WAIT_H_
#define _SYS_WAIT_H_
#include <sys/wait.h>
#endif

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_
#include <sys/stat.h>
#endif

#ifndef _STDLIB_H_
#define _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STDIO_H_
#define _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#ifndef _UNISTD_H_
#define _UNISTD_H_
#include <unistd.h>
#endif

#define urlPerProcess 5//the max amount of url expected tp dispatch to per child process

#define OUTPUT_DIR "webpage/"

#include "errDefine.h"
#include "defineUrlFileLen.h"
#include "urlNormalized.h"
#include "simpleDiskHash.h"
#include "socket.h"
#include "directoryOperation.h"

#endif
