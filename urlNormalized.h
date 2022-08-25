#ifndef _URLNORMALIZED_H_
#define _URLNORMALIZED_H_

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
#define URL_SAVED_CHARSET "-_.~!*'();:@&=+$,/?%%#[]"

#include "defineUrlFileLen.h"
#include "errDefine.h"

_Bool href2url(char *aHrefStr, char hostname[MAX_URL_SIZE], char cur_url[MAX_CONVERT_URL_SIZE],_Bool protocolTypeHttps);
void urlDeleteRedundantInf(char *aHrefStr);
void webUrlNormalized(char *href, char *webUrl);

#endif
