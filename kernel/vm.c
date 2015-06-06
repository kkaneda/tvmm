#include "string.h"
#include "printf.h"
#include "failure.h"
#include "page.h"
#include "multiboot.h"
#include "system.h"
#include "msr.h"
#include "elf.h"
#include "vm.h"
#include "svm.h"
#include "alloc.h"
#include "vmexit.h"
#include "vmm.h"


static struct vmcb *
alloc_vmcb ( void )
{
	struct vmcb *vmcb;

	const unsigned long pfn = alloc_pages ( 1, 1 );
	vmcb = ( struct vmcb * ) VIRT ( pfn << PAGE_SHIFT );
	memset ( ( char * ) vmcb, 0, sizeof ( struct vmcb ) );

	return vmcb;
}

static unsigned long 
create_intercept_table ( unsigned long size )
{
	const unsigned long pfn = alloc_pages ( size >> PAGE_SHIFT, 1 );
	void *p = ( void * ) VIRT ( pfn << PAGE_SHIFT );

	/* vol. 2, p. 445 */
	memset ( p, 0xff, size );

	return pfn << PAGE_SHIFT; 
}

static void	
set_control_area ( struct vmcb *vmcb )
{
	/* Enable nested paging (See AMD64 manual Vol. 3, p. 488) */
	vmcb->np_enable = 1; 
	printf ( "Nested paging enabled.\n" );

	/* Flush TLB on VMRUN (all entries, all ASIDs) */
        vmcb->tlb_control = 1;

	/* To be added in RDTSC and RDTSCP */
	vmcb->tsc_offset = 0; 
	
	/* Guest address space identifier (ASID) */
	vmcb->guest_asid = 1;

	/* Intecept the VMRUN instruction */
	vmcb->general2_intercepts = INTRCPT_VMRUN;

	/* [REF] vol.2, p. 454 */
	vmcb->iopm_base_pa  = create_intercept_table ( 12 << 10 ); /* 12 Kbytes */
	vmcb->msrpm_base_pa = create_intercept_table ( 8 << 10 );  /* 8 Kbytes */
}

/* Setup the segment registers and all their hidden states 
 * (See AMD64 manual Vol 2, pp. 98-104 to know the format of generic segment descriptors) */
static void
set_descriptors ( struct vmcb *vmcb )
{
	int i;

	/* Must be a 32-bit read/execute code segment with an offset of 0 and a limit of 0xffffffff. 
	 * The exact value is undefined. */
	vmcb->cs.sel               = 0;   /* dummy */
	vmcb->cs.attrs.fields.type = 0xa; /* Data-segment, R(read) */
	vmcb->cs.attrs.fields.s    = 1;   /* S(ystem/user) */
	vmcb->cs.attrs.fields.p    = 1;   /* P(resent) */
  	vmcb->cs.attrs.fields.l    = 1;   /* L(ong) */
	vmcb->cs.limit             = 0xfffff;
	vmcb->cs.base              = 0;

	/* Must be a 32-bit read/write data segment with an offset of 0 and a limit of 0xffffffff. 
	 * The exact values are all undefined. */
	struct seg_selector *segregs [] = { &vmcb->ss, &vmcb->ds, &vmcb->es, &vmcb->fs, &vmcb->gs, NULL };
	for ( i = 0; segregs [ i ] != NULL; i++ ) {
		struct seg_selector *x = segregs [ i ];
		x->sel               = 0;   /* dummy */
		x->attrs.fields.type = 0x2; /* Data-segment, W(ritable) */
		x->attrs.fields.s    = 1;   /* S(ystem/user) */
		x->attrs.fields.p    = 1;   /* P(resent) */
		x->limit             = 0xfffff;
		x->base              = 0;
	}
}

/* See Multiboot Specification Manual */
static void
set_state_save_area ( struct vmcb *vmcb )
{
	vmcb->cpl = 0; /* must be equal to SS.DPL  */

	/* Must contatin the magin value */
	vmcb->rax = MULTIBOOT_BOOTLOADER_MAGIC;

	/* The OS image must create its own stack as soon as it needs one. */
	vmcb->rsp = 0;

	/* Bit 31 (PG) must be cleared.  Bit 0 (PE) must be set. Other bits are all undefined. */
	vmcb->cr0 = X86_CR0_PE | X86_CR0_PG; 
	vmcb->cr3 = 0x200000 + PAGE_SIZE * 3; // dummy
	vmcb->cr4 = X86_CR4_PAE;

	/* Bit 17 (VM) must be cleared. Bit 9 (IF) must be cleared.  Other bits are all undefined */
	vmcb->rflags = 2; /* set the reserved bit */

	vmcb->g_pat = 0x7040600070406UL; // ???

	vmcb->efer = EFER_LME | EFER_LMA | EFER_SVME;

	set_descriptors ( vmcb );
}

static unsigned long
alloc_vm_pmem ( unsigned long size )
{
	const unsigned long align = 1 << ( PAGE_SHIFT_2MB - PAGE_SHIFT ); /* alignment for 2 MB page table  */
	const unsigned long pfn   = alloc_pages ( size >> PAGE_SHIFT, align );
	return ( unsigned long ) VIRT ( pfn << PAGE_SHIFT );
}

/* [TODO] */
static struct multiboot_info *
init_vm_mbi ( unsigned long vm_pmem_start )
{
	enum { INSTALL_PADDR = 0x2d0e0UL }; /* < 1 MB [TODO] */
	struct multiboot_info *mbi = ( struct multiboot_info *) ( vm_pmem_start + INSTALL_PADDR );

	printf ( "Multiboot information initialized.\n" );

	return mbi;
}

static int
is_reserved_pmem ( unsigned long pfn )
{
	return ( pfn == 0 );
}

/* Create a page table that maps VM's physical addresses to PM's physical address and 
 * return the (PM's) physical base address of the table.  */
static unsigned long 
create_vm_pmem_mapping_table ( unsigned long vm_pmem_start, unsigned long vm_pmem_size )
{
	const unsigned long cr3  = pml4_table_create ( );
	const unsigned long pml4 = ( unsigned long ) VIRT ( cr3 );
	const unsigned long vm_pmem_pfn = PFN_DOWN_2MB ( PHYS ( vm_pmem_start ) );
	int i;

	for ( i = 0; i < PFN_UP_2MB ( vm_pmem_size ); i++ ) {
		const unsigned long vm_paddr = i << PAGE_SHIFT_2MB;
		const unsigned long pm_pfn   = i + ( is_reserved_pmem ( i ) ? 0 : vm_pmem_pfn ); /* for VGA (too naive) */
		const unsigned long pm_paddr = pm_pfn << PAGE_SHIFT_2MB;

		mmap ( pml4, vm_paddr, pm_paddr, 1 /* is_user */ );
	}

	printf ( "Page table for nested paging created.\n" );
	return cr3;
}

static void
create_temp_page_table ( unsigned long vm_pmem_start, unsigned long cr3 ) 
{
	const unsigned long base = vm_pmem_start + cr3;
	int i;

	printf ( "Temporal page table for virtual machine created.\n" );	
	
	for ( i = 0; i < 3; i++ ) {
		memset ( ( char * ) ( base + PAGE_SIZE * i ), 0, PAGE_SIZE );
	}
	
	union pgt_entry *e;
	
	// page-map level-4 entry 
	e = ( union pgt_entry * ) ( base );
	e->non_term.base  = ( cr3 + PAGE_SIZE ) >> PAGE_SHIFT;
	e->non_term.flags = PTTEF_PRESENT | PTTEF_RW;
	
	// page-directory-pointer 
	e = ( union pgt_entry * ) ( base + PAGE_SIZE );
	e->non_term.base  = ( cr3 + PAGE_SIZE * 2 ) >> PAGE_SHIFT;
	e->non_term.flags = PTTEF_PRESENT | PTTEF_RW;
	
	// page-directory-pointer 
	e = ( union pgt_entry * ) ( base + PAGE_SIZE * 2 );
	e->term.base  = 0;
	e->term.flags = PTTEF_PRESENT | PTTEF_RW | PTTEF_PAGE_SIZE;
	
	// page-directory-pointer
	e = ( union pgt_entry * ) ( base + PAGE_SIZE * 2 + sizeof ( union pgt_entry ) );
	e->term.base  = 1;
	e->term.flags = PTTEF_PRESENT | PTTEF_RW | PTTEF_PAGE_SIZE;
}

void
vm_create ( struct vm *vm, unsigned long guest_image_start, unsigned long guest_image_size, unsigned long vm_pmem_size )
{
	struct vmcb *vmcb;

	/* Allocate a new page for storing VMCB.  */
	vmcb = alloc_vmcb ( );
	vm->vmcb = vmcb;

	set_control_area ( vm->vmcb );
	set_state_save_area ( vm->vmcb );

	/* Allocate new pages for physical memory of the guest OS.  */
	const unsigned long vm_pmem_start = alloc_vm_pmem ( vm_pmem_size );

	/* Set Host-level CR3 to use for nested paging.  */
	vm->h_cr3   = create_vm_pmem_mapping_table ( vm_pmem_start, vm_pmem_size );
	vmcb->h_cr3 = vm->h_cr3;

	/* Copy the OS image to the specified region by interpreting the ELF format.  */
	vmcb->rip = load_elf_image ( guest_image_start, guest_image_size, vm_pmem_start );

	/* Setup multiboot info.  */
	vm->mbi = init_vm_mbi ( vm_pmem_start );

	create_temp_page_table ( vm_pmem_start, vmcb->cr3 );

	printf ( "New virtual machine created.\n" ); 	
}

/******************************************************/

static void
switch_to_guest_os ( struct vm *vm )
{
	u64 p_vmcb = PHYS ( vm->vmcb );
	svm_launch ( p_vmcb );
}

static void
handle_vmexit ( struct vm *vm )
{
	printf ( "********************\n" );
	

	print_vmexit_exitcode ( vm->vmcb->exitcode );
	printf ( "VMCB: rip=%x\n", vm->vmcb->rip );

	printf ( "cpl=%x\n", vm->vmcb->cpl );
	printf ( "cr0=%x, cr3=%x, cr4=%x\n", vm->vmcb->cr0, vm->vmcb->cr3, vm->vmcb->cr4 );
	printf ( "rflags=%x, efer=%x\n", vm->vmcb->rflags, vm->vmcb->efer );

	printf ( "cs.attrs=%x, ds.attrs=%x\n", vm->vmcb->cs.attrs.bytes, vm->vmcb->ds.attrs.bytes );

	printf ( "error_code=%x, fault address=%x\n", vm->vmcb->exitinfo1, vm->vmcb->exitinfo2 );

	/* p. 268, p.490 */
	if ( vm->vmcb->exitinfo1 & 1 ) {
		printf ( "page fault was caused by a page-protection violation\n" );
	} else {
		printf ( "page fault was caused by a not-present page\n" );
	}

	if ( vm->vmcb->exitinfo1 & 2 ) {
		printf ( "memory access was write\n" );
	} else {
		printf ( "memory access was read\n" );
	}

	if ( vm->vmcb->exitinfo1 & 4 ) {
		printf ( "an access in user mode caused the page fault\n" );
	} else {
		printf ( "an access in supervisor mode caused the page fault\n" );
	}
}

void
vm_boot ( struct vm *vm )
{
	printf ( "Booting guest operating system...\n\n" ); 

	vmcb_check_consistency ( vm->vmcb );

	while ( 1 ) {
		/* [TODO] setup registers (set %ebx to mbi address) */

		switch_to_guest_os ( vm );

		handle_vmexit ( vm );

		break; /* [DEBUG] */
	}
}
