#include "high_level.h"

void __attribute__((fastcall,noreturn))
high_level_syscall(struct hw_context *cur_hw_ctx, int syscall_number){
  /* This works because hw_context must be the first field of a context. */
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0);
  struct context *cur_ctx = (struct context *) cur_hw_ctx;
}

/* For now: the application is not separated. */
#include "application.c"

void context_init(struct context *ctx, uint32_t sp, uint32_t pc){
  hw_context_init(&ctx->hw_context, sp, pc);
}

void __attribute__((noreturn))
high_level_init(void){
  context_init(&ctx0,
               (uint32_t) &user_stack[USER_STACK_SIZE - sizeof(uint32_t)],
               (uint32_t) &test_userspace);
  hw_context_switch(&(ctx0.hw_context));
}
