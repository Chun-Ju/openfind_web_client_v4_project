#ifndef _ERRDEFINE_H_
#define _ERRDEFINE_H_

#define SUCCESS 0
#define SKIP 1

#define ERR_BASE 0
#define ERR_ARGS (ERR_BASE-1)
#define ERR_SOCKET (ERR_BASE-2)
#define ERR_FOPEN (ERR_BASE-3)
#define ERR_MALLOC (ERR_BASE-4)
#define ERR_FREAD (ERR_BASE-5)
#define ERR_DOPEN (ERR_BASE-6)
#define ERR_MKDIR (ERR_BASE-7)

#define ERR_MKDIR_EACCES (ERR_MKDIR-1)
#define ERR_MKDIR_EEXIST (ERR_MKDIR-2)
#define ERR_MKDIR_ENAMETOOLONG (ERR_MKDIR-3)

#define TRUE 1
#define FALSE 0

//for pdf verison (that don't need parsing only)
#define SSL_SUCCESS "ssl_Success"

#endif
