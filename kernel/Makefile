TVMM = tvmm
SOS  = sos

TOP_DIR     = ..
TOOLS_DIR   = ${TOP_DIR}/tools
INCLUDE_DIR = ${TOP_DIR}/include

CC      = gcc
NM      = nm
LD      = ld
MKELF32 = ${TOOLS_DIR}/mkelf32

INCLUDES = -I${INCLUDE_DIR}
CFLAGS   = -Wall ${INCLUDES} -fPIC -nostdinc -nostdlib -fno-builtin -iwithprefix include 

HDRS  = ${INCLUDE_DIR}/types.h ${INCLUDE_DIR}/multiboot.h ${INCLUDE_DIR}/string.h ${INCLUDE_DIR}/printf.h ${INCLUDE_DIR}/failure.h \
	${INCLUDE_DIR}/bitops.h ${INCLUDE_DIR}/msr.h ${INCLUDE_DIR}/e820.h ${INCLUDE_DIR}/cpufeature.h ${INCLUDE_DIR}/cpu.h \
	${INCLUDE_DIR}/system.h ${INCLUDE_DIR}/elf.h ${INCLUDE_DIR}/page.h ${INCLUDE_DIR}/svm.h \
	${INCLUDE_DIR}/vmexit.h ${INCLUDE_DIR}/vmcb.h ${INCLUDE_DIR}/vm.h  ${INCLUDE_DIR}/pmem_layout.h \
	${INCLUDE_DIR}/vmm.h ${INCLUDE_DIR}/alloc.h

COMMON_OBJECTS = string.o printf.o failure.o e820.o

# [???] boot.o must be the head of list
TVMM_OBJECTS   = boot.o ${COMMON_OBJECTS} elf.o cpu.o \
	         alloc.o svm.o svm_asm.o page.o vmexit.o vmcb.o vm.o setup.o 

SOS_OBJECTS    = sos_boot.o ${COMMON_OBJECTS} sos.o


TEXT_OFFSET    = 0xFFFF830000200000
LOAD_BASE_ADDR = 0x200000

all: ${TVMM} ${SOS}

${TVMM}: ${TVMM_OBJECTS} ${TVMM}.lds
	${LD} ${LDFLAGS} -T ${TVMM}.lds -o ${TVMM}-syms -Ttext ${TEXT_OFFSET} ${TVMM_OBJECTS} 
	${MKELF32} $@-syms $@ ${LOAD_BASE_ADDR} \
	  `${NM} ${TVMM}-syms | sort | tail -n 1 | sed -e 's/^\([^ ]*\).*/0x\1/'`

${SOS}: ${SOS_OBJECTS} ${SOS}.lds
	${LD} ${LDFLAGS} -T ${SOS}.lds -o ${SOS}-syms -Ttext ${TEXT_OFFSET} ${SOS_OBJECTS} 
	${MKELF32} $@-syms $@ ${LOAD_BASE_ADDR} \
	  `${NM} ${SOS}-syms | sort | tail -n 1 | sed -e 's/^\([^ ]*\).*/0x\1/'`

%.o: %.c ${HDRS} Makefile
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S ${HDRS} Makefile
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${COMMON_OBJECTS} ${TVMM_OBJECTS} ${SOS_OBJECTS} \
	      ${TVMM} ${TVMM}-syms ${SOS} ${SOS}-syms 
