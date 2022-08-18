/* ------------------------------------------------------------ *
 * fprintf format "%0[NUM_LEN]x%0[MAX_CONVERT_URL_SIZE_LEN]x%s" *
 * ------------------------------------------------------------ */

//about url
#define MAX_URL_SIZE (0x2000)
#define MAX_CONVERT_URL_SIZE (3*MAX_URL_SIZE)
#define MAX_CONVERT_URL_SIZE_LEN 4

//about the len of number for file of url
#define NUM_LEN 8

//format Str total long, ex:%08x\t%04x\t%s\t consists of 13 char, so its str long is 13
#define FORMAT_STR_LEN 13

//format num part len, ex:the num part of %08x\t%04x\t%s\t is [%08x]\t, so its len is NUM_LEN + 1(8 + 1 = 9)
#define FORMAT_NUM_PART_LEN (NUM_LEN + 1)

//format url part len, ex:the url part of %08x\t%04x\t%s\t is [%04x]\t, so its len is MAX_CONVERT_URL_SIZE_LEN + 1(4 + 1 = 5)
#define FORMAT_URL_PART_LEN (MAX_CONVERT_URL_SIZE_LEN + 1)

//format pre information len, ex:the preinformation of %08x\t%04x\t%s\t is [%08x]\t[%04x]\t, so its len is FORMAT_NUM_PART_LEN + FORMAT_URL_LEN(9 + 5 = 14)
#define FORMAT_PRE_INFO_LEN (FORMAT_NUM_PART_LEN + FORMAT_URL_PART_LEN)

/* -------------------------------- *
 * the path to store disk hash file *
 * -------------------------------- */

#define HASH_PATH "diskhash/"
#define HASH_PATH_LEN 9
