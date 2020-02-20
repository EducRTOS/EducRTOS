#ifndef __HIGH_LEVEL_H__
#define __HIGH_LEVEL_H__

#include "low_level.h"

/* High-level: parts of the kernel which are independent from the
   hardware architecture. */

struct context {
  struct hw_context hw_context;
};

_Static_assert(__builtin_offsetof(struct context,hw_context) == 0);

/* Called upon syscalls. */
void __attribute__((fastcall,noreturn))
high_level_syscall(struct hw_context *cur_ctx, int syscall_number);



/* Should be called once the low-level initialization is complete. */
void high_level_init(void);



#endif /* __HIGH_LEVEL_H__ */
