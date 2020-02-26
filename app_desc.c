#define STRING(x) #x
#define XSTRING(x) STRING(x)

#define INCBIN(name, file) \
    asm(".section .rodata\n" \
            ".global " XSTRING(name) "_begin\n" \
            ".type " XSTRING(name) "_begin, @object\n" \
            ".balign 16\n" \
            XSTRING(name) "_begin:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global " XSTRING(name) "_end\n" \
            ".type " XSTRING(name) "_end, @object\n" \
            ".balign 1\n" \
            XSTRING(name) "_end:\n" \
            ".byte 0\n" \
    ); \
    extern __attribute__((aligned(16))) char name ## _begin[]; \
    extern char name ## _end[]; \

INCBIN(task0, "task0.bin");
/* INCBIN(task1, "task1.bin"); */

#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"

struct context ctx0, ctx1;

static const struct task_description tasks[] = {
  [0] = {
     .context = &ctx0,
     .start_pc = 0,
     .task_begin = task0_begin,
     .task_end = task0_end,     
  },
  /* [1] = { */
  /*    .context = &ctx0, */
  /*    .start_pc = 0, */
  /*    .task_begin = task1_begin, */
  /*    .task_end = task1_end,      */
  /* }, */
};

const struct user_tasks_image user_tasks_image = {
  .nb_tasks = 1,
  .tasks = tasks,
};
