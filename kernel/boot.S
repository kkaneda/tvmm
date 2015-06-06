#define __ASSEMBLY__


#include "multiboot.h"
#include "msr.h"	
#include "page.h"
#include "system.h"	
#include "vmm.h"


/* Which file is good to declare the following macro? */	
#define CPU_ID_EXT_FEATURES	0
	
	.text
        .code32

	.globl	start, _start
start:
_start:
	jmp	physical_entry	/* NOTE: relative addressing */

	.align	4		/* Align 32 bits boundary.  */
	
	/* Multiboot header.  */
multiboot_header:
	.long	MULTIBOOT_HEADER_MAGIC
	.long	MULTIBOOT_HEADER_FLAGS
	.long	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS) /* checksum */
	
die:
        jmp     die

physical_entry:
        cld
        cli

	cmpl	$MULTIBOOT_BOOTLOADER_MAGIC, %eax
	jne	die /* NOTE : relative addressing */

        /* Save the multiboot info structure for later use.  */
        movl	%ebx, PHYS(multiboot_ptr)
	
	/* Set up descriptors.  */
 	lgdt 	PHYS(boot_gdt_descr)
	movl 	$VMM_DS32, %eax
	movl 	%eax, %ds
	movl 	%eax, %es
	movl 	%eax, %ss

        /* Set up FPU.  */
        fninit
        
	/* Enabling paging mechanism...  */

	/* Enable PAE in CR4.  */
        movl	$X86_CR4_PAE, %ecx 
        movl	%ecx, %cr4

	/* Load pagetable base register.  */
        movl	$(PHYS(vmm_pml4_table)), %eax
        movl	%eax, %cr3
	
        /* Set up EFER (Extended Feature Enable Register).  */
        movl	$MSR_EFER, %ecx
        rdmsr
        btsl	$_EFER_LME, %eax
        btsl	$_EFER_SCE, %eax
        movl	$CPU_ID_EXT_FEATURES, %edi
        btl	$20, %edi        /* CPUID 0x80000001, EDX[20] */
        jnc	1f
        btsl    $_EFER_NX, %eax
1:      wrmsr

	/* Load CR0 (See AMD64 manual vol. 2, p. 53).  */
        movl	$(X86_CR0_PE | X86_CR0_MP | X86_CR0_ET | X86_CR0_NE | X86_CR0_WP | X86_CR0_AM | X86_CR0_PG), %eax
        movl	%eax, %cr0
	
	/* Paging enabled.  Now in compatibility mode.  */

	/*  Long-jump into 64-bit mode. (See AMD64 manual Vol. 2, p. 104) */ 
	ljmp	$(VMM_CS64), $(PHYS(virtual_entry)) /* PHYS(...) is used to calculate the offset of the target address into the code segment */

        .code64
virtual_entry:
        /* Install relocated selectors (FS/GS unused). */
        lgdt	gdt_descr(%rip) /* [Note] RIP-Relative Addressing (See AMD64 manual Vol. 3, p. 23) */

	/* Initialize the stack.  */
	movabs	$(boot_stack + STACK_SIZE), %rsp
	
        /* Reset EFLAGS (subsumes CLI and CLD). */
        pushq   $0
        popfq

	/* [???] In data segment descriptor of long-mode, only present-bit is valid. */
	mov	$(VMM_DS64), %ecx /* ??? */
        mov	%ecx, %ds
        mov	%ecx, %es
        mov	%ecx, %fs
        mov	%ecx, %gs
        mov	%ecx, %ss
	
#if 0
	/* [TODO] Initialize IDT.  */
	
        lidt    idt_descr(%rip)

        /* Initialize BSS (no nasty surprises!) */
        lea     __bss_start(%rip), %rdi
        lea     _end(%rip), %rcx
        sub     %rdi, %rcx
        xor     %rax, %rax
        rep     stosb

        /* Initialise IDT with simple error defaults. */
        leaq    ignore_int(%rip),%rcx
        movl    %ecx,%eax
        andl    $0xFFFF0000,%eax
        orl     $0x00008E00,%eax
        shlq    $32,%rax
        movl    %ecx,%edx
        andl    $0x0000FFFF,%edx
        orl     $(__HYPERVISOR_CS64<<16),%edx
        orq     %rdx,%rax
        shrq    $32,%rcx
        movl    %ecx,%edx
        leaq    idt_table(%rip),%rdi
        movl    $256,%ecx
1:      movq    %rax,(%rdi)
        movq    %rdx,8(%rdi)
        addq    $16,%rdi
        loop    1b
#endif

        /* Pass off the Multiboot info structure to C land (move its address into %rdi). */
        movl	multiboot_ptr(%rip), %edi
	movq 	$VMM_OFFSET, %rax
	addq	%rax, %rdi /* Translate a physical address to a virtual address.  */
	
        call    start_vmm
	
	/* Force a panic (the program should not reach this point of the code).  */	
        ud2 

	.data
        .code32
	
/* Values loaded by the LGDT instruction */	
	.align	4
boot_gdt_descr:
	.word	GDT_ENTRIES * 8 - 1  /* limit */ 
	.quad	PHYS(gdt_table)  /* address of GDT (when paging is disabled) */

gdt_descr:
	.word	GDT_ENTRIES * 8 - 1 /* limit */
	.quad	gdt_table	 /* address of GDT (when paging is enabled) */
	
/* Initial Global Descriptor Table (See AMD64 manual vol. 2, pp. 109-110) */
	.align	PAGE_SIZE	
gdt_table:
#        .quad	0x0000000000000000     /* unused */
        .quad	0x0000800000000000     /* (present only) */
        .quad	0x00cf9a000000ffff     /* ring 0 code, compatibility 
				          (base =0, limit = ffff, R, P, G, D) */
        .quad	0x00af9a000000ffff     /* ring 0 code, 64-bit mode
				          (base =0, limit = ffff, R, P, G, L) */
        .quad	0x00cf92000000ffff     /* ring 0 data                */
        .quad	0x00cffa000000ffff     /* ring 3 code, compatibility */
        .quad	0x00cff2000000ffff     /* ring 3 data                */
        .quad	0x00affa000000ffff     /* ring 3 code, 64-bit mode   */
        .quad	0x0000000000000000     /* unused                            */
        .fill	4, 8, 0    	       /* space for TSS and LDT per CPU     */

/* Page tables (See AMD64 manual vol. 2, pp. 165-167) */
/* Initial Page-Map Level-4 (PML4) table */
	.align	PAGE_SIZE
vmm_pml4_table:	
        .quad	(PHYS(vmm_pdp_table) + PTTEF_PRESENT + PTTEF_RW + PTTEF_US) /* 0-th entry */
        .fill	261, 8, 0
        .quad	(PHYS(vmm_pdp_table) + PTTEF_PRESENT + PTTEF_RW + PTTEF_US) /* x-th entry where x = 262 = PML4 offset (39-47 bit) of VMM_OFFSET */
        .fill	249, 8, 0
	
/* Initial Page-Directory-Pointer (PDP) table */
	.align	PAGE_SIZE
vmm_pdp_table:	
        .quad	(PHYS(vmm_pdir_table) + PTTEF_PRESENT + PTTEF_RW + PTTEF_US)
        .fill	511, 8, 0
	
/* Initial Page-Directory (PD) table */
/* Maps first 1GB physical memory. */
	.align	PAGE_SIZE
vmm_pdir_table:	
        .macro	identmap from=0, count=512
        .if 	\count-1
        identmap "(\from+0)","(\count/2)"
        identmap "(\from+(0x200000*(\count/2)))","(\count/2)"
        .else
	.quad	\from | PTTEF_PRESENT | PTTEF_RW | PTTEF_PAGE_SIZE
        .endif
        .endm
        identmap
	
/* 4-byte region for multiboot info structure */	
multiboot_ptr:
	.fill	4

	.align	4
	.code64	
boot_stack:
 	.fill	STACK_SIZE
