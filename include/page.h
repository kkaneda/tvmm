#ifndef __PAGE_H__
#define __PAGE_H__


#define PAGE_SHIFT 12
#define PAGE_SIZE  ( 1 << PAGE_SHIFT )
#define PAGE_MASK  ( ~ ( PAGE_SIZE - 1 ) )

#define PAGE_SHIFT_2MB 21
#define PAGE_SIZE_2MB  ( 1 << PAGE_SHIFT_2MB )


#define PFN_UP(x)	(((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((x) << PAGE_SHIFT)

#define PFN_UP_2MB(x)	(((x) + PAGE_SIZE_2MB - 1) >> PAGE_SHIFT_2MB)
#define PFN_DOWN_2MB(x)	((x) >> PAGE_SHIFT_2MB)


#define PAGE_UP(p)    ( ( (p) + ( PAGE_SIZE - 1 ) ) & PAGE_MASK )
#define PAGE_DOWN(p)  ( (p) & PAGE_MASK )



/* Page-Translation-Table Entry Fields
   [REF] vol.2, p. 168- */
#define _PTTEF_PRESENT   0
#define _PTTEF_RW        1 /* Read/Write */
#define _PTTEF_US        2 /* User/Supervisor */
#define _PTTEF_ACCESSED  5
#define _PTTEF_DIRTY     6
#define _PTTEF_PAGE_SIZE 7
#define _PTTEF_GLOBAL	 8
#define PTTEF_PRESENT    (1 << _PTTEF_PRESENT)
#define PTTEF_RW         (1 << _PTTEF_RW)
#define PTTEF_US         (1 << _PTTEF_US)
#define PTTEF_ACCESSED   (1 << _PTTEF_ACCESSED)
#define PTTEF_DIRTY      (1 << _PTTEF_DIRTY)
#define PTTEF_PAGE_SIZE  (1 << _PTTEF_PAGE_SIZE)
#define PTTEF_GLOBAL     (1 << _PTTEF_GLOBAL)


#ifdef __ASSEMBLY__

#define VMM_OFFSET	0xFFFF830000000000
#define PHYS(va)	((va) - VMM_OFFSET)

#else /* ! __ASSEMBLY__ */

#define VMM_OFFSET	0xFFFF830000000000UL
#define PHYS(va)	((unsigned long)(va) - VMM_OFFSET)
#define VIRT(pa)	((void *)((unsigned long)(pa) + VMM_OFFSET))

#endif /* __ASSEMBLY__ */


#ifndef __ASSEMBLY__


enum pg_table_level {
	PGT_LEVEL_PML4 = 4,
	PGT_LEVEL_PDP  = 3,
	PGT_LEVEL_PD   = 2
};

/* [REF] AMD64 manual Vol. 2, pp. 166-167 */

/* For 2-Mbyte page translation (long-mode) */
union pgt_entry {
	/* 2-Mbyte PML4E and PDPE */
	struct non_term {
		u16 flags: 12; /* Bit 0-11  */
		u64 base:  40; /* Bit 12-51 */
		u16 avail: 11; /* Bit 52-62 */
		u16 nx:    1;  /* Bit 63    */
	} __attribute__ ((packed)) non_term;

	/* 2-Mbyte PDE */
	struct term {
		u32 flags: 21; /* Bit 0-20  */
		u32 base:  31; /* Bit 21-51 */
		u16 avail: 11; /* Bit 52-62 */
		u16 nx:    1;  /* Bit 63    */
	} __attribute__ ((packed)) term;
};  

unsigned long pml4_table_create ( void );
extern void mmap ( unsigned long pml4_table_base_vaddr, unsigned long vaddr, unsigned long paddr, int is_user );
extern unsigned long vaddr_to_paddr ( unsigned long pml4_table_base_vaddr, unsigned long vaddr );
extern void print_pg_table ( unsigned long pml4_table_base_vaddr );

#endif /* ! __ASSEMBLY__ */


#endif /* __PAGE_H__ */
