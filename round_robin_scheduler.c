#include "scheduler.h"
#include "user_tasks.h"
#include "high_level.h"

/* Execute all the tasks round-robin; do not care about time; none of
   the tasks ever block. */
void sched_set_waiting(struct context *ctx){ (void) ctx; }
void sched_wake_tasks(date_t curtime){ (void) curtime; }

struct context *sched_maybe_preempt(struct context *ctx){return ctx;}

/* Should be per-cpu. */
static struct context *current;

void scheduler_init(void){
  unsigned int const nb_tasks = user_tasks_image.nb_tasks;

  /* Initialize the circular list of contexts. */
  struct context *prev =  user_tasks_image.tasks[nb_tasks - 1].context;
  for (unsigned int i = 0; i < nb_tasks; i++)
    {
      struct task_description const *task = &user_tasks_image.tasks[i];
      prev->sched_context.next = task->context;
      prev = task->context;
    }
  current = user_tasks_image.tasks[0].context;
}

struct context *sched_choose_next(void){
  current = current->sched_context.next;
  return current;
}
