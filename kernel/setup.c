#include "types.h"
#include "printf.h"
#include "failure.h"
#include "string.h"
#include "page.h"
#include "multiboot.h"
#include "e820.h"
#include "pmem_layout.h"
#include "alloc.h"
#include "cpu.h"
#include "elf.h"
#include "vm.h"
#include "vmm.h"


struct cmdline_option {
	unsigned long vmm_heap_size;
	unsigned long vm_pmem_size;
};

static struct cmdline_option __init
parse_cmdline ( const struct multiboot_info *mbi )
{
	struct cmdline_option opt 
		= { DEFAULT_VMM_HEAP_SIZE, 
		    DEFAULT_VM_PMEM_SIZE };

	if ( ( mbi->flags & MBI_CMDLINE ) && ( mbi->cmdline != 0 ) ) {
		char *cmdline = VIRT ( mbi->cmdline );
		printf ("Command line: %s\n", cmdline );
	}
	
	/* [TODO] */

	return opt;
}

static int __init
has_guest_image ( const struct multiboot_info *mbi )
{
	return ( ( mbi->flags & MBI_MODULES ) && ( mbi->mods_count > 0 ) );
}

static int __init
find_memory_region_for_saving_guest_image ( const struct e820_map *e820, const struct pmem_layout *pml, unsigned long len ) 
{
	int i;
	
	for ( i = 0; i < e820->nr_map; i++ ) {
		const struct e820_entry *p = &e820->map[i];
		
		if ( ( p->type != E820_RAM ) || ( p->size < len ) ) {
			continue;
		}

		if ( ( p->addr < pml->vmm_heap_end ) && ( pml->vmm_heap_end + len < p->addr + p->size ) ) {
			return pml->vmm_heap_end;
		}

		if ( p->addr >= pml->vmm_heap_end ) {
			return p->addr;
		} 
	}

	fatal_failure ( "Not enough memory to stash the guest kernel image.\n" );
	return -1;
}

static void __init
copy_guest_image ( const struct multiboot_info *mbi, const struct e820_map *e820, struct pmem_layout *pml )
{
	if ( ! has_guest_image ( mbi ) ) {
		fatal_failure ( "No guest operating system is specified. Check bootloader configuration.\n");
	}

	const struct module *mod = ( struct module * ) VIRT ( mbi->mods_addr );

	pml->guest_image_size  = mod->mod_end - mod->mod_start;
	pml->guest_image_start = find_memory_region_for_saving_guest_image ( e820, pml, pml->guest_image_size );
	
	memmove ( VIRT ( pml->guest_image_start ), VIRT ( mod->mod_start ), pml->guest_image_size );
}

static void __init 
setup_arch ( const struct multiboot_info *mbi, const struct cmdline_option *opt, struct pmem_layout *pml )
{
	struct e820_map e820;
	setup_memory_region ( &e820, mbi );

	pml->total_pages  = get_nr_pages ( &e820 );
	pml->max_page     = get_max_pfn ( &e820 );
	pml->vmm_heap_end = opt->vmm_heap_size;

	/* [Note] We need move a guest image to elsewhere since the
	 * page allocater may destroy the image */
	copy_guest_image ( mbi, &e820, pml );

	naive_allocator_init ( &e820, pml );

	/* [Note] only first 1 GB of the virtual memory of the VMM is 
	 * mapped to the physical memory of the physical machine.  */

	identify_cpu ( );
}

void __init
start_vmm ( const struct multiboot_info *mbi )
{
	printf ( "\n\n\n\n\n\n\n\n" ); /* [DEBUG] */

	struct cmdline_option opt = parse_cmdline ( mbi );

	struct pmem_layout pml;	
	setup_arch ( mbi, &opt, &pml );

	struct vm vm;
	vm_create ( &vm, ( unsigned long ) VIRT ( pml.guest_image_start ), pml.guest_image_size, opt.vm_pmem_size ); 
	vm_boot ( &vm );
}
