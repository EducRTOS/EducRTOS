#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "config.h"

struct context *sched_choose_from(struct context *ctx);

#ifdef ROUND_ROBIN_SCHEDULING
struct scheduling_context {
  struct context *next;
};

#endif


#endif
