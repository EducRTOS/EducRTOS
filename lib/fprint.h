/* A helper function to print strings, with no buffering, one
   character at a time. */

void __attribute__ ((format (printf, 2, 3)))
fprint(int (*putchar)(int), char * format,...);


