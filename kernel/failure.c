#include "printf.h"


static void 
die ( void ) 
{
	__asm__ __volatile__ ( "ud2a" : : );		
}

void
fatal_failure ( const char *msg )
{
	putstr ( "FATAL FAILURE: " );
	putstr ( msg );

//	__asm__ __volatile__ ( "leaq 0(%%rip), %0" : "=r" (rip) : );	

	die ( );
}
