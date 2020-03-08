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
	cli\n\
1:	hlt\n\
	jmp 1b\n\
");

/* static void puts(void (*putchar)(unsigned char), char *string){ */
/*   while(*string) putchar(*string++); */
/* } */

void __attribute__((used))
test_userspace(void)  {

  yield(0x1122334455667788ULL,0xaabbccddeeff0011ULL);
  
  /* puts(putchar, "Hello from user task " XSTRING(TASK_NUMBER) "\n"); */
  
  putchar('0' + TASK_NUMBER);

  printf(" Hello from task%d\n", TASK_NUMBER);
  
  /* putchar('A' + TASK_NUMBER); */
  
  for(int i = 0; i < 3; i++){
    /* putchar('0' + TASK_NUMBER);     */
    printf("task" XSTRING(TASK_NUMBER) ": i=%d\n", i);
    yield(1000000000ULL,2000000000ULL);
  }
  while(1);
}
