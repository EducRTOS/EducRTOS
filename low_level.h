/* Low-level (architecture-dependent) external (for export) header for
   x86 32bit architecture. */
#ifndef __LOW_LEVEL_H__
#define __LOW_LEVEL_H__

#include <stdint.h>

/* These types should not be manipulated directly, but the high-level
   context has to know their size. */

struct interrupt_frame
{
  uint32_t eip;
  uint32_t cs;
  uint32_t flags;
  uint32_t esp;
  uint32_t ss;
} __attribute__((packed));

struct pusha
{
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;                 /* Saved, but not restored by popa. */
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
} __attribute__((packed));


/* A segment descriptor is an entry in a GDT or LDT. */
typedef uint64_t segment_descriptor_t;

struct hw_context {
  /* The hardware context is restored with popa;iret. */
  struct pusha           regs;
  struct interrupt_frame iframe;
  /* Segment selectors are initialized once. They point to the same
     address range, but code and segment are different. */
  segment_descriptor_t code_segment;
  segment_descriptor_t data_segment;
} __attribute__((packed,aligned(4)));


void
hw_context_init(struct hw_context* ctx, int idx, uint32_t pc,
                uint32_t start_address, uint32_t end_address);

void __attribute__((noreturn))
hw_context_switch(struct hw_context* ctx);


#define SOFTWARE_INTERRUPT_NUMBER 0x27
/* We initialize the pic here, so 0x40...47 are for the master PIC,
   and 0x48...4F for the slave PIC. */
#define TIMER_INTERRUPT_NUMBER 0x40
#define SPURIOUS_TIMER_INTERRUPT_NUMBER 0x48

/**************** For use by user tasks. ****************/


/* We try to pass the arguments in register using the regparm(3) GCC
   calling convention; except that we do not use the first argument
   eax, as it will be use in the kernel to pass the hardware context.

   First argument (syscall number) is edx, second is ecx, third is
   ebx, fourth is esi, fifth is edi, sixth is ebp. */

static inline void
syscall1(uint32_t arg){
  asm volatile ("int %0": :
                "i"(SOFTWARE_INTERRUPT_NUMBER),
                "d"(arg)
                );
}

static inline void
syscall2(uint32_t arg1, uint32_t arg2){
  asm volatile ("int %0": :
                "i"(SOFTWARE_INTERRUPT_NUMBER),
                "d"(arg1),
                "c"(arg2)
                );
}


/**************** For use by system description. ****************/

#define NUM_CPUS 1

enum gdt_indices {
 NULL_SEGMENT_INDEX,
 KERNEL_CODE_SEGMENT_INDEX,
 KERNEL_DATA_SEGMENT_INDEX,
 TSS_SEGMENTS_FIRST_INDEX,
 NEXT = TSS_SEGMENTS_FIRST_INDEX + NUM_CPUS,
 FIXED_SIZE_GDT,
 START_USER_INDEX = FIXED_SIZE_GDT, /* (code,data descriptors). */
};

#define LOW_LEVEL_SYSTEM_DESC(NB_TASKS)                                         \
  segment_descriptor_t system_gdt[FIXED_SIZE_GDT + 2 * NB_TASKS];


#endif /* __LOW_LEVEL_H__ */
