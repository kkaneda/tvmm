#define VMRUN  .byte 0x0F,0x01,0xD8
#define VMLOAD .byte 0x0F,0x01,0xDA
#define VMSAVE .byte 0x0F,0x01,0xDB
#define STGI   .byte 0x0F,0x01,0xDC
#define CLGI   .byte 0x0F,0x01,0xDD
	
	.global svm_launch
svm_launch:	

	/* Set the pointer to VMCB to %rax (vol. 2, p. 440) */
	movq 	%rdi, %rax 
        pushq	%r15
        pushq	%r14
        pushq	%r13
        pushq	%r12
        pushq	%rbp
        pushq	%rbx

        CLGI

	/* save other general-purpose registers? */

	
#         VMLOAD
        VMRUN
#        VMSAVE

	/* restore other general-purpose registers? */
	
        STGI

        popq	%rbx
        popq	%rbp
        popq	%r12
        popq	%r13
        popq	%r14
        popq	%r15
	
	retq
	
#	ud2a
	
