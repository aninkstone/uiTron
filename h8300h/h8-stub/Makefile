#
# gdb-h8-stub/Makefile
#

.S.o:
	$(CC) -D__ASSEMBLY__ $(AFLAGS) -traditional -c -o $*.o $<

#CROSS_COMPILE=h8300-hms-
CROSS_COMPILE=h8300-linux-elf-

CC      = $(CROSS_COMPILE)gcc -I.
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS		 = -O2 -g
#AFLAGS		+= -mgas

CFLAGS		+= -mh -mint32
AFLAGS		+= -mh -mint32 

CFLAGS		+= -pipe
LINKSCRIPT	 = h8-stub.lds
LINKFLAGS	+= -T $(word 1,$(LINKSCRIPT)) -e _start -Map h8-stub.map

ADJUST_VMA = 0

OBJECTS=\
	head.o\
	entry.o\
	isr.o\
	h8-sci.o\
	h8-stub.o\
	string.o

h8-stub.mot: h8-stub
	$(OBJCOPY) -S -g -R .data -R .bss -R .stack -R .comment -O srec \
		h8-stub h8-stub.mot
	$(OBJDUMP) -S h8-stub > h8-stub.dump
		
h8-stub.elf: h8-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
	--adjust-vma=${ADJUST_VMA} \
	h8-stub h8-stub.elf

h8-stub.hex: h8-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} -O ihex \
		h8-stub h8-stub.hex

h8-stub.bin: h8-stub
	$(OBJCOPY) -S -g -R .data -R .stack -R .bss -R .comment -O binary \
		h8-stub h8-stub.bin

		
h8-stub: ${OBJECTS} h8-stub.lds
	$(LD) $(LINKFLAGS) ${OBJECTS} \
		-o h8-stub

h8-stub.lds: h8-stub.lds.S
	$(CC) $(CFLAGS) -E -C -P -I. h8-stub.lds.S >h8-stub.lds

all: h8-stub.elf h8-stub.bin h8-stub.hex h8-stub.mot
clean:
	rm -f h8-stub h8-stub.bin h8-stub.elf h8-stub.mot h8-stub.hex *.o h8-stub.lds h8-stub.map h8-stub.dump

