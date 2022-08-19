#ifndef _URLPROCESSED_H_
#define _URLPROCESSED_H_

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

#include "defineUrlFileLen.h"

void webUrlProcessed(char *href, char *webUrl);

#endif
