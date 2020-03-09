#ifndef __HIGH_LEVEL_H__
#define __HIGH_LEVEL_H__

#include "config.h"
#include "scheduler.h"
#include "low_level.h"


/* High-level: parts of the kernel which are independent from the
   hardware architecture. */

struct context {
  struct hw_context hw_context;
  /* Round-robin scheduling. */
  /* struct context *next; */
  struct scheduling_context sched_context;
};

_Static_assert(__builtin_offsetof(struct context,hw_context) == 0);

/* Called upon syscalls. */
void __attribute__((regparm(3),noreturn))
high_level_syscall(struct hw_context *cur_ctx, int syscall_number, int arg1);



/* Should be called once the low-level initialization is complete. */
void high_level_init(void);


void
high_level_timer_interrupt_handler(struct hw_context *cur_hw_ctx, date_t curtime);

/**************** For system description ****************/

#define HIGH_LEVEL_SYSTEM_DESC(NB_TASKS)                \
  struct context system_contexts[NB_TASKS];

#endif /* __HIGH_LEVEL_H__ */
