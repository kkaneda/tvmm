OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

/* Program headers */
PHDRS
{
	text PT_LOAD; /* Indicate that this program header describes a segment to be loaded from the file */
}

phys = 0x200000; /* 2 MB */
SECTIONS
{
	.text : AT(phys)
	{	code = .;
	  	*(.text)
	  	. = ALIGN(4096);
	} : text /* Assign this section to the `text` segment described by the program header */

	.rodata : AT(phys + (rodata - code))
	{	rodata = .;
		*(.rodata) 
         	. = ALIGN(4096);
	} : text

	.data : AT(phys + (data - code))
	{ 	data = .;
	  	*(.data)
         	. = ALIGN(4096);
    	} : text
	_edata = .;

	.bss : AT(phys + (bss - code))
	{ 	bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4096);
	} : text
	_end = .;
}
