#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"

#define USER_STACK_SIZE 1024
static char user_stack[USER_STACK_SIZE] __attribute__((aligned(16)));

struct context ctx0;

void test_userspace(void){
  for(int i = 0; i < 3; i++){
    terminal_writestring("Hello from userspace ");
    terminal_write_uint32(i);
    terminal_writestring("\n");
    yield();
    terminal_writestring("Ret\n");
  }
  while(1);
}
