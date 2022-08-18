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

void url2FileName(char *webUrl, char *webUrlFile);
void webUrlProcessed(char *href, char *webUrl, char *webUrlFile);
