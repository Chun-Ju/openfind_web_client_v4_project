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

#define URL_FILE "url.txt\0"//for saving the file of url's order to dispatch
#define urlPerProcess 5//the max amount of url expected tp dispatch to per child process

#define PATH_MAX 0x1000
