#include "user_tasks.h"

#ifndef TASK_NUMBER
#warning "You should define a TASK_NUMBER from the command line (default 0)"
#define TASK_NUMBER 0
#endif

#define USER_STACK_SIZE 1024
#define XSTRING(x) STRING(x)
#define STRING(x) #x

/* The tasks setup their stack themselves, in assembly. */
static char user_stack[USER_STACK_SIZE] __attribute__((used, aligned(16)));

asm("\
.global _start\n\
.type _start, @function\n\
_start:\n\
        /* Setup the stack */ \n\
	mov $(user_stack + " XSTRING(USER_STACK_SIZE) "), %esp\n\
        call test_userspace\n\
        jmp user_error_infinite_loop\n\
/* setup size of _start symbol. */\n\
.size _start, . - _start\n\
");

asm("\
.global user_error_infinite_loop\n\
.type user_error_infinite_loop, @function\n\
user_error_infinite_loop:\n\
        /* Infinite loop. */\n\
1:	hlt\n\
	jmp 1b\n\
");

void __attribute__((used))
test_userspace(void)  {
  putchar('0' + TASK_NUMBER);
  printf(" says hello %d\n", TASK_NUMBER);
  int i = 0;
  while(1){
    i++;
    printf("task" XSTRING(TASK_NUMBER) ": i=%d\n", i);
    yield((TASK_NUMBER + 2)*1000000000ULL,1000000000ULL);
    /* yield(3000000ULL,3000000ULL);     */
  }
  while(1);
}
