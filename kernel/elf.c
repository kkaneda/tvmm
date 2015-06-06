#include "string.h"
#include "printf.h"
#include "elf.h"
#include "vm.h"


static int
is_loadable_phdr ( struct Elf_Phdr *phdr )
{
	return ( ( phdr->p_type == PT_LOAD ) && ( ( phdr->p_flags & ( PF_W | PF_X ) ) ) );
}

struct Elf_Phdr *
get_elf_phdr ( struct Elf_Ehdr *ehdr, int i )
{
	char *p = ( char * ) ehdr;
	return ( struct Elf_Phdr * ) ( p + ehdr->e_phoff + ( i * ehdr->e_phentsize ) );
}

unsigned long 
load_elf_image ( unsigned long guest_image_start, unsigned long guest_image_size, unsigned long vm_pmem_start )
{
	struct Elf_Ehdr *ehdr = ( struct Elf_Ehdr * ) guest_image_start;

	// [DEBUG] 
//	printf ( "Entry point address: %x\n",  ( unsigned long ) ehdr->e_entry );

	int i;
	for ( i = 0; i < ehdr->e_phnum; i++ ) {

		struct Elf_Phdr *phdr = get_elf_phdr ( ehdr, i );
		
		if ( ! is_loadable_phdr ( phdr ) ) {
			continue;
		}

		/* [DEBUG] */
//		printf ( "[%x] vaddr=%x, paddr=%x, filesz=%x, memsz=%x\n", i, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz );

		if ( phdr->p_filesz > 0 ) {
			memmove ( ( char * ) ( vm_pmem_start + phdr->p_paddr ), ( ( char * ) ehdr ) + phdr->p_offset, phdr->p_filesz );
		}
		
		size_t len = phdr->p_memsz - phdr->p_filesz;
		if ( len > 0 ) {
			memset ( ( char * ) ( vm_pmem_start + phdr->p_paddr + phdr->p_filesz ), 0, len );
		}
	}

	printf ( "ELF image loaded.\n" );

	return ehdr->e_entry;
}

