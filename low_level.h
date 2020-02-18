/* Low-level (architecture-dependent) external (for export) header for
   x86 32bit architecture. */
#ifndef __LOW_LEVEL_H__
#define __LOW_LEVEL_H__

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
} __attribute__((packed));

#endif /* __LOW_LEVEL_H__ */