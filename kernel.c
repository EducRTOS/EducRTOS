#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "segment.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
//#if defined(__linux__)
//#error "You are not using a cross-compiler, you will most certainly run into trouble"
//#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
/* #if !defined(__i386__) */
/* #error "This tutorial needs to be compiled with a ix86-elf compiler" */
/* #endif */


/* The TSS; almost everything is unused when using software task switching. */
struct tss
{
   uint32_t unused_prev_tss;
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t unused_esp1;      
   uint32_t unused_ss1;
   uint32_t unused_esp2;
   uint32_t unused_ss2;
   uint32_t unused_cr3;
   uint32_t unused_eip;
   uint32_t unused_eflags;
   uint32_t unused_eax;
   uint32_t unused_ecx;
   uint32_t unused_edx;
   uint32_t unused_ebx;
   uint32_t unused_esp;
   uint32_t unused_ebp;
   uint32_t unused_esi;
   uint32_t unused_edi;
   uint32_t unused_es;         
   uint32_t unused_cs;        
   uint32_t unused_ss;        
   uint32_t unused_ds;        
   uint32_t unused_fs;       
   uint32_t unused_gs;         
   uint32_t unused_ldt;      
   uint16_t unused_trap;
   uint16_t unused_iomap_base;
} __packed;

/* TSS for the processors. */

#define NUM_CPUS 1
static struct tss tss_array[NUM_CPUS];


/* A note on x86 privilege.
   
   DPL = privilege in the data segment descriptor.
   RPL = privilege in the data segment selector. Allows to lower the privilege for some operations.
   CPL = privilege of th code segment selector, this is the current privilege level.
   There is a general protection fault if max(CPL,RPL) >= DPL. */

typedef uint64_t descriptor_t;

/* Size of descriptors. */
#define S16BIT 0
#define S32BIT 1

/* Granularity of descriptors: byte or 4k. */


#define create_descriptor(/* uint32_t */ base,                          \
/* uint32_t */ limit,                                                   \
/* bool */ present, /* 1 if used */           \
/* unsigned int */ privilege, /*  0 = full privileged, 3 = unprivileged  */ \
/* bool */ normal_segment ,   /* 0 for TSS and LDT, 1 for normal segment */ \
/* enum permissions */ permissions,                                   \
/* bool */ accessed,                                                  \
/* bool */ granularity, /* 0 = par byte, 1 = par block de 4k */ \
/* bool */ size)  /* 0 = 16 bit, 1 = 32 bit */ \
  (( (uint64_t) limit & 0x000F0000) |                                   \
   ((base >> 16) & 0x000000FF) |                                        \
   (base & 0xFF000000) |                                        \
   ((accessed | (permissions << 1) | (normal_segment << 4) | (privilege << 5) | (present << 7)) << 8) | \
   (((size << 2) | (granularity << 3)) << 20)) << 32 |                  \
  (base << 16) |                                                        \
  (limit & 0x0000FFFF)

#define create_code_descriptor(base,limit,privilege,conforming,readable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(4 | (conforming << 1) | readable),accessed,granularity,size)

#define create_data_descriptor(base,limit,privilege,growdown,writable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(0 | (growdown << 1) | writable),accessed,granularity,size)

#define create_tss_descriptor(base,limit,privilege,busy,granularity)    \
  create_descriptor(base,limit,1,privilege,0,(4 | 0 << 1 | busy),1,granularity,0)


static const descriptor_t null_descriptor = create_descriptor(0,0,0,0,0,0,0,0,0);

/* Full access to all addresses for the kernel code and data. */
static const descriptor_t kernel_code_descriptor = create_code_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);
static const descriptor_t kernel_data_descriptor = create_data_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);

/* Does not work because &tss0 is not a compile-time constant, it is split which cannot be done by the linker. */
//static const descriptor_t tss0_descriptor = create_tss_descriptor((&tss0),sizeof(tss0),0,1,0);

enum gdt_entries {
 NULL_SEGMENT_INDEX,
 KERNEL_CODE_SEGMENT_INDEX,
 KERNEL_DATA_SEGMENT_INDEX,
 USER_CODE_SEGMENT_INDEX, 
 USER_DATA_SEGMENT_INDEX, 
 TSS_SEGMENTS_FIRST_INDEX,
 NEXT = TSS_SEGMENTS_FIRST_INDEX + NUM_CPUS,
 SIZE_GDT
};

static  descriptor_t gdt[SIZE_GDT] = {
                              
 [NULL_SEGMENT_INDEX] = null_descriptor,
 [KERNEL_CODE_SEGMENT_INDEX] = kernel_code_descriptor,
 [KERNEL_DATA_SEGMENT_INDEX] = kernel_data_descriptor,
 [USER_CODE_SEGMENT_INDEX] = create_code_descriptor(0,0xFFFFFFFF,3,0,1,0,1,S32BIT),
 [USER_DATA_SEGMENT_INDEX] = create_data_descriptor(0,0xFFFFFFFF,3,0,1,0,1,S32BIT),
 /* We perform software task switching, but still need one TSS by processor. */
 // [TSS0_SEGMENT_INDEX] = 0       /* cannot be a compile-time constant. */
 
};

/* Address of the gdt, and size in bytes. */
static inline void lgdt(descriptor_t *gdt, int size)
{
  struct gdt_register {
    uint16_t limit; /* Maximum offset to access an entry in the GDT. */
    uint32_t base; } __attribute__((packed,aligned(8)));
  struct gdt_register gdtr;
  gdtr.limit = size;
  gdtr.base = (uint32_t) gdt;
  asm volatile ("lgdt %0": : "m" (gdtr) : "memory");
}


/* Because of how we manipulate the address, create_tss_descriptor
   cannot be a compile-time constant in C. */
static void register_tss_in_gdt(int idx, struct tss *tss, size_t size){
  descriptor_t tss_descriptor = create_tss_descriptor((uint32_t) tss,
                                                       (size <= 0x67? 0x67 : size),
                                                       3,0,0);
  /* The descriptor should not be busy if we want to load the tss. */
  gdt[idx] = tss_descriptor;
}


#define STACK_SIZE 1024
char new_stack[STACK_SIZE];


void test_userspace(void){
  terminal_writestring("Hello from userspace\n");
  while(1);
}


/* struct descriptor create_descriptor(uint32_t base, uint32_t limit, uint16_t flags){ */

  
/* } */


/* https://wiki.osdev.org/Getting_to_Ring_3 */


static inline void switch_to_userspace(uint32_t stack, uint32_t pc){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%es\n\
                 movw %%ax,  %%fs\n\
                 movw %%ax,  %%gs\n\
                 movw %%ax,  %%ds\n\
                 push %0         /* next ss     */ \n\
                 push %2         /* next esp    */ \n\
                 pushf           /* next eflags */ \n\
                 push %1         /* next cs     */ \n\
                 push %3         /* next eip    */ \n\
                 iret\n"             
                :
                : "i"(gdt_segment_selector(3, USER_DATA_SEGMENT_INDEX)),
                  "i"(gdt_segment_selector(3, USER_CODE_SEGMENT_INDEX)),
                  "r"(stack),
                  "r"(pc)
                : "memory","eax");
}



void kernel_main(void) 
{
  /* Initialize terminal interface */
  terminal_initialize();

  terminal_writestring("Kernel start\n");

  /* Up to now we used the segments loaded by grub. Set up a new gdt
     and make sure that the segments use it. */
  {
    lgdt(gdt,sizeof(gdt));
    terminal_writestring("After lgdt\n");

    load_code_segment(gdt_segment_selector(0,KERNEL_CODE_SEGMENT_INDEX));
    terminal_writestring("after load_cs\n");
    
    load_data_segments(gdt_segment_selector(0,KERNEL_DATA_SEGMENT_INDEX));
    terminal_writestring("after load data segments\n");
  }

  /* Set-up the tss and load the task register for CPU 0. */
  {
    for(int i = 0; i < NUM_CPUS; i++){
      register_tss_in_gdt(i + TSS_SEGMENTS_FIRST_INDEX, &tss_array[i], sizeof(tss_array[i]));
    }
    load_tr(gdt_segment_selector(0,TSS_SEGMENTS_FIRST_INDEX));
  }

  switch_to_userspace(&new_stack[STACK_SIZE - sizeof(uint32_t)],&test_userspace);

  


  
  /* Now I should enter ring 3, and make a syscall. */
  /* https://wiki.osdev.org/SYSENTER // quite simple, should support segmentation, and also works on AMD */
  /* Bof, non il faut faire un iret je pense. */

    /* https://manybutfinite.com/post/cpu-rings-privilege-and-protection/ */
    /* => Explique les privilege levels: CPL, DPL, RPL */
    
  /* TODO: Fixed number of tasks. We could prove correctness without a configuration. */
  
  //  while(1);

  /* terminal_writestring("before reload data segments\n"); */
  
  //reload_data_segments(0,false,KERNEL_DATA_SEGMENT_INDEX);


 
  /* /\* Newline support is left as an exercise. *\/ */
  /* terminal_writestring("Hello, kernel World!\n Hello again!\n"); */
}

/* MAYBE: rename kernel.c into context-switch.c; this is the low-level part of the kernel. */
