KERNELENTRY	=	0x30400

ASM		=	nasm
CC		=	gcc
LD		=	ld
ASMBFLAGS	=	-I boot/include/
ASMKFLAGS	= 	-I include/ -f elf
CFLAGS		= 	-I include/ -c -fno-builtin -fno-stack-protector
LDFLAGS		=	-s -Ttext $(KERNELENTRY)

BOOTBIN		=	boot/boot.bin
LOADERBIN	=	boot/loader.bin
KERNELBIN	=	kernel/kernel.bin
OBJS		=	kernel/kernel.o kernel/global.o kernel/start.o kernel/main.o kernel/proc.o kernel/syscall.o kernel/i8259.o kernel/keyboard.o kernel/protect.o kernel/clock.o lib/klib.o lib/string.o lib/kliba.o kernel/tty.o  kernel/console.o kernel/printf.o kernel/vsprintf.o
IMG		=	a.img

.PHONY:	all everything buildimg clean

all:  everything buildimg

everything: 	$(BOOTBIN) $(LOADERBIN) $(KERNELBIN)

buildimg:
	cp img/a.img $(IMG)
	dd if=$(BOOTBIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) /mnt/floppy/ -t msdos
	sudo cp $(LOADERBIN) /mnt/floppy/ -v
	sudo cp $(KERNELBIN) /mnt/floppy/ -v
	sudo umount /mnt/floppy

$(BOOTBIN): boot/boot.asm boot/include/fat12h.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(LOADERBIN): boot/loader.asm boot/include/loader16.inc boot/include/loader32.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(KERNELBIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNELBIN) $(OBJS)

kernel/kernel.o: kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/global.o: kernel/global.c include/type.h include/const.h include/protect.h include/proc.h include/kliba.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/main.o: kernel/main.c include/type.h include/const.h include/protect.h include/kliba.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/start.o: kernel/start.c  include/type.h include/const.h include/string.h include/kliba.h include/protect.h include/proc.h include/kliba.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o:kernel/i8259.c include/type.h include/const.h include/kliba.h include/protect.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o:kernel/protect.c include/type.h include/const.h include/kliba.h include/proc.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<
kernel/clock.o: kernel/clock.c 
	$(CC) $(CFLAGS) -o $@ $<
kernel/keyboard.o: kernel/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<
kernel/printf.o: kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<
kernel/vsprintf.o: kernel/vsprintf.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o:kernel/proc.c
	$(CC) $(CFLAGS) -o $@ $<
kernel/syscall.o:kernel/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<
lib/klib.o: lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<
lib/kliba.o: lib/klib.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(IMG)
