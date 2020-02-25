#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"

struct context ctx0, ctx1;

extern void _start0(void);
extern void _start1(void);

static const struct task_description tasks[] = {
  [0] = {
     .context = &ctx0,
     .start_pc = (uint32_t) &_start0,
  },
  [1] = {
     .context = &ctx1,
     .start_pc = (uint32_t) &_start1,
  },
};

const struct user_tasks_image user_tasks_image = {
  .nb_tasks = 2,
  .tasks = tasks,
};
