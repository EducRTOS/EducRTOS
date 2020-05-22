#include "high_level.h"
#include "terminal.h"
#include "user_tasks.h"
#include "per_cpu.h"

/* Conversion from hw_context to context works because of this. */
_Static_assert(__builtin_offsetof(struct context,hw_context) == 0,
               "Hardware context must be the first field to allow type casts");

/* Prepare the arguments for the call. This prologue will be the same
   for all functions that take 5 arguments. */
extern void syscall_yield(void);
asm("\
.global syscall_yield\n\t\
.type syscall_yield, @function\n\
syscall_yield:\n\
        push %edi\n\
        push %esi\n\
        call c_syscall_yield\n\
.size syscall_yield, . - syscall_yield\n\
");


void __attribute__((regparm(3),noreturn,used)) 
c_syscall_yield(struct context *ctx,
                uint32_t next_wakeup_high, uint32_t next_wakeup_low,
                uint32_t next_deadline_high, uint32_t next_deadline_low){
  /* terminal_print("Syscall yield %x\n", ctx); */
  /* terminal_print("%d|%x %x %x %x\n", next_wakeup_high, next_wakeup_low, */
  /*        next_deadline_high, next_deadline_low); */
  duration_t incr_wakeup = ((uint64_t) next_wakeup_high << 32) + (uint64_t) next_wakeup_low;
  ctx->sched_context.wakeup_date += incr_wakeup;
#if defined(EDF_SCHEDULING) || defined (DEADLINE_MONITORING)
  duration_t incr_deadline = ((uint64_t) next_deadline_high << 32) + (uint64_t) next_deadline_low;
  ctx->sched_context.deadline = ctx->sched_context.wakeup_date + incr_deadline;
#else
  /* Suppress warnings. */
  (void) next_deadline_high; (void) next_deadline_low;
#endif  
  sched_set_waiting(ctx);
  struct context *new_ctx  = sched_choose_next();
  hw_context_switch(&new_ctx->hw_context);
}

void __attribute__((regparm(3),noreturn,used)) 
syscall_putchar(struct context *ctx, int arg1) {
  /* terminal_print("Syscall putchar %x\n", ctx); */
  terminal_putchar(arg1);
  hw_context_switch(&ctx->hw_context);
}

void * const syscall_array[SYSCALL_NUMBER] __attribute__((used)) = {
  [SYSCALL_YIELD] = syscall_yield,
  [SYSCALL_PUTCHAR] = syscall_putchar,
};

void __attribute__((noreturn,used))
high_level_timer_interrupt_handler(struct hw_context *cur_hw_ctx, date_t curtime){
  _Static_assert(__builtin_offsetof(struct context,hw_context) == 0,
                 "Hardware context must be the first field to allow type casts");
  struct context *cur_ctx = (struct context *) cur_hw_ctx;
  /* terminal_print("interrupt handler %x\n", cur_ctx); */
  sched_wake_tasks(curtime);
  struct context *new_ctx;
  if(cur_ctx == &per_cpu[current_cpu()].idle_ctx) {
    new_ctx = sched_choose_next();
  }
  else {
    new_ctx = sched_maybe_preempt(cur_ctx);
  }
  hw_context_switch(&new_ctx->hw_context);
}

void context_init(struct context * const ctx, int idx,
                  uint32_t pc,
                  uint32_t start, uint32_t end) {
  (void) idx;
  hw_context_init(&ctx->hw_context, pc, start, end);
}

void __attribute__((noreturn))
high_level_init(void){
  unsigned int const nb_tasks = user_tasks_image.nb_tasks;
  
  for(unsigned int i = 0; i < nb_tasks; i++){
    struct task_description const *task = &user_tasks_image.tasks[i];
    context_init(task->context, i, task->start_pc,
                 (uint32_t) task->task_begin, (uint32_t) task->task_end);
  }

  for(int i =0; i < NUM_CPUS; i++ ){
    struct context * ctx = &per_cpu[i].idle_ctx;
    ctx->sched_context.wakeup_date = 0ULL;
#if defined(EDF_SCHEDULING) || defined(DEADLINE_MONITORING)
    ctx->sched_context.deadline = 0xFFFFFFFFFFFFFFFFULL;
#endif    
    hw_context_idle_init(&ctx->hw_context);
  }
  scheduler_init();

  struct context *new_ctx = sched_choose_next();
  /* terminal_print("Next is %x\n", new_ctx); */
  hw_context_switch(&new_ctx->hw_context);  
}
