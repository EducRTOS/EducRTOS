#include "scheduler.h"
#include "high_level.h"
#include "user_tasks.h"

#ifdef ROUND_ROBIN_SCHEDULING
struct context *sched_choose_from(struct context *ctx){
  return ctx->sched_context.next;
}

void scheduler_init(void){
  unsigned int const nb_tasks = user_tasks_image.nb_tasks;

  /* Initialize the circular list of contexts. */  
  struct context * prev = user_tasks_image.tasks[nb_tasks - 1].context;
  for(unsigned int i = 0; i < nb_tasks; i++){
    struct task_description const *task = &user_tasks_image.tasks[i];    
    struct context *ctx = task->context;
    prev->sched_context.next = ctx;
    prev = task->context;
  }
}


#endif
