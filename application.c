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

static const struct task_description tasks[] = {
  [0] = {
     .context = &ctx0,
     .start_pc = (uint32_t) &test_userspace0,
     .start_sp = (uint32_t) &user_stack0[USER_STACK_SIZE],
  },
  [1] = {
     .context = &ctx1,
     .start_pc = (uint32_t) &test_userspace1,
     .start_sp = (uint32_t) &user_stack1[USER_STACK_SIZE],
  },
};

struct user_tasks_image user_tasks_image = {
  .nb_tasks = 2,
  .tasks = tasks,
};
