#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

#include "low_level.h"
#include "timer.h"

enum syscalls {
   SYSCALL_YIELD,
   SYSCALL_PUTCHAR,
   SYSCALL_NUMBER
   /* SYSCALL_SLEEP = 0x33 */
};

static inline void
yield(duration_t next_wakeup, duration_t next_deadline){
  uint32_t next_wakeup_high = (uint32_t) (next_wakeup >> 32ULL);
  uint32_t next_wakeup_low = (uint32_t) (next_wakeup & 0xFFFFFFFFULL);  

  uint32_t next_deadline_high = (uint32_t) (next_deadline >> 32ULL);
  uint32_t next_deadline_low = (uint32_t) (next_deadline & 0xFFFFFFFFULL);  

  syscall5(SYSCALL_YIELD, next_wakeup_high, next_wakeup_low, next_deadline_high, next_deadline_low);
}

static inline void putchar(unsigned char x){
  syscall2(SYSCALL_PUTCHAR, x);
}

#include "lib/fprint.h"
#define printf(...) fprint(putchar, __VA_ARGS__)

struct task_description {
  struct context * const context;
  uint32_t const start_pc;
  char* const task_begin;
  char* const task_end;
  duration_t period;
};

/* High-level description of the application. */
extern const struct user_tasks_image {
  unsigned int const nb_tasks;
  struct task_description const *const tasks;
  struct low_level_description const low_level;
  struct context ** const ready_heap_array;
  struct context ** const waiting_heap_array;  
} user_tasks_image;

/* Provided by the application */


#endif  /* __USER_TASKS_H__ */
