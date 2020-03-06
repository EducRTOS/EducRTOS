#include "user_tasks.h"

#define STRING(x) #x
#define XSTRING(x) STRING(x)

#define INCBIN(name, file) \
    asm(".section .data.task\n" \
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
INCBIN(task1, "task1.bin");

#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"


#define NB_TASKS 2
#include "system_desc.h"

/* Note: we could put this description at the beginning of each task. 
   It would make it easy to pass tasks on the command line. */
static const struct task_description tasks[] = {
  [0] = {
     .context = &system_contexts[0],
     .start_pc = 0,
     .task_begin = task0_begin,
     .task_end = task0_end,
     .period = 3000000,     
  },
  [1] = {
     .context = &system_contexts[1],
     .start_pc = 0,
     .task_begin = task1_begin,
     .task_end = task1_end,
     .period = 2000000,
  },
};


static struct context *ready_heap_array[NB_TASKS];
static struct context *waiting_heap_array[NB_TASKS];
  
const struct user_tasks_image user_tasks_image = {
  .nb_tasks = 2,
  .tasks = tasks,
  .low_level = {}/* low_level_description */,
  .ready_heap_array = &ready_heap_array,
  .waiting_heap_array = &waiting_heap_array,  
};
