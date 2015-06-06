#ifndef __PRINTF_H__
#define __PRINTF_H__


#include <stdarg.h>
#include "types.h"

extern void putstr ( const char *s );
extern void printf ( const char *fmt, ... );
extern void print_binary ( char *p, size_t len );


#endif /* __PRINTF_H__ */
