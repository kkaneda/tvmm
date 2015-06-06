#ifndef __VM_H__
#define __VM_H__


#include "multiboot.h"
#include "vmcb.h"

struct vm {
	struct vmcb *vmcb;

	unsigned long h_cr3;  /* [Note] When #VMEXIT occurs with
			       * nested paging enabled, hCR3 is not
			       * saved back into the VMCB (p. 488) */
	struct multiboot_info *mbi; /* virtual address */
};

extern void vm_create ( struct vm *vm, unsigned long guest_image_start, unsigned long guest_image_size, unsigned long vm_pmem_size );
extern void vm_boot ( struct vm *vm );


#endif /* __VM_H__ */
