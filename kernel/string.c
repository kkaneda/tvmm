#include "string.h"

int
strcmp ( const char *cs, const char *ct )
{
	signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}
	return __res;
}

char *
strcpy ( char *dest, const char *src )
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

int 
strncmp ( const char *cs, const char *ct, size_t count )
{
	signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}
	return __res;
}



/* Only used for special circumstances. Stolen from i386/string.h */ 
static inline void *
__inline_memcpy ( void * to, const void * from, size_t n )
{
	unsigned long d0, d1, d2;
	__asm__ __volatile__(
		"rep ; movsl\n\t"
		"testb $2,%b4\n\t"
		"je 1f\n\t"
		"movsw\n"
		"1:\ttestb $1,%b4\n\t"
		"je 2f\n\t"
		"movsb\n"
		"2:"
		: "=&c" (d0), "=&D" (d1), "=&S" (d2)
		:"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
		: "memory");
	return (to);
}

void *
memmove ( void *dest, const void *src, size_t count )
{
	if ( dest < src ) { 
		__inline_memcpy ( dest, src, count );
	} else {
		char *p = (char *) dest + count;
		char *s = (char *) src + count;
		while ( count-- ) {
			*--p = *--s;
		}
	}
	return dest;
} 

void *
memset(void *s, int c, size_t count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}
