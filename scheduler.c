#include "scheduler.h"
#include "high_level.h"

#ifdef ROUND_ROBIN_SCHEDULING
struct context *sched_choose_from(struct context *ctx){
  return ctx->sched_context.next;
}
#endif
