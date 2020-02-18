#include "terminal.h"           /* For now. */
#include "user_tasks.h"
#include "high_level.h"

#define USER_STACK_SIZE 1024
static char user_stack0[USER_STACK_SIZE] __attribute__((aligned(16)));
static char user_stack1[USER_STACK_SIZE] __attribute__((aligned(16)));

struct context ctx0, ctx1;

void test_userspace0(void){
  for(int i = 0; i < 3; i++){
    terminal_writestring("Hello from userspace0 ");
    terminal_write_uint32(i);
    terminal_writestring("\n");
    yield();
    terminal_writestring("Ret\n");
  }
  while(1);
}


void test_userspace1(void){
  for(int i = 0x10; i < 0x13; i++){
    terminal_writestring("Hello from userspace1 ");
    terminal_write_uint32(i);
    terminal_writestring("\n");
    yield();
    terminal_writestring("Ret\n");
  }
  while(1);
}
