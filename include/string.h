#ifndef __STRING_H__
#define __STRING_H__


#include "types.h"

extern void *__memcpy ( void *to, const void *from, size_t len ); 
extern void * memmove ( void * dest, const void *src, size_t count );
extern void * memset ( void *s, int c, size_t count );

extern char * strcpy(char * dest,const char *src);
extern int strcmp ( const char * cs,const char * ct );
extern int strncmp ( const char *cs, const char *ct, size_t count );


#endif /* __STRING_H__ */
