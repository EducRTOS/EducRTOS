#ifndef __HIGH_LEVEL_H__
#define __HIGH_LEVEL_H__

#include "config.h"
#include "scheduler.h"
#include "timer.h"
#include "low_level.h"


/* High-level: parts of the kernel which are independent from the
   hardware architecture. */

struct context {
  /* Hardware context must come first. */
  struct hw_context hw_context;
  struct scheduling_context sched_context;
};

_Static_assert(__builtin_offsetof(struct context,hw_context) == 0,
               "Hardware context must be the first field to allow type casts");

/* Should be called once the low-level initialization is complete. */
void high_level_init(void);


void
high_level_timer_interrupt_handler(struct hw_context *cur_hw_ctx, date_t curtime);

/**************** For system description ****************/

#define HIGH_LEVEL_SYSTEM_DESC(NB_TASKS)                \
  struct context system_contexts[NB_TASKS];

#endif /* __HIGH_LEVEL_H__ */
