#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>

void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_write_uint32(uint32_t num);
#endif
