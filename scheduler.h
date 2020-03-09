#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "config.h"
#include "timer.h"

struct context;

/* Block a task, i.e. set it as waiting (for a time event.) */
void sched_set_waiting(struct context *ctx);

/* Change the status (to ready) of all tasks that are now ready. */
void sched_wake_tasks(date_t curtime);

/* Pass to the scheduler the current context; either return it, or
   return one with higher priority if a preemption is needed. */
struct context *sched_maybe_preempt(struct context *ctx);

/* Return the next context to execute (may be the idle context). */
struct context *sched_choose_next(void);

/* Initialize the scheduler. */
void scheduler_init(void);


struct scheduling_context {
  date_t wakeup_date;           /* If active, last time it was awaken. If inactive: next time. */
  date_t deadline;
#ifdef FP_SCHEDULING
  unsigned int priority;
#endif  
  
};

#endif
