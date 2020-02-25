#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>

void terminal_initialize(void);
void terminal_print(char * format,...) __attribute__ ((format (printf, 1, 2)));
void terminal_writestring(const char* data);
void terminal_write_uint32(uint32_t num);
void terminal_putchar(unsigned char c);
#endif
