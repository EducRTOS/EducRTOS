#ifndef __CONFIG_H__
#define __CONFIG_H__

/* If set, the GDT has a fixed size, the user and code descriptors are
   set in the context and overwritten on context switches. */
/* #define FIXED_SIZE_GDT */

/* If set, the GDT has a fixed size, and the descriptors are created
   dynamically on context switch. This is necessary for dynamic task
   creation. */
#define DYNAMIC_DESCRIPTORS

/* If nothing is set, the GDT has a parametric size and the user and
   code descriptors are written once at boot time. */
#define NUM_CPUS 1


//#define DEADLINE_MONITORING
#if defined(DEADLINE_MONITORING)
#error "Not yet implemented"
#endif

/* #define FP_SCHEDULING */
/* #define EDF_SCHEDULING */
/* #define ROUND_ROBIN_SCHEDULING */

#if !defined(FP_SCHEDULING) && !defined(EDF_SCHEDULING) && !defined(ROUND_ROBIN_SCHEDULING)
#error "Must define one scheduler"
#endif

#if defined(FP_SCHEDULING) && defined(EDF_SCHEDULING)
||  defined(FP_SCHEDULING) && defined(ROUND_ROBIN_SCHEDULING)
||  defined(EDF_SCHEDULING) && defined(ROUND_ROBIN_SCHEDULING)  
#error "Cannot define two schedulers simultaneously"
#endif



#endif
