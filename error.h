#ifndef __ERROR_H__
#define __ERROR_H__

/* Error handling functions. */

void __attribute__((noreturn))
error_infinite_loop(void);

void __attribute__ ((format (printf, 1, 2)))
fatal(char * format,...);

#define assert(x) if(!(x)) fatal("Assertion " #x " failed")


#endif
