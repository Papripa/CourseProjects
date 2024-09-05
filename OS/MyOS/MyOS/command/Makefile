# commands/Makefile

#ENTRYPOINT	= 0x1000
HD		= ../80m.img

ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
ASMFLAGS	= -I ../include/ -f elf
CFLAGS		= -I ../include/ -c -fno-builtin -fno-stack-protector -Wall
LDFLAGS		= -Ttext 0x1000
DASMFLAGS	= -D
LIB		= ../lib/orangescrt.a
BIN		= echo pwd stackoverflow  counter1 counter2 counter3  testa attack 

# All Phony Targets
.PHONY : everything final clean realclean disasm all install 

# Default starting position
everything : $(BIN)

install : all clean
	cp ../kernel.bin ./ -v
	tar vcf inst.tar kernel.bin $(BIN)
	dd if=inst.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk -F " " '{print $$5}'` conv=notrunc

all : realclean everything

final : all clean

clean :
	rm -f *.o

realclean :
	rm -f $(BIN) *.o

kernel.bin :
	cp ../kernel.bin ./

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: pwd.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

pwd : pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

stackoverflow.o: stackoverflow.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

stackoverflow : stackoverflow.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?



counter1.o: counter1.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

counter1 : counter1.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?


counter2.o: counter2.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

counter2 : counter2.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

counter3.o: counter3.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

counter3 : counter3.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?


attack.o: attack.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

attack : attack.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

testa.o: testa.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

testa : testa.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?



