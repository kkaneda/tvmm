#ifndef __ELF_H__
#define __ELF_H__


#include "types.h"


typedef u8	Elf_Byte;

typedef u32	Elf32_Addr;	/* Unsigned program address */
typedef u32	Elf32_Off;	/* Unsigned file offset */
typedef s32	Elf32_Sword;	/* Signed large integer */
typedef u32	Elf32_Word;	/* Unsigned large integer */
typedef u16	Elf32_Half;	/* Unsigned medium integer */

typedef u64	Elf64_Addr;
typedef u64	Elf64_Off;
typedef s32	Elf64_Shalf;

typedef s32	Elf64_Sword;
typedef u32	Elf64_Word;

typedef s64	Elf64_Sxword;
typedef u64	Elf64_Xword;

typedef u32	Elf64_Half;
typedef u16	Elf64_Quarter;


enum segment_type {
	PT_LOAD = 1	/* loadable segment */
};

enum segment_flag {
	PF_X = 0x1,	/* Executable */
	PF_W = 0x2,	/* Writable */
	PF_R = 0x4,	/* Readable */
};

enum {
	EI_NIDENT = 16
};

/* ELF Header */
struct Elf32_Ehdr {
	unsigned char	e_ident[EI_NIDENT]; /* ELF Identification */
	Elf32_Half	e_type;		/* object file type */
	Elf32_Half	e_machine;	/* machine */
	Elf32_Word	e_version;	/* object file version */
	Elf32_Addr	e_entry;	/* virtual entry point */
	Elf32_Off	e_phoff;	/* program header table offset */
	Elf32_Off	e_shoff;	/* section header table offset */
	Elf32_Word	e_flags;	/* processor-specific flags */
	Elf32_Half	e_ehsize;	/* ELF header size */
	Elf32_Half	e_phentsize;	/* program header entry size */
	Elf32_Half	e_phnum;	/* number of program header entries */
	Elf32_Half	e_shentsize;	/* section header entry size */
	Elf32_Half	e_shnum;	/* number of section header entries */
	Elf32_Half	e_shstrndx;	/* section header table's "section 
					   header string table" entry offset */
};

struct Elf64_Ehdr {
	unsigned char	e_ident[EI_NIDENT];	/* Id bytes */
	Elf64_Quarter	e_type;			/* file type */
	Elf64_Quarter	e_machine;		/* machine type */
	Elf64_Half	e_version;		/* version number */
	Elf64_Addr	e_entry;		/* entry point */
	Elf64_Off	e_phoff;		/* Program hdr offset */
	Elf64_Off	e_shoff;		/* Section hdr offset */
	Elf64_Half	e_flags;		/* Processor flags */
	Elf64_Quarter	e_ehsize;		/* sizeof ehdr */
	Elf64_Quarter	e_phentsize;		/* Program header entry size */
	Elf64_Quarter	e_phnum;		/* Number of program headers */
	Elf64_Quarter	e_shentsize;		/* Section header entry size */
	Elf64_Quarter	e_shnum;		/* Number of section headers */
	Elf64_Quarter	e_shstrndx;		/* String table index */
};


/* Program Header */
struct  Elf32_Phdr {
	Elf32_Word	p_type;		/* segment type */
	Elf32_Off	p_offset;	/* segment offset */
	Elf32_Addr	p_vaddr;	/* virtual address of segment */
	Elf32_Addr	p_paddr;	/* physical address - ignored? */
	Elf32_Word	p_filesz;	/* number of bytes in file for seg. */
	Elf32_Word	p_memsz;	/* number of bytes in mem. for seg. */
	Elf32_Word	p_flags;	/* flags */
	Elf32_Word	p_align;	/* memory alignment */
};

struct Elf64_Phdr {
	Elf64_Half	p_type;		/* entry type */
	Elf64_Half	p_flags;	/* flags */
	Elf64_Off	p_offset;	/* offset */
	Elf64_Addr	p_vaddr;	/* virtual address */
	Elf64_Addr	p_paddr;	/* physical address */
	Elf64_Xword	p_filesz;	/* file size */
	Elf64_Xword	p_memsz;	/* memory size */
	Elf64_Xword	p_align;	/* memory & file alignment */
};


/* [TODO] We should support both 32-bit and 64-bit version of elf format.  */
#if 1

#define Elf_Ehdr	Elf32_Ehdr
#define Elf_Phdr	Elf32_Phdr

#else 

#define Elf_Ehdr	Elf64_Ehdr
#define Elf_Phdr	Elf64_Phdr

#endif 

extern unsigned long load_elf_image ( unsigned long guest_image_start, unsigned long guest_image_size, unsigned long vm_pmem_start );


#endif /* __ELF_H__ */
