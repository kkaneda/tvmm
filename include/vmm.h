#ifndef __VMM_H__
#define __VMM_H__


#define STACK_SIZE	(1 << 16) /* 64 KB */
#define	DEFAULT_VMM_HEAP_SIZE (1 << 22) /* 4 MB */
#define	DEFAULT_VM_PMEM_SIZE  (1 << 22) /* 4 MB */

#define VMM_CS64_ENTRY	2
#define VMM_DS32_ENTRY	3

#define VMM_CS64	16 // second entry of gdt
#define VMM_DS32	24 // third entry of gdt
#define VMM_DS64	0  /* ??? */

#define GDT_ENTRIES	12 /* ??? */


#endif /* __VMM_H__ */
