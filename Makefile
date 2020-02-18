QEMU_OPTIONS= -d in_asm,int,cpu_reset,pcall,cpu -no-reboot -no-shutdown

LD_FLAGS= -nostdlib -ffreestanding
CFLAGS = -ffreestanding -O2 -Wall -Wextra -std=gnu99

FILES = interrupt.s terminal.c low_level.c application.c

all: $(FILES)
	gcc -m32 $(LD_FLAGS) -T linker.ld -o myos.bin $(CFLAGS) boot.s $(FILES)
	if grub-file --is-x86-multiboot myos.bin; then echo multiboot confirmed; else  echo the file is not multiboot; fi
	qemu-system-i386 $(QEMU_OPTIONS) -kernel myos.bin 2>&1 | tee out | tail -n 500



# Note: xorriso and mtools should be installed for grub-mkrescure to work.
# myos.iso:
# 	TMP=`mktemp -d`; mkdir -p $$TMP/boot/grub && cp grub.cfg $$TMP/boot/grub/grub.cfg && cp myos.bin $$TMP/boot/myos.bin && grub-mkrescue --verbose -o myos.iso $$TMP && rm -R $$TMP
# 	bochs -q 'boot:cdrom' 'ata0-slave: type=cdrom, path="myos.iso", status=inserted'
#	qemu-system-i386 -cdrom myos.iso

#	cp grub-rescue-floppy.img a.img
#	mdir -i a.img ::/
# disk images: we have some with grub-rescue-pc.
#	bochs -q 'boot:a' 'floppya: 1_44=a.img, status=inserted'
#  ata0-master: type=disk, path="c.img", mode=flat
#	qemu-system-i386 -d int -kernel myos.bin
#	bochs -q 'boot:c' 'ata0-master: type=disk, path="c.img", mode=flat'
#	bochs -q 'boot:c' 'ata0-master: type=disk, path="CorePlus.img", mode=flat'
