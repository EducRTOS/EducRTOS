CC=gcc
#CC=clang

QEMU_OPTIONS= -d in_asm,int,cpu_reset,pcall,cpu -no-reboot -no-shutdown

QEMU_OPTIONS += -machine q35 # More recent hardware.
# QEMU_OPTIONS += -machine accel=kvm -cpu 'Nehalem' # Better CPU, but no logging anymore
# Support for TSC Deadline. But no log anymore..
#QEMU_OPTIONS += -cpu max -machine pc,kernel_irqchip=on,accel=kvm

LD_FLAGS= -nostdlib -ffreestanding
CFLAGS = -ffreestanding -O2 -Wall -Wextra -std=gnu11 -fno-pic -foptimize-sibling-calls # -mregparm=3
CFLAGS += -fno-asynchronous-unwind-tables # Disable generation of eh_frames.
CFLAGS += -fno-stack-protector
# CFLAGS += -msse2 # Better atomic operations, but does not work by default with Qemu.
# CFLAGS += -fwhole-program		  # Aggressive link-time optimisation.
CFLAGS += -g 			          # Debug annotations.

#QEMU_GDB=-s -S

KERNEL_FILES := low_level.c error.c high_level.c terminal.c lib/fprint.c pit_timer.c per_cpu.c # vga.c

include config.mk
CFLAGS += -D$(SCHEDULER)

ifeq ($(SCHEDULER),ROUND_ROBIN_SCHEDULING)
	KERNEL_FILES:=$(KERNEL_FILES) round_robin_scheduler.c
else
	KERNEL_FILES:=$(KERNEL_FILES) priority_scheduler.c
endif




M32 ?= -m32

all: system.exe system.objdump

.PHONY: qemu
qemu:   system_10tasks.exe # system_100tasks.exe system_manual.exe
	qemu-system-i386 $(QEMU_OPTIONS) $(QEMU_GDB) -kernel $< 2>&1 | tee out | tail -n 500
#	qemu-system-i386 $(QEMU_OPTIONS) $(QEMU_GDB) -kernel myos.exe -initrd task.bin 2>&1 | tee out | tail -n 500

# Compiles everything together in a single system
singlefile.c:	$(KERNEL_FILES)
	cat $(KERNEL_FILES) > singlefile.c

singlefile.i: singlefile.c $(KERNEL_FILES)
	gcc -m32 -E $(CFLAGS) singlefile.c > singlefile.i

singlefile.exe: singlefile.i system_desc.o
	$(CC) $(M32) $(LD_FLAGS) -Wl,-Tkernel.ld -o $@ $(CFLAGS) $^ -lgcc
	if grub-file --is-x86-multiboot $@; then echo multiboot confirmed; else  echo the file is not multiboot; fi

singlefile.qemu: singlefile.exe
	qemu-system-i386 $(QEMU_OPTIONS) $(QEMU_GDB) -kernel singlefile.exe 2>&1 | tee out | tail -n 500


system_desc_%tasks.o: task0.bin system_desc_%tasks.c
	$(CC) -c $(M32) $(CFLAGS) -fno-common system_desc_$*tasks.c

system_desc_%tasks.c: system_desc_gen
	./system_desc_gen $* > $@

system_desc_gen: system_desc_gen.ml
	ocamlc system_desc_gen.ml -o system_desc_gen

system%.exe: $(KERNEL_FILES) system_desc%.o # system_desc.o
	sed -e s/system_desc/system_desc$*/g kernel.ld.tpl > kernel.ld
	$(CC) $(M32) $(LD_FLAGS) -Wl,-Tkernel.ld -o $@ $(CFLAGS) $^ -lgcc
	if grub-file --is-x86-multiboot $@; then echo multiboot confirmed; else  echo the file is not multiboot; fi
	objdump -M intel -D $@ > system$*.objdump

system.objdump: system.exe
	objdump -M intel -D system.exe > system.objdump

system_desc_manual.o: task0.bin task1.bin task2.bin system_desc_manual.c
	$(CC) -c $(M32) $(CFLAGS) -fno-common system_desc_manual.c
# Note: we use -fno-common to force allocation of initialized data at the right place.

task0.exe: task.c lib/fprint.c user_task.ld
	$(CC) $(M32) $(LD_FLAGS) -Wl,-Tuser_task.ld -DTASK_NUMBER=0 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc
task1.exe: task.c lib/fprint.c user_task.ld
	$(CC) $(M32) $(LD_FLAGS) -Wl,-Tuser_task.ld -DTASK_NUMBER=1 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc
task2.exe: task.c lib/fprint.c user_task.ld
	$(CC) $(M32) $(LD_FLAGS) -Wl,-Tuser_task.ld -DTASK_NUMBER=2 -o $@ $(CFLAGS) task.c lib/fprint.c -lgcc

%.bin: %.exe
	objcopy -Obinary -j.all $*.exe $*.bin

.PHONY: clean
clean:
	rm -f *.exe *.bin *.o singlefile.c

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
