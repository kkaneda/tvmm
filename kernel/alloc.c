#include "string.h"
#include "failure.h"
#include "printf.h"
#include "page.h"
#include "pmem_layout.h"
#include "e820.h"


enum {
	ALLOC_BITMAP_SHIFT = 6 /* 1 << ALLOC_BITMAP_SHIFT  
				  = 1 << 6 
				  = 64 
				  = sizeof ( unsigned long ) * 8 */
};

struct naive_allocator  { 
	unsigned long *alloc_bitmap;
	unsigned long max_page;
};

static struct naive_allocator naive_allocator;


static inline unsigned long
get_alloc_bitmap_idx ( unsigned long pfn ) 
{
	return ( pfn >> ALLOC_BITMAP_SHIFT );
}

static inline unsigned long
get_alloc_bitmap_offset ( unsigned long pfn ) 
{
	return ( pfn & ( ( 1 << ALLOC_BITMAP_SHIFT ) - 1 ) );
}

static inline int 
allocated_in_map ( const struct naive_allocator *nalloc, unsigned long pfn ) 
{
	const unsigned long *tbl   = nalloc->alloc_bitmap;
	const unsigned long idx    = get_alloc_bitmap_idx ( pfn );
	const unsigned long offset = get_alloc_bitmap_offset ( pfn );
	
	return !! ( tbl [ idx ] & ( 1UL << offset ) );
}

static void 
map_alloc ( struct naive_allocator *nalloc, unsigned long first_page, unsigned long nr_pages )
{
	unsigned long *tbl = nalloc->alloc_bitmap;
	unsigned long start_off, end_off, curr_idx, end_idx;

//	printf ( "alloc: pfn=%x, size=%x\n", first_page, nr_pages ); /* [DEBUG] */
	
	curr_idx  = get_alloc_bitmap_idx ( first_page );
	start_off = get_alloc_bitmap_offset ( first_page );
	end_idx   = get_alloc_bitmap_idx ( first_page + nr_pages );
	end_off   = get_alloc_bitmap_offset ( first_page + nr_pages );
	
	if ( curr_idx == end_idx ) {
		/* set all n-th bits s.t. start_off <= n < end_off */
		tbl [ curr_idx ] |= ( ( ( 1UL << end_off ) - 1 ) &   /*  (1<<n)-1 sets all bits < n.  */
				      ( - ( 1UL << start_off ) ) );  /*  -(1<<n)  sets all bits >= n.  */
	} else  {
		tbl [ curr_idx ] |= - ( 1UL << start_off );
		for ( curr_idx += 1; curr_idx < end_idx; curr_idx++ ) {
			tbl [ curr_idx ] = ~0UL;
		}
		tbl [ curr_idx ] |= ( 1UL << end_off ) - 1;
	}
}

static void 
map_free ( struct naive_allocator *nalloc, unsigned long first_page, unsigned long nr_pages )
{
	unsigned long *tbl = nalloc->alloc_bitmap;
	unsigned long start_off, end_off, curr_idx, end_idx;

//	printf ( "free: pfn=%x, size=%x\n", first_page, nr_pages );

	curr_idx  = get_alloc_bitmap_idx ( first_page );
	start_off = get_alloc_bitmap_offset ( first_page );
	end_idx   = get_alloc_bitmap_idx ( first_page + nr_pages );
	end_off   = get_alloc_bitmap_offset ( first_page + nr_pages );

	if ( curr_idx == end_idx ) {
		tbl [ curr_idx ] &= - ( 1UL << end_off ) | ( ( 1UL << start_off ) - 1 );
	} else {
		tbl [ curr_idx ] &= ( 1UL << start_off ) - 1;
		for ( curr_idx += 1; curr_idx < end_idx; curr_idx++ ) {
			tbl [ curr_idx ] = 0;
		}
		tbl [ curr_idx ] &= - ( 1UL << end_off );
	}
}

static void 
__init_alloc_bitmap ( struct naive_allocator *nalloc, unsigned long _start, unsigned long _end )
{
	const unsigned long start = PAGE_UP ( _start );
	const unsigned long end   = PAGE_DOWN ( _end );

	if ( end <= start ) {
		return;
	}
	
	map_free ( nalloc, start >> PAGE_SHIFT, ( end - start ) >> PAGE_SHIFT );
}

static void 
init_alloc_bitmap ( const struct e820_map *e820, struct naive_allocator *nalloc, const struct pmem_layout *pml )
{
	int i;
		
	for ( i = 0; i < e820->nr_map; i++ ) {
		const struct e820_entry *p = &e820->map [ i ];
		
		if ( p->type != E820_RAM ) {
			continue;
		}
		
		/* Initialize boot heap */
		unsigned long s = p->addr;
		unsigned long e = s + p->size;
		
		/* Skip vmm heap and guest image. */
		if ( s < pml->vmm_heap_end ) {
			s = pml->vmm_heap_end;
		}

		const unsigned long guest_image_end = pml->guest_image_start + pml->guest_image_size;
		if ( ( s < guest_image_end ) && ( e > pml->guest_image_start ) ) {
			s = guest_image_end;
		}

		__init_alloc_bitmap ( nalloc, s, e );
	}
}

void __init
naive_allocator_init ( const struct e820_map *e820, struct pmem_layout *pml )
{
	extern unsigned long _end; /* standard ELF symbol */
	const unsigned long alloc_bitmap_start = PAGE_UP ( PHYS ( &_end ) );

	/* Allocate space for the allocation bitmap. Include an extra longword
	 * of padding for possible overrun in map_alloc and map_free. */
	const unsigned long bytes = pml->max_page / 8;
	const unsigned long bitmap_size = PAGE_UP ( bytes + sizeof ( unsigned long ) );
	pml->vmm_heap_start = alloc_bitmap_start + bitmap_size;
	if ( pml->vmm_heap_start >= pml->vmm_heap_end ) {
		fatal_failure ( "No heap space.\n" );
	}
	
	struct naive_allocator *nalloc = &naive_allocator;
	{
		nalloc->alloc_bitmap = ( unsigned long * ) VIRT ( alloc_bitmap_start );
		nalloc->max_page = pml->max_page;

		/* All allocated by default. */
		memset ( nalloc->alloc_bitmap, ~0, bitmap_size );
	}

	init_alloc_bitmap ( e820, nalloc, pml );
}

static int
is_free_contiguous_region ( const struct naive_allocator *nalloc, unsigned long pfn, unsigned long nr_pfns )
{
	unsigned long i;

	for ( i = 0; i < nr_pfns; i++ ) {
		if ( allocated_in_map ( nalloc, pfn + i ) ) {
			return 0;
		}
	}
	return 1;
}

static unsigned long 
alloc_boot_pages ( unsigned long nr_pfns, unsigned long pfn_align )
{
	struct naive_allocator *nalloc = &naive_allocator;
	unsigned long i;

	for ( i = 0; i + nr_pfns < nalloc->max_page; i += pfn_align )
	{
		if ( is_free_contiguous_region ( nalloc, i, nr_pfns ) ) {
			map_alloc ( nalloc, i, nr_pfns );
			return i;
		}
	}

	fatal_failure ( "alloc_boot_pages\n" );
	return 0;
}

unsigned long 
alloc_pages ( unsigned long nr_pfns, unsigned long pfn_align )
{
	return alloc_boot_pages ( nr_pfns, pfn_align );
}
