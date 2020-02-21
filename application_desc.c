#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"

#define USER_STACK_SIZE 1024
/* static char user_stack0[USER_STACK_SIZE] __attribute__((aligned(16))); */
/* static char user_stack1[USER_STACK_SIZE] __attribute__((aligned(16))); */
struct context ctx0, ctx1;
/* extern void test_userspace0(void); */
/* extern void test_userspace1(void); */

extern void _start0(void);
extern void _start1(void);

static const struct task_description tasks[] = {
  [0] = {
     .context = &ctx0,
     .start_pc = (uint32_t) &_start0,
     /* .start_sp = (uint32_t) &user_stack0[USER_STACK_SIZE], */
  },
  [1] = {
     .context = &ctx1,
     .start_pc = (uint32_t) &_start1,
     /* .start_sp = (uint32_t) &user_stack1[USER_STACK_SIZE], */
  },
};

const struct user_tasks_image user_tasks_image = {
  .nb_tasks = 2,
  .tasks = tasks,
};
