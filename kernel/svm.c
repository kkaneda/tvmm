#include "types.h"
#include "bitops.h"
#include "string.h"
#include "printf.h"
#include "failure.h"
#include "page.h"
#include "msr.h"
#include "cpufeature.h"
#include "cpu.h"
#include "svm.h"
#include "alloc.h"

/* AMD64 manual Vol. 2, p. 441 */
/* Host save area */
static void *host_save_area;


void *
alloc_host_save_area ( void )
{
	void *hsa;

	unsigned long n  = alloc_pages ( 1, 1 );
	hsa = ( void * ) VIRT ( n << PAGE_SHIFT );
	
	if ( hsa ) {
		memset ( hsa, 0, PAGE_SIZE );
	}
	
	return hsa;
}

void __init
enable_svm ( struct cpuinfo_x86 *c )
{
 	/* Xen does not fill x86_capability words except 0. */
	{
		u32 ecx = cpuid_ecx ( 0x80000001 );
		c->x86_capability[5] = ecx;
	}
    
	if ( ! ( test_bit ( X86_FEATURE_SVME, &c->x86_capability ) ) ) {
		fatal_failure ( "No svm featurel\n" );
		return;
	}
   
	{ /* Before any SVM instruction can be used, EFER.SVME (bit 12
	   * of the EFER MSR register) must be set to 1.  
	   * (See AMD64 manual Vol. 2, p. 439) */
		u32 eax, edx;
		rdmsr ( MSR_EFER, eax, edx );
		eax |= EFER_SVME;
		wrmsr ( MSR_EFER, eax, edx );
	}

	printf ( "AMD SVM Extension is enabled.\n" );

	/* Initialize the HSA */
	{
		u64 phys_hsa;
		u32 phys_hsa_lo, phys_hsa_hi;   

		host_save_area = alloc_host_save_area ( );
		phys_hsa = ( u64 ) PHYS ( host_save_area ); 
		phys_hsa_lo = ( u32 ) phys_hsa;
		phys_hsa_hi = ( u32 ) (phys_hsa >> 32);    
		wrmsr ( MSR_K8_VM_HSAVE_PA, phys_hsa_lo, phys_hsa_hi ); 
	}
}
