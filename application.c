#include "user_tasks.h"

#define USER_STACK_SIZE 1024
#define XSTRING(x) STRING(x)
#define STRING(x) #x

/* The tasks setup their stack themselves, in assembly. */
char user_stack0[USER_STACK_SIZE] __attribute__((aligned(16)));
char user_stack1[USER_STACK_SIZE] __attribute__((aligned(16)));


asm("\
.global _start0\n\
.type _start0, @function\n\
_start0:\n\
        /* Setup the stack */ \n\
	mov $(user_stack0 + " XSTRING(USER_STACK_SIZE) "), %esp\n\
        call test_userspace0\n\
        jmp user_error_infinite_loop\n\
/* setup size of _start symbol. */\n\
.size _start0, . - _start0\n\
");

asm("\
.global _start1\n\
.type _start1, @function\n\
_start1:\n\
        /* Setup the stack */ \n\
	mov $(user_stack1 + " XSTRING(USER_STACK_SIZE) "), %esp\n\
        call test_userspace1\n\
        jmp user_error_infinite_loop\n\
/* setup size of _start symbol. */\n\
.size _start1, . - _start1\n\
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


/* TODO: When in userspace, I will not be able to write directly, so I
   need to call putchar instead. */

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
