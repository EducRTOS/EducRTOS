#include "terminal.h"           /* For now. */
#include "user_tasks.h"

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
