#include "types.h" 
#include "failure.h"
#include "printf.h"
#include "page.h"
#include "multiboot.h"
#include "e820.h"


static void __init 
add_memory_region ( struct e820_map *e820, u64 start, u64 size, enum e820_type type )
{
	if ( e820->nr_map >= E820_MAX_ENTRIES ) {
		fatal_failure ( "Too many entries in the memory map.\n" );
		return;
	}

	struct e820_entry *p = &e820->map [ e820->nr_map ];
	p->addr = start;
	p->size = size;
	p->type = type;

	e820->nr_map++;
}

static void __init 
e820_print_map ( const struct e820_map *e820 )
{
	int i;

	printf ( "BIOS-provided physical RAM map:\n" );

	for ( i = 0; i < e820->nr_map; i++ ) {
		const struct e820_entry *p = &e820->map [ i ];

		printf (" %x - %x ", (unsigned long long) p->addr, (unsigned long long) ( p->addr + p->size ) );
		switch ( p->type ) {
		case E820_RAM:		printf ( "(usable)\n" ); break;
		case E820_RESERVED: 	printf ( "(reserved)\n" ); break;
		case E820_ACPI:		printf ( "(ACPI data)\n" ); break;
		case E820_NVS:  	printf ( "(ACPI NVS)\n" ); break;
		default:		printf ( "type %x\n", p->type ); break;
		}
	}
}

void __init
setup_memory_region ( struct e820_map *e820, const struct multiboot_info *mbi )
{
	if ( ! ( mbi->flags & MBI_MEMMAP ) ) {
		fatal_failure ( "Bootloader provided no memory information.\n" );
	}

	e820->nr_map = 0;

	unsigned long p = 0;
	while ( p < mbi->mmap_length ) {
		const struct memory_map *mmap = ( struct memory_map *) ( mbi->mmap_addr + p );
		const u64 start = ( (u64) mmap->base_addr_high << 32 ) | (u64) mmap->base_addr_low;
		const u64 size = ( (u64) mmap->length_high << 32 ) | (u64) mmap->length_low;

		add_memory_region ( e820, start, size, mmap->type );

		p += mmap->size + sizeof ( mmap->size );
	}
}

unsigned long __init 
get_nr_pages ( const struct e820_map *e820 ) 
{
	unsigned long n = 0;
	int i;

	for ( i = 0; i < e820->nr_map; i++ ) {
		const struct e820_entry *p = &e820->map [ i ];

		if ( p->type != E820_RAM ) {
			continue;
		}

		n += p->size >> PAGE_SHIFT;
	}

	return n;
}

unsigned long __init 
get_max_pfn ( const struct e820_map *e820 )
{
	unsigned long n = 0;
	int i;
	
	for ( i = 0; i < e820->nr_map; i++ ) {
		const struct e820_entry *p = &e820->map [ i ];

		if ( p->type != E820_RAM ) {
			continue;
		}

		const unsigned long start = PFN_UP ( p->addr );
		const unsigned long end = PFN_DOWN ( p->addr + p->size );
		if ( ( start < end ) && ( end > n ) ) {
			n = end; 
		}
	}
	
	return n;
}

