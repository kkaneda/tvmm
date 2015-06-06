#include "printf.h"
#include "vmexit.h"


void
print_vmexit_exitcode ( enum vmexit_exitcode x )
{
	printf ( "#VMEXIT: ");

	switch ( x ) {
	case VMEXIT_EXCEPTION_PF: printf ( "EXCP (page fault)" ); break;
	case VMEXIT_NPF:          printf ( "NPF (nested-paging: host-level page fault)" ); break;
	case VMEXIT_INVALID:      printf ( "INVALID" ); break;
	default:                  printf ( "%x", ( unsigned long ) x ); break;
	}

	printf ( "\n" );
}
