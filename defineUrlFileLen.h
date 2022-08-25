#ifndef _DEFINEURLFILELEN_H_
#define _DEFINEURLFILELEN_H_

/* ------------------------------------------------------------ *
 * fprintf format "%0[NUM_LEN]x%0[MAX_CONVERT_URL_SIZE_LEN]x%s" *
 * ------------------------------------------------------------ */
//about url
#define MAX_URL_SIZE (0x2000)
#define MAX_CONVERT_URL_SIZE (3*MAX_URL_SIZE)
#define MAX_CONVERT_URL_SIZE_LEN 4

//format Str total long, ex:%04x\t%s\t consists of 8 char, so its str long is 8
#define FORMAT_STR_LEN 8

//format url part len, ex:the url part of %04x\t%s\t is [%04x]\t, so its len is FORMAT_PRE_INFO_LEN + 1(4 + 1 = 5)
#define FORMAT_PRE_INFO_LEN (MAX_CONVERT_URL_SIZE_LEN + 1)

/* -------------------------------- *
 * the path to store disk hash file *
 * -------------------------------- */

#define HASH_PATH "diskhash/"
#define HASH_PATH_LEN 9
//the number of url limit is 0xffffffff(8 digits)
#define NUM_LEN 8
#define FILE_EXTENSION_TXT ".txt"

/* -------------------- *
 * reconnect time limit *
 * -------------------- */
#define RETRY_LIMIT 3


/* -------------------- *
 * protocol type string *
 * -------------------- */
#define HTTPS_PROTOCOL_STR "https://\0"
#define HTTP_PROTOCOL_STR "http://\0"


#endif
