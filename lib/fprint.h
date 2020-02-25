#ifndef  __FPRINT_H__
#define  __FPRINT_H__

/* A helper function to print strings, with no buffering, one
   character at a time. */

void __attribute__ ((format (printf, 2, 3)))
fprint(void (*putchar)(unsigned char), char * format,...);

#endif
