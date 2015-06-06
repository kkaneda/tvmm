#ifndef __PMEM_LAYOUT_H__
#define __PMEM_LAYOUT_H__


struct pmem_layout {
	unsigned long max_page;
	unsigned long total_pages;

	unsigned long vmm_heap_start, vmm_heap_end;
	unsigned long guest_image_start, guest_image_size;
};


#endif /* __PMEM_LAYOUT_H__ */
