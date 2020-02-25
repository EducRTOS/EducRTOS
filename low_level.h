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

/* The hardware context is restored with popa;iret; */
struct hw_context {
  struct pusha           regs;
  struct interrupt_frame iframe;
} __attribute__((packed,aligned(4)));


void
hw_context_init(struct hw_context* ctx, uint32_t stack, uint32_t pc);

void __attribute__((noreturn))
hw_context_switch(struct hw_context* ctx);


#define SOFTWARE_INTERRUPT_NUMBER 0x27

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

#endif /* __LOW_LEVEL_H__ */
