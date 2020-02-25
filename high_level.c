#include "high_level.h"
#include "terminal.h"
#include "user_tasks.h"

void __attribute__((regparm(3),noreturn))
high_level_syscall(struct hw_context *cur_hw_ctx, int syscall_number, int arg1){
  /* This works because hw_context must be the first field of a context. */
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0);
  struct context *cur_ctx = (struct context *) cur_hw_ctx;
  terminal_print("Calling interrupt %x\n", syscall_number);
  hw_context_switch(&cur_ctx->next->hw_context);
}


void context_init(struct context *ctx, uint32_t pc,
                  struct context *prev){
  hw_context_init(&ctx->hw_context, 0xabcdef00, pc);
  prev->next = ctx;
}

void __attribute__((noreturn))
high_level_init(void){
  unsigned int nb_tasks = 2/* user_tasks_image.nb_tasks */;

  /* Initialize the circular list of contexts. */
  struct context *prev = user_tasks_image.tasks[nb_tasks - 1].context;
  for(unsigned int i = 0; i < nb_tasks; i++){
    struct task_description *task = &user_tasks_image.tasks[i];
    context_init(task->context, task->start_pc, prev);
    prev = task->context;
  }
  hw_context_switch(&user_tasks_image.tasks[0].context->hw_context);
}
