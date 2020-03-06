#include "high_level.h"
#include "terminal.h"
#include "user_tasks.h"

void __attribute__((regparm(3),noreturn,used))
high_level_syscall(struct hw_context *cur_hw_ctx, int syscall_number, int arg1){
  /* This works because hw_context must be the first field of a context. */
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0);
  struct context *cur_ctx = (struct context *) cur_hw_ctx;

  /* terminal_print("ctx %x syscall number %x", cur_ctx, syscall_number); */
  /* terminal_print("Calling interrupt %x\n", syscall_number); */
  /* terminal_print("Calling interrupt %x\n", arg1); */
  /* terminal_print("Calling interrupt %x\n", cur_hw_ctx); */
  switch(syscall_number){
  case SYSCALL_YIELD:
    cur_ctx = sched_choose_from(cur_ctx);
    /* hw_context_switch(&cur_ctx->next->hw_context); */
    break;
  case SYSCALL_PUTCHAR:
    terminal_putchar(arg1);
    /* terminal_print("After putchar (arg1 %x)\n", arg1); */
    /* while(1);     */
    break;
  }
  hw_context_switch(&cur_ctx->hw_context);
}

void __attribute__((noreturn,used))
high_level_timer_interrupt_handler(struct hw_context *cur_hw_ctx){
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0);
  struct context *cur_ctx = (struct context *) cur_hw_ctx;

  while(1);
}


void context_init(struct context * const ctx, int idx,
                  uint32_t pc,
                  uint32_t start, uint32_t end) {
  hw_context_init(&ctx->hw_context, idx, pc, start, end);
}

void __attribute__((noreturn))
high_level_init(void){
  unsigned int const nb_tasks = user_tasks_image.nb_tasks;
  
  for(unsigned int i = 0; i < nb_tasks; i++){
    struct task_description const *task = &user_tasks_image.tasks[i];
    context_init(task->context, i, task->start_pc,
                 (uint32_t) task->task_begin, (uint32_t) task->task_end);
  }

  scheduler_init();
  
  hw_context_switch(&user_tasks_image.tasks[0].context->hw_context);
}
