QEMU_OPTIONS= -d in_asm,int,cpu_reset,pcall,cpu -no-reboot -no-shutdown

QEMU_OPTIONS += -machine q35 # More recent hardware.
# Support for TSC Deadline. But no log anymore..
#QEMU_OPTIONS += -cpu max -machine pc,kernel_irqchip=on,accel=kvm

LD_FLAGS= -nostdlib -ffreestanding
CFLAGS = -ffreestanding -O2 -Wall -Wextra -std=gnu11 -fno-pic -foptimize-sibling-calls # -mregparm=3
CFLAGS += -fno-asynchronous-unwind-tables # Disable generation of eh_frames.
# CFLAGS += -msse2 # Better atomic operations, but does not work by default with Qemu.
# CFLAGS += -fwhole-program		  # Aggressive link-time optimisation.
CFLAGS += -g 			          # Debug annotations.

#QEMU_GDB=-s -S

KERNEL_FILES = low_level.c high_level.c terminal.c lib/fprint.c pit_timer.c
M32 ?= -m32

all: system.exe system.objdump

.PHONY: qemy
qemu:
	qemu-system-i386 $(QEMU_OPTIONS) $(QEMU_GDB) -kernel system.exe 2>&1 | tee out | tail -n 500
#	qemu-system-i386 $(QEMU_OPTIONS) $(QEMU_GDB) -kernel myos.exe -initrd task.bin 2>&1 | tee out | tail -n 500

# Compiles everything together in a single system
system.exe: $(KERNEL_FILES) system_desc.o
	gcc $(M32) $(LD_FLAGS) -T kernel.ld -o $@ $(CFLAGS) $(KERNEL_FILES) system_desc.o -lgcc
	if grub-file --is-x86-multiboot $@; then echo multiboot confirmed; else  echo the file is not multiboot; fi

system.objdump: system.exe
	objdump -M intel -D system.exe > system.objdump

system_desc.o: task0.bin task1.bin task2.bin system_desc.c
	gcc -c $(M32) $(CFLAGS) system_desc.c

task0.exe: task.c lib/fprint.c user_task.ld
	gcc $(M32) $(LD_FLAGS) -T user_task.ld -DTASK_NUMBER=0 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc
task1.exe: task.c lib/fprint.c user_task.ld
	gcc $(M32) $(LD_FLAGS) -T user_task.ld -DTASK_NUMBER=1 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc
task2.exe: task.c lib/fprint.c user_task.ld
	gcc $(M32) $(LD_FLAGS) -T user_task.ld -DTASK_NUMBER=2 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc

%.bin: %.exe
	objcopy -Obinary -j.all $*.exe $*.bin

.PHONY: clean
clean:
	rm *.exe *.bin *.o

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
