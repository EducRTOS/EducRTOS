/* Low-level (architecture-dependent) external (for export) header for
   x86 32bit architecture. */
#ifndef __LOW_LEVEL_H__
#define __LOW_LEVEL_H__

#include "config.h"
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
#ifdef FIXED_SIZE_GDT  
  /* Segment selectors are initialized once. They point to the same
     address range, but code and segment are different. */
  segment_descriptor_t code_segment;
  segment_descriptor_t data_segment;
#endif  
} __attribute__((packed,aligned(4)));


void
hw_context_init(struct hw_context* ctx, uint32_t pc,
                uint32_t start_address, uint32_t end_address);

void
hw_context_idle_init(struct hw_context* ctx);


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


static inline void
syscall5(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5){
  asm volatile ("int %0": :
                "i"(SOFTWARE_INTERRUPT_NUMBER),
                "d"(arg1),
                "c"(arg2),
                "b"(arg3),
                "S"(arg4),
                "D"(arg5)                
                );
}



/**************** For use by system description. ****************/

#define NUM_CPUS 1

struct user_task_descriptors {
  segment_descriptor_t code_descriptor;
  segment_descriptor_t data_descriptor;  
} __attribute__((packed));

struct system_gdt {
  segment_descriptor_t null_descriptor;
  segment_descriptor_t kernel_code_descriptor;
  segment_descriptor_t kernel_data_descriptor;
#ifdef FIXED_SIZE_GDT
  segment_descriptor_t user_code_descriptor;
  segment_descriptor_t user_data_descriptor;
#endif  
  segment_descriptor_t tss_descriptor[NUM_CPUS];
#ifndef FIXED_SIZE_GDT  
  struct user_task_descriptors user_task_descriptors[]; /* One per task */
#endif  
} __attribute__((packed));

struct low_level_description {
#ifndef FIXED_SIZE_GDT  
  struct system_gdt * const system_gdt;
#endif
};

#define KERNEL_CODE_SEGMENT_INDEX \
  (offsetof(struct system_gdt,kernel_code_descriptor)/sizeof(segment_descriptor_t))
#define KERNEL_DATA_SEGMENT_INDEX \
  (offsetof(struct system_gdt,kernel_data_descriptor)/sizeof(segment_descriptor_t))
#define TSS_SEGMENTS_FIRST_INDEX \
  (offsetof(struct system_gdt,tss_descriptor)/sizeof(segment_descriptor_t))
#ifndef FIXED_SIZE_GDT
#define START_USER_INDEX \
  (offsetof(struct system_gdt,user_task_descriptors)/sizeof(segment_descriptor_t))
#else
#define USER_CODE_SEGMENT_INDEX \
  (offsetof(struct system_gdt,user_code_descriptor)/sizeof(segment_descriptor_t))
#define USER_DATA_SEGMENT_INDEX \
  (offsetof(struct system_gdt,user_data_descriptor)/sizeof(segment_descriptor_t))
#endif



/* #define START_USER_INDEX (sizeof(struct system_gdt)/sizeof(segment_descriptor_t)) */


#ifdef FIXED_SIZE_GDT
#define SYSTEM_GDT(NB_TASKS) struct system_gdt system_gdt;
#define SYSTEM_GDT_FIELD 
#else
#define SYSTEM_GDT(NB_TASKS)                                            \
  struct {                                                              \
  struct system_gdt begin;                                              \
  struct user_task_descriptors desc[NB_TASKS]; } __attribute__((packed)) system_gdt;
#define SYSTEM_GDT_FIELD .system_gdt = (struct system_gdt *) &system_gdt,
#endif


#define LOW_LEVEL_SYSTEM_DESC(NB_TASKS)                                 \
  SYSTEM_GDT(NB_TASKS);                                                 \
  static const struct low_level_description low_level_description =     \
    { SYSTEM_GDT_FIELD };

#endif /* __LOW_LEVEL_H__ */
