/* https://en.wikibooks.org/wiki/X86_Assembly/Programmable_Interval_Timer */
/* https://wiki.osdev.org/Programmable_Interval_Timer#PIT_Channel_0_Example_Code */
/* https://en.wikipedia.org/wiki/Intel_8253#Operation_modes */

#include "timer.h"
#include "config.h"

#define PIT_HZ 1193182             /* The PIT fixed frequency */
#define WANTED_HZ 1000             /* A tick every ms. */
#define DIVISOR (PIT_HZ/WANTED_HZ) /* By dividing by that, we have the wanted tick frequency. */
#define TICK 1000ULL               /* The time, in nano seconds, between ticks. */

/* DIVISOR must be held on 16 bits. */
_Static_assert(DIVISOR < (1 << 16));


#include <stdint.h>
#include "low_level.h"
#include <stdatomic.h> 
#include "x86/port.h"



void init_apic(void){
  /* We do not use the APIC for now. But we can use the PIT with the
     APIC. */
}

static const uint16_t master_pic_command = 0x20;
static const uint16_t master_pic_data = 0x21;
static const uint16_t slave_pic_command = 0xA0;
static const uint16_t slave_pic_data = 0xA1;


/* https://wiki.osdev.org/8259_PIC */
void init_pic(void){

  /* Initialize. Wait for three words on the data port.*/
  outb(master_pic_command, 0x11);
  outb(slave_pic_command, 0x11);  

  /* Change the interrupt number from defaults 0x08 and 0x70. */
  outb(master_pic_data, TIMER_INTERRUPT_NUMBER);
  outb(slave_pic_data, SPURIOUS_TIMER_INTERRUPT_NUMBER);

  /* Connect master and slave PIC. */
  outb(master_pic_data, 4);
  outb(slave_pic_data, 2);

  /* Require explicit end of interrupt. */
  outb(master_pic_data, 1);
  outb(slave_pic_data, 1);

  uint8_t activate_none = 0xFF;
  uint8_t activate_irq0 = 0xFE;

  /* If we want to disable the PIC, we put activate_none here. */
  outb(master_pic_data, activate_irq0);
  outb(slave_pic_data, activate_none);  

  
}

static const uint16_t pit_command = 0x43;
static const uint16_t pit_data = 0x40;

#if NUM_CPUS == 1
/* Time from the boot, in nano-seconds. */
static /* _Atomic */uint64_t current_time;
/* No need to read/write time atomically on single core, and if we
   disable interrupts in the kernel.  */

uint64_t timer_current_time(void){
  return *(&current_time);
}
#else
static _Atomic(uint64_t) current_time;

uint64_t timer_current_time(void){
  return atomic_load(&current_time);
}

#endif

void timer_init(void){
  current_time = 0;

  /* Just send the divisor to the PIT. */
  outb(pit_command, 0x34);
  outb(pit_data, DIVISOR & 0xFF);
  outb(pit_data, DIVISOR >> 8);
}




static uint64_t next_wake_date = DATE_FAR_AWAY;

static int count;

void __attribute__((regparm(3),noreturn,used))
timer_interrupt_handler(struct hw_context *cur_hw_ctx){
  /* Acknowledge interrupt. */
  outb(master_pic_command, 0x20);

  /* This instance is the only one changing the time. So we do not
     need to be atomic. */
  uint64_t cur = *(&current_time);
  cur += TICK;
  *(&current_time) = cur;

  /* Temporary: write a & every second, to show that it is working. */
  if(count++ % 1000 == 0)
    terminal_putchar('&');

  if(cur >= next_wake_date)
    high_level_timer_interrupt_handler();
  
  hw_context_switch(cur_hw_ctx);
}
