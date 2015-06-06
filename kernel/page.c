#include "string.h"
#include "failure.h"
#include "printf.h"
#include "page.h"
#include "alloc.h"

static unsigned long 
pg_table_create ( void )
{
	const unsigned long pfn   = alloc_pages ( 1, 1 );
	const unsigned long paddr = pfn << PAGE_SHIFT;

	memset ( ( char * ) VIRT ( paddr ), 0, PAGE_SIZE );

	return paddr;
}

unsigned long 
pml4_table_create ( void )
{
	return pg_table_create ( );
}

static unsigned long 
get_index ( unsigned long vaddr, enum pg_table_level level )
{
	unsigned long shift = 0;
	const unsigned long MASK = ( ( 1 << 9 ) - 1 );

	switch ( level ) {
	case PGT_LEVEL_PML4: shift = 39; break;
	case PGT_LEVEL_PDP:  shift = 30; break;
	case PGT_LEVEL_PD:   shift = 21; break;
	default:             fatal_failure ( "wrong level\n" ); break;
	}

	return ( vaddr >> shift ) & MASK;
}

static union pgt_entry *
get_entry ( unsigned long pg_table_base_vaddr, unsigned long vaddr, enum pg_table_level level )
{
	const unsigned long index = get_index ( vaddr, level );
	return ( union pgt_entry *) ( pg_table_base_vaddr + index * sizeof ( union pgt_entry ) );
}

static int 
entry_is_present ( const union pgt_entry *x )
{
	return x->term.flags & PTTEF_PRESENT;
}

/* [Note] paging with compatibility-mode (long mode with 2-Mbyte page tranlation) is only supported.  */
static void
__mmap ( unsigned long pg_table_base_vaddr, unsigned long vaddr, unsigned long paddr, enum pg_table_level level, int is_user )
{
//	printf ( "__mmap: level=%x, vaddr=%x, paddr=%x.\n", level, vaddr, paddr );

	union pgt_entry *e = get_entry ( pg_table_base_vaddr, vaddr, level );

	if ( level == PGT_LEVEL_PD ) {
		/* For page directory entry */

		e->term.base = paddr >> PAGE_SHIFT_2MB;
		e->term.flags = PTTEF_PRESENT | PTTEF_RW | PTTEF_PAGE_SIZE;
		if ( is_user ) { e->term.flags |= PTTEF_US; }
		return;
	}

	/* For page-map level-4 entry and page-directory-pointer entry */

	if ( ! entry_is_present ( e ) ) {
		const unsigned long paddr = pg_table_create ( );
		e->non_term.base  = paddr >> PAGE_SHIFT;
		e->non_term.flags = PTTEF_PRESENT | PTTEF_RW | PTTEF_US;
	} 

	// pg_table_base で指定されたアドレスから，次のレベルのページを調べる
	const unsigned long next_table_base_vaddr = ( unsigned long ) VIRT ( e->non_term.base << PAGE_SHIFT );
	__mmap ( next_table_base_vaddr, vaddr, paddr, level - 1, is_user ); 
}

void
mmap ( unsigned long pml4_table_base_vaddr, unsigned long vaddr, unsigned long paddr, int is_user )
{
	__mmap ( pml4_table_base_vaddr, vaddr, paddr, PGT_LEVEL_PML4, is_user );
}

/******************************************************/

static unsigned long
__vaddr_to_paddr ( unsigned long pg_table_base_vaddr, unsigned long vaddr, enum pg_table_level level )
{
	union pgt_entry *e = get_entry ( pg_table_base_vaddr, vaddr, level );

	if ( ! entry_is_present ( e ) ) {
		fatal_failure ( "Page table entry is not present.\n" );
	}

	if ( level == PGT_LEVEL_PD ) {

		/* For page directory entry */

		if ( ! ( e->term.flags & PTTEF_PAGE_SIZE ) ) {
			fatal_failure ( "Not 2 Mbyte page size.\n" );
		}

		return ( ( e->term.base << PAGE_SHIFT_2MB ) + ( vaddr & ( ( 1 << PAGE_SHIFT_2MB ) - 1 ) ) );
	}

	const unsigned long next_table_base_vaddr = ( unsigned long ) VIRT ( e->non_term.base << PAGE_SHIFT );
	return __vaddr_to_paddr ( next_table_base_vaddr, vaddr, level - 1 ); 
}

unsigned long 
vaddr_to_paddr ( unsigned long pml4_table_base_vaddr, unsigned long vaddr )
{
	return __vaddr_to_paddr ( pml4_table_base_vaddr, vaddr, PGT_LEVEL_PML4 );
}

/******************************************************/

static void
__print_pg_table ( unsigned long pg_table_base_vaddr, enum pg_table_level level )
{
	int i;

	for ( i = 0; i < 512; i++ ) {
		union pgt_entry *e = ( union pgt_entry *) ( pg_table_base_vaddr + i * sizeof ( union pgt_entry ) );

		if ( ! entry_is_present ( e ) ) {
			continue;
		}

		if ( level == PGT_LEVEL_PD ) {
			printf ( "level=%x, index=%x, base=%x, flags=%x\n" ,
				 level, i, e->term.base, e->term.flags );
		} else {
			printf ( "level=%x, index=%x, base=%x, flags=%x\n" ,
				 level, i, e->non_term.base, e->non_term.flags );
			
			const unsigned long next_table_base_vaddr = ( unsigned long ) VIRT ( e->non_term.base << PAGE_SHIFT );
			__print_pg_table ( next_table_base_vaddr, level - 1 );
		}
	}
}

void 
print_pg_table ( unsigned long pml4_table_base_vaddr )
{
	printf ( "-----------------------------------------\n" );
	__print_pg_table ( pml4_table_base_vaddr, PGT_LEVEL_PML4 );	
	printf ( "-----------------------------------------\n" );
}
