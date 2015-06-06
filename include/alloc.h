#ifndef __ALLOC_H__
#define __ALLOC_H__


#include "e820.h"
#include "pmem_layout.h"


extern void __init naive_allocator_init ( const struct e820_map *e820, struct pmem_layout *pml );
unsigned long alloc_pages ( unsigned long nr_pfns, unsigned long pfn_align );



#endif /* __ALLOC_H__ */
