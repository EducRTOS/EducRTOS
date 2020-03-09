#ifndef __CONFIG_H__
#define __CONFIG_H__

/* If set, the GDT has a fixed size and the user and code descriptors
   are overwritten. */
#define FIXED_SIZE_GDT
#define NUM_CPUS 1


//#define DEADLINE_MONITORING
#if defined(DEADLINE_MONITORING)
#error "Not yet implemented"
#endif

#define FP_SCHEDULING 
/* #define EDF_SCHEDULING */

#if defined(FP_SCHEDULING) && defined(EDF_SCHEDULING)
#error "Cannot define two schedulers simultaneously"
#endif

#if !defined(FP_SCHEDULING) && !defined(EDF_SCHEDULING)
#error "Must define one scheduler"
#endif


#endif
