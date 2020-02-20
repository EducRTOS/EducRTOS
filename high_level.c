#include "high_level.h"
#include "terminal.h"
#include "user_tasks.h"


/* For now: the application is not separated. */
#include "application.c"


void __attribute__((fastcall,noreturn))
high_level_syscall(struct hw_context *cur_hw_ctx, int syscall_number){
  /* This works because hw_context must be the first field of a context. */
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0);
  struct context *cur_ctx = (struct context *) cur_hw_ctx;

  terminal_writestring("Calling interrupt\n");
  terminal_write_uint32(syscall_number);

  if(cur_ctx == &ctx0)
    hw_context_switch(&ctx1.hw_context);
  else
    hw_context_switch(&ctx0.hw_context);
    /* hw_context_switch(&cur_ctx->hw_context); */    
}


void context_init(struct context *ctx, uint32_t sp, uint32_t pc){
  hw_context_init(&ctx->hw_context, sp, pc);
}

void __attribute__((noreturn))
high_level_init(void){
  /* context_init(&ctx0, */
  /*              (uint32_t) &user_stack0[USER_STACK_SIZE - sizeof(uint32_t)], */
  /*              (uint32_t) &test_userspace0); */
  /* context_init(&ctx1, */
  /*              (uint32_t) &user_stack1[USER_STACK_SIZE - sizeof(uint32_t)], */
  /*              (uint32_t) &test_userspace1);   */
  for(unsigned int i = 0; i < user_tasks_image.nb_tasks; i++){
    struct task_description *task = &user_tasks_image.tasks[i];
    context_init(task->context, task->start_sp, task->start_pc);
  }

  hw_context_switch(&(ctx0.hw_context));
}
