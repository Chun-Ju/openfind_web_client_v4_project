#include <ctype.h>

#ifndef _STDIO_H_
#define _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STRING_H_
#define _STRING_H_
#include <string.h>
#endif

#define URL_SAVED_CHARSET_COUNT 19
#define URL_SAVED_CHARSET "-. ~!*'();:@&=+$,/?%%#[]"

#ifndef  MAX_URL_SIZE
#define MAX_URL_SIZE 0x2000
#define MAX_CONVERT_URL_SIZE (3*MAX_URL_SIZE)
#endif

void url2FileName(char *webUrl, char *webUrlFile);
void webUrlProcessed(char *href, char *webUrl, char *webUrlFile);
