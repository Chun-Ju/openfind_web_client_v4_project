#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

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

#define URL_FILE "url.txt\0"//for saving the file of url's order to dispatch
#define urlPerProcess 5//the max amount of url expected tp dispatch to per child process

#define OUTPUT_DIR "webpage/"

#include "errDefine.h"
#include "defineUrlFileLen.h"
#include "urlProcessed.h"
#include "simpleDiskHash.h"
#include "socket.h"

#endif
