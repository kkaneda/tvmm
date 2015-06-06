#define __ASSEMBLY__

#include "multiboot.h"
#include "msr.h"	
#include "page.h"

	
	.text
        .code64

	.globl	start, _start
start:
_start:
        mov     $(PHYS(check_msg)), %esi 
        jmp     print_err	
check_msg:
        .asciz "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SAMPLE OPERATING SYSTEM !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
print_err:
        mov     $0xB8000,%edi  # VGA framebuffer
1:      mov     (%esi), %bl
        test    %bl, %bl        # Terminate on '\0' sentinel
	je      2f		
        movsb                  # Write a character to the VGA framebuffer
        mov     $7,%al
        stosb                  # Write an attribute to the VGA framebuffer
        jmp     1b
2:	ud2a
