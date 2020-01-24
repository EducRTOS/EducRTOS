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
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

typedef uint64_t descriptor_t;

#define create_descriptor(/* uint32_t */ base,                          \
/* uint32_t */ limit,                                                   \
/* bool */ present, /* 1 if used */           \
/* unsigned int */ privilege, /* 0 = full privileged, 3 = unprivileged  */ \
/* bool */ normal_segment ,    /* 0 for TSS and LDT, 1 for normal segment */ \
/* enum permissions */ permissions,                                   \
/* bool */ accessed,                                                  \
/* bool */ granularity, /* 0 = par byte, 1 = par block de 4k */ \
/* bool */ size)  /* 0 = 16 bit, 1 = 32 bit */ \
  (( (uint64_t) limit & 0x000F0000) |                                   \
   ((base >> 16) & 0x000000FF) |                                        \
   (base & 0xFF000000) |                                        \
   ((accessed | (permissions << 1) | (normal_segment << 4) | (privilege << 6) | (present << 7)) << 8) | \
   (((size << 2) | (granularity << 3)) << 20)) << 32 |                  \
  (base << 16) |                                                        \
  (limit & 0x0000FFFF)

#define create_code_descriptor(base,limit,privilege,conforming,readable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(4 | (conforming << 1) | readable),accessed,granularity,size)

#define create_data_descriptor(base,limit,privilege,growdown,writable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(0 | (growdown << 1) | writable),accessed,granularity,size)

static const descriptor_t null_descriptor = create_descriptor(0,0,0,0,0,0,0,0,0);

/* Full access to all addresses for the kernel code and data. */
static const descriptor_t kernel_code_descriptor = create_code_descriptor(0,0xFFFFFFFF,0,0,1,0,1,1);
static const descriptor_t kernel_data_descriptor = create_data_descriptor(0,0xFFFFFFFF,0,0,1,0,1,1);


enum gdt_entries {
 NULL_SEGMENT_INDEX,           /* 0 */
 KERNEL_CODE_SEGMENT_INDEX,    /* 1 */
 KERNEL_DATA_SEGMENT_INDEX,    /* 2 */
};

static  descriptor_t gdt[] = {
 [NULL_SEGMENT_INDEX] = null_descriptor,
 [KERNEL_CODE_SEGMENT_INDEX] = kernel_code_descriptor,
 [KERNEL_DATA_SEGMENT_INDEX] = kernel_data_descriptor,
 [3] = null_descriptor                      /* For now. */
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





/* struct descriptor create_descriptor(uint32_t base, uint32_t limit, uint16_t flags){ */

  
/* } */


/* https://wiki.osdev.org/Getting_to_Ring_3 */

int ext;

void kernel_main(void) 
{
  /* Initialize terminal interface */
  terminal_initialize();

  terminal_writestring("Kernel start\n");

  /* Up to now we used the segments loaded by grub. Set up a new gdt
     and make sure that the segments use it. */
  lgdt(gdt,sizeof(gdt));
  terminal_writestring("After lgdt\n");

  reload_code_segment(0,false,KERNEL_CODE_SEGMENT_INDEX);
  terminal_writestring("after reload_cs\n");

  reload_data_segments(0,false,KERNEL_DATA_SEGMENT_INDEX);
  terminal_writestring("after reload data segments\n");
  //  while(1);

  /* terminal_writestring("before reload data segments\n"); */
  
  //reload_data_segments(0,false,KERNEL_DATA_SEGMENT_INDEX);
  
 
  /* /\* Newline support is left as an exercise. *\/ */
  /* terminal_writestring("Hello, kernel World!\n Hello again!\n"); */
}
