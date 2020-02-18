#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

#include "low_level.h"

enum syscalls {
   SYSCALL_YIELD = 0x11,
   /* SYSCALL_PUTCHAR = 0x22, */
   /* SYSCALL_SLEEP = 0x33 */
};

static inline void
yield(void){
  syscall1(SYSCALL_YIELD);
}

#endif  /* __USER_TASKS_H__ */
