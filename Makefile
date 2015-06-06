TARGET = tvmm

all: ${TARGET}

${TARGET}:
	cd kernel/ && make ${TARGET}

clean:
	cd kernel/ && make clean

