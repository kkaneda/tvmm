#ifndef __VMEXIT_H__
#define __VMEXIT_H__


/* [REF] AMD64 manual Vol. 2, Appendix C */


enum vmexit_exitcode {
    /* control register read exitcodes */
    VMEXIT_CR0_READ    =   0,
    VMEXIT_CR1_READ    =   1,
    VMEXIT_CR2_READ    =   2,
    VMEXIT_CR3_READ    =   3,
    VMEXIT_CR4_READ    =   4,
    VMEXIT_CR5_READ    =   5,
    VMEXIT_CR6_READ    =   6,
    VMEXIT_CR7_READ    =   7,
    VMEXIT_CR8_READ    =   8,
    VMEXIT_CR9_READ    =   9,
    VMEXIT_CR10_READ   =  10,
    VMEXIT_CR11_READ   =  11,
    VMEXIT_CR12_READ   =  12,
    VMEXIT_CR13_READ   =  13,
    VMEXIT_CR14_READ   =  14,
    VMEXIT_CR15_READ   =  15,

    /* control register write exitcodes */
    VMEXIT_CR0_WRITE   =  16,
    VMEXIT_CR1_WRITE   =  17,
    VMEXIT_CR2_WRITE   =  18,
    VMEXIT_CR3_WRITE   =  19,
    VMEXIT_CR4_WRITE   =  20,
    VMEXIT_CR5_WRITE   =  21,
    VMEXIT_CR6_WRITE   =  22,
    VMEXIT_CR7_WRITE   =  23,
    VMEXIT_CR8_WRITE   =  24,
    VMEXIT_CR9_WRITE   =  25,
    VMEXIT_CR10_WRITE  =  26,
    VMEXIT_CR11_WRITE  =  27,
    VMEXIT_CR12_WRITE  =  28,
    VMEXIT_CR13_WRITE  =  29,
    VMEXIT_CR14_WRITE  =  30,
    VMEXIT_CR15_WRITE  =  31,

    /* debug register read exitcodes */
    VMEXIT_DR0_READ    =  32,
    VMEXIT_DR1_READ    =  33,
    VMEXIT_DR2_READ    =  34,
    VMEXIT_DR3_READ    =  35,
    VMEXIT_DR4_READ    =  36,
    VMEXIT_DR5_READ    =  37,
    VMEXIT_DR6_READ    =  38,
    VMEXIT_DR7_READ    =  39,
    VMEXIT_DR8_READ    =  40,
    VMEXIT_DR9_READ    =  41,
    VMEXIT_DR10_READ   =  42,
    VMEXIT_DR11_READ   =  43,
    VMEXIT_DR12_READ   =  44,
    VMEXIT_DR13_READ   =  45,
    VMEXIT_DR14_READ   =  46,
    VMEXIT_DR15_READ   =  47,

    /* debug register write exitcodes */
    VMEXIT_DR0_WRITE   =  48,
    VMEXIT_DR1_WRITE   =  49,
    VMEXIT_DR2_WRITE   =  50,
    VMEXIT_DR3_WRITE   =  51,
    VMEXIT_DR4_WRITE   =  52,
    VMEXIT_DR5_WRITE   =  53,
    VMEXIT_DR6_WRITE   =  54,
    VMEXIT_DR7_WRITE   =  55,
    VMEXIT_DR8_WRITE   =  56,
    VMEXIT_DR9_WRITE   =  57,
    VMEXIT_DR10_WRITE  =  58,
    VMEXIT_DR11_WRITE  =  59,
    VMEXIT_DR12_WRITE  =  60,
    VMEXIT_DR13_WRITE  =  61,
    VMEXIT_DR14_WRITE  =  62,
    VMEXIT_DR15_WRITE  =  63,

    /* processor exception exitcodes (VMEXIT_EXCP[0-31]) */
    VMEXIT_EXCEPTION_DE  =  64, /* divide-by-zero-error */
    VMEXIT_EXCEPTION_DB  =  65, /* debug */
    VMEXIT_EXCEPTION_NMI =  66, /* non-maskable-interrupt */
    VMEXIT_EXCEPTION_BP  =  67, /* breakpoint */
    VMEXIT_EXCEPTION_OF  =  68, /* overflow */
    VMEXIT_EXCEPTION_BR  =  69, /* bound-range */
    VMEXIT_EXCEPTION_UD  =  70, /* invalid-opcode*/
    VMEXIT_EXCEPTION_NM  =  71, /* device-not-available */
    VMEXIT_EXCEPTION_DF  =  72, /* double-fault */
    VMEXIT_EXCEPTION_09  =  73, /* unsupported (reserved) */
    VMEXIT_EXCEPTION_TS  =  74, /* invalid-tss */
    VMEXIT_EXCEPTION_NP  =  75, /* segment-not-present */
    VMEXIT_EXCEPTION_SS  =  76, /* stack */
    VMEXIT_EXCEPTION_GP  =  77, /* general-protection */
    VMEXIT_EXCEPTION_PF  =  78, /* page-fault */
    VMEXIT_EXCEPTION_15  =  79, /* reserved */
    VMEXIT_EXCEPTION_MF  =  80, /* x87 floating-point exception-pending */
    VMEXIT_EXCEPTION_AC  =  81, /* alignment-check */
    VMEXIT_EXCEPTION_MC  =  82, /* machine-check */
    VMEXIT_EXCEPTION_XF  =  83, /* simd floating-point */

    /* exceptions 20-31 (exitcodes 84-95) are reserved */

    /* ...and the rest of the #VMEXITs */
    VMEXIT_INTR             =  96,
    VMEXIT_NMI              =  97,
    VMEXIT_SMI              =  98,
    VMEXIT_INIT             =  99,
    VMEXIT_VINTR            = 100,
    VMEXIT_CR0_SEL_WRITE    = 101,
    VMEXIT_IDTR_READ        = 102,
    VMEXIT_GDTR_READ        = 103,
    VMEXIT_LDTR_READ        = 104,
    VMEXIT_TR_READ          = 105,
    VMEXIT_IDTR_WRITE       = 106,
    VMEXIT_GDTR_WRITE       = 107,
    VMEXIT_LDTR_WRITE       = 108,
    VMEXIT_TR_WRITE         = 109,
    VMEXIT_RDTSC            = 110,
    VMEXIT_RDPMC            = 111,
    VMEXIT_PUSHF            = 112,
    VMEXIT_POPF             = 113,
    VMEXIT_CPUID            = 114,
    VMEXIT_RSM              = 115,
    VMEXIT_IRET             = 116,
    VMEXIT_SWINT            = 117,
    VMEXIT_INVD             = 118,
    VMEXIT_PAUSE            = 119,
    VMEXIT_HLT              = 120,
    VMEXIT_INVLPG           = 121,
    VMEXIT_INVLPGA          = 122,
    VMEXIT_IOIO             = 123,
    VMEXIT_MSR              = 124,
    VMEXIT_TASK_SWITCH      = 125,
    VMEXIT_FERR_FREEZE      = 126,
    VMEXIT_SHUTDOWN         = 127,
    VMEXIT_VMRUN            = 128,
    VMEXIT_VMMCALL          = 129,
    VMEXIT_VMLOAD           = 130,
    VMEXIT_VMSAVE           = 131,
    VMEXIT_STGI             = 132,
    VMEXIT_CLGI             = 133,
    VMEXIT_SKINIT           = 134,
    VMEXIT_RDTSCP           = 135,
    VMEXIT_ICEBP            = 136,
    VMEXIT_NPF              = 1024, /* nested paging fault */
    VMEXIT_INVALID          =  -1
};


extern void print_vmexit_exitcode ( enum vmexit_exitcode x );


#endif /* __VMEXIT_H__ */
