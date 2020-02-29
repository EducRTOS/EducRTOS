#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

#include "low_level.h"

enum syscalls {
   SYSCALL_YIELD = 0x11,
   SYSCALL_PUTCHAR = 0x22,
   /* SYSCALL_SLEEP = 0x33 */
};

static inline void
yield(void){
  syscall1(SYSCALL_YIELD);
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
};

struct low_level_description {
  segment_descriptor_t * const system_gdt;
};


/* High-level description of the application. */
extern const struct user_tasks_image {
  unsigned int const nb_tasks;
  struct task_description const *const tasks;
  struct low_level_description const low_level;
  
} user_tasks_image;

/* Provided by the application */


#endif  /* __USER_TASKS_H__ */
