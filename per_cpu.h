#ifndef __PER_CPU_H__
#define __PER_CPU_H__

#include "config.h"
#include "high_level.h"

struct per_cpu {
  /*struct context idle_ctx;*/
};

extern struct per_cpu per_cpu[NUM_CPUS];

#define current_cpu() 0

#endif
