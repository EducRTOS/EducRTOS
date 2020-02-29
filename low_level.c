#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "segment.h"
#include "low_level.h"
#include "high_level.h"

/* As Qemu can dump the state before each basic block, the following
   fake jump is useful to debug assembly code.  */
#define CHANGE_BB_FOR_ASM_DEBUG "jmp 1f\n1:\n"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#warning "Not using a cross-compiler"
#endif
 
/* This OS works only for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "Compile for 32 bit x86"
#endif

#include "user_tasks.h"

/**************** Boot ****************/

#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAGS ((1 << 0) | (1 << 1))
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_FLAGS + MULTIBOOT_MAGIC)

struct multiboot {
  uint32_t magic;
  uint32_t flags;
  uint32_t checksum;
} __attribute__((packed,aligned(4)));

const struct multiboot multiboot __attribute__((section(".multiboot"))) = {
   .magic = MULTIBOOT_MAGIC,
   .flags = MULTIBOOT_FLAGS,
   .checksum = MULTIBOOT_CHECKSUM,
};



#define KERNEL_STACK_SIZE 1024
/* System V ABI mandates that stacks are 16-byte aligned. */
static char kernel_stack[KERNEL_STACK_SIZE] __attribute__((used,aligned(16)));

/* Expand x and stringify it; usually what we want. */
#define XSTRING(x) STRING(x)
#define STRING(x) #x

/* Startup: just setup the stack and call the C function. */
asm("\
.global _start\n\
.type _start, @function\n\
_start:\n\
        /* Setup the stack */ \n\
	mov $(kernel_stack +" XSTRING(KERNEL_STACK_SIZE) "), %esp\n\
        mov %eax, %ecx\n\
        mov %ebx, %edx\n\
        call low_level_init\n\
        jmp error_infinite_loop\n\
/* setup size of _start symbol. */\n\
.size _start, . - _start\n\
");



/* An endless infinite loop, used to catch errors, even in contexts
   where there is no stack.
   MAYBE: Tell why we enter this loop in an ecx argument.  */
void __attribute__((noreturn))
error_infinite_loop(void);

asm("\
.global error_infinite_loop\n\
.type error_infinite_loop, @function\n\
error_infinite_loop:\n\
        /* Infinite loop. */\n\
	cli\n\
1:	hlt\n\
	jmp 1b\n\
");


/**************** TSS ****************/

/* The TSS; almost everything is unused when using software task switching. */
struct tss
{
   uint32_t unused_prev_tss;
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t unused_esp1;      
   uint32_t unused_ss1;
   uint32_t unused_esp2;
   uint32_t unused_ss2;
   uint32_t unused_cr3;
   uint32_t unused_eip;
   uint32_t unused_eflags;
   uint32_t unused_eax;
   uint32_t unused_ecx;
   uint32_t unused_edx;
   uint32_t unused_ebx;
   uint32_t unused_esp;
   uint32_t unused_ebp;
   uint32_t unused_esi;
   uint32_t unused_edi;
   uint32_t unused_es;         
   uint32_t unused_cs;        
   uint32_t unused_ss;        
   uint32_t unused_ds;        
   uint32_t unused_fs;       
   uint32_t unused_gs;         
   uint32_t unused_ldt;      
   uint16_t unused_trap;
   uint16_t unused_iomap_base;
} __attribute__((packed));

/* TSS for the processors. */

#define NUM_CPUS 1
#define current_cpu() 0
static struct tss tss_array[NUM_CPUS];

/**************** GDT and segment descriptors. ****************/

/* A note on x86 privilege.
   
   DPL = privilege in the data segment descriptor.
   RPL = privilege in the data segment selector. Allows to lower the privilege for some operations.
   CPL = privilege of th code segment selector, this is the current privilege level.
   There is a general protection fault if max(CPL,RPL) >= DPL. */

/* Size of descriptors. */
#define S16BIT 0
#define S32BIT 1

/* Granularity of descriptors: byte or 4k. */


#define create_descriptor(/* uint32_t */ base,                          \
/* uint32_t */ limit,                                                   \
/* bool */ present, /* 1 if used */           \
/* unsigned int */ privilege, /*  0 = full privileged, 3 = unprivileged  */ \
/* bool */ normal_segment ,   /* 0 for TSS and LDT, 1 for normal segment */ \
/* enum permissions */ permissions,                                   \
/* bool */ accessed,                                                  \
/* bool */ granularity, /* 0 = par byte, 1 = par block de 4k */ \
/* bool */ size)  /* 0 = 16 bit, 1 = 32 bit */ \
  (( (uint64_t) (limit) & 0x000F0000) |                                 \
   (((base) >> 16) & 0x000000FF) |                                      \
   ((base) & 0xFF000000) |                                              \
   ((accessed | (permissions << 1) | (normal_segment << 4) | (privilege << 5) | (present << 7)) << 8) | \
   (((size << 2) | (granularity << 3)) << 20)) << 32 |                  \
  ((base) << 16) |                                                      \
  ((limit) & 0x0000FFFF)

#define create_code_descriptor(base,limit,privilege,conforming,readable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(4 | (conforming << 1) | readable),accessed,granularity,size)

#define create_data_descriptor(base,limit,privilege,growdown,writable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(0 | (growdown << 1) | writable),accessed,granularity,size)

/* TSS descriptors must have a minimum size of 0x67. */
#define create_tss_descriptor(base,limit,privilege,busy,granularity)    \
  create_descriptor(base,((limit) <= 0x67?0x67:(limit)),1,privilege,0,(4 | 0 << 1 | busy),1,granularity,0)


static const segment_descriptor_t null_descriptor = create_descriptor(0,0,0,0,0,0,0,0,0);

/* Full access to all addresses for the kernel code and data. */
static const segment_descriptor_t kernel_code_descriptor = create_code_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);
static const segment_descriptor_t kernel_data_descriptor = create_data_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);

/* Because we use that in file-scope assembly, this must be a macro
   instead of an enum. */
#define _KERNEL_DATA_SEGMENT_INDEX   2
_Static_assert(_KERNEL_DATA_SEGMENT_INDEX == KERNEL_DATA_SEGMENT_INDEX);

/* Address of the gdt, and size in bytes. */
static inline void lgdt(segment_descriptor_t *gdt, int size)
{
  struct gdt_register {
    uint16_t limit; /* Maximum offset to access an entry in the GDT. */
    uint32_t base; } __attribute__((packed,aligned(8)));
  struct gdt_register gdtr;
  gdtr.limit = size;
  gdtr.base = (uint32_t) gdt;
  asm volatile ("lgdt %0": : "m" (gdtr) : "memory");
}

/**************** Interrupts ****************/

/* Do we need CLD in all interrupt handlers? static analysis will tell! */

/* This:
   - Saves the registers in the context structure;
   - Restores the cld flag (maybe not useful);
   - Restores the ds register (cs and ss are restored by the interrupt mechanism)
   - Loads the kernel stack, call high_level_syscall  */
asm("\
.global interrupt_handler\n\t\
interrupt_handler:\n\
	pusha\n\
	cld\n\
        movw $(" XSTRING(_KERNEL_DATA_SEGMENT_INDEX) " << 3), %ax \n \
        movw %ax, %ds\n\
        mov %esp, %eax\n\
	mov $(kernel_stack +" XSTRING(KERNEL_STACK_SIZE) "), %esp\n\
        /* Note: must use the fastcall discipline */\n\
	call high_level_syscall\n\
        jmp error_infinite_loop\n\
");

extern void interrupt_handler(void);

void __attribute__((noreturn))
hw_context_switch(struct hw_context* ctx);

/* https://wiki.osdev.org/IDT */
/* I shoud set up interrupt gates, so that interrupts are disabled on entry. */

/* Gate descriptors are entries in the idt. */
typedef uint64_t gate_descriptor_t;

/* We don't want to use task gate, these are for hardware context switching. */
/* The difference between interrupt and task gates is that interrupt
   gates clear the IF flag (interrupts are masked). */

/* No need to define a gate descriptor for every interrupt; the later
   ones can be caught by the general protection fault handler. */
#define NB_GATE_DESCRIPTORS 256


static gate_descriptor_t idt[NB_GATE_DESCRIPTORS];

/* Privilege is the needed privilege level for calling this interrupt.
   So, hardware interrupts (or exceptions?) should have 0, but for
   softwrare interrupt it should be 3. */
#define create_interrupt_gate_descriptor(func_ptr,code_segment_selector,privilege,size) \
((((func_ptr) & 0xFFFFULL) |                                               \
  ((code_segment_selector) << 16)) |                                    \
 ((((func_ptr) & 0xFFFF0000) |                                          \
  ((1 << 15) |                                                          \
   (privilege << 13) |                                                  \
   (size << 11) |                                                       \
   (0b00110ULL << 8ULL))) << 32ULL))

 /* Address of variables cannot be constants.. */
 /* gate_descriptor_t idt[] = { */
 /*  //  [0x00] = create_interrupt_gate_descriptor(doit,0,0,S32BIT), */
 /*    [0x00] = create_interrupt_gate_descriptor((uint32_t) doit,0,0,S32BIT) */
 /* }; */

extern void dummy_interrupt_handler(void);



void init_interrupts(void){
  for(int i = 0; i < NB_GATE_DESCRIPTORS; i++){
    idt[i] = create_interrupt_gate_descriptor((uintptr_t) &dummy_interrupt_handler,
                                              gdt_segment_selector(0,KERNEL_CODE_SEGMENT_INDEX),
                                              0, S32BIT);
  }
  idt[SOFTWARE_INTERRUPT_NUMBER] =
    create_interrupt_gate_descriptor((uintptr_t) &interrupt_handler,
                                     gdt_segment_selector(0,KERNEL_CODE_SEGMENT_INDEX),
                                     3, S32BIT);
  struct idt_register {
    uint16_t limit; /* Maximum offset to access an entry in the GDT. */
    uint32_t base; } __attribute__((packed,aligned(8)));
  struct idt_register idtr;
  idtr.limit = sizeof(idt);
  idtr.base = (uint32_t) idt;
  asm volatile ("lidt %0": : "m" (idtr) : "memory");    

}

 



/* Here is what interrupt does:

   - Compare the CPL with the privilege level of the gate
     descriptor. A general protection fault is issued if they do not
     match.

   - load eip from func_ptr in the interrupt gate
   - load cs from the interrupt gate

   - load esp and ss from tss?
 */


/****************  ****************/





//struct hw_context *cur_ctx;

void __attribute__((noreturn))
hw_context_switch(struct hw_context* ctx){
  /* terminal_print("Switching to %x\n", ctx); */

  /* terminal_print("Code segment is %llx\n", ctx->code_segment);   */
  /* terminal_print("Data segment is %llx\n", ctx->data_segment); */
  
  /* We will save the context in the context structure. */
  tss_array[current_cpu()].esp0 = (uint32_t) ctx + sizeof(struct pusha) + sizeof(struct interrupt_frame);

  load_ds_reg(ctx->iframe.ss);
  /* Load the context. */
  asm volatile
    ("mov %0,%%esp \n\
      popa\n\
      iret" : : "r"(ctx) : "memory");
  __builtin_unreachable();
}


void hw_context_init(struct hw_context* ctx, int idx, uint32_t pc,
                     uint32_t start_address, uint32_t end_address){
#ifdef DEBUG
  ctx->regs.eax = 0xaaaaaaaa;
  ctx->regs.ecx = 0xcccccccc;
  ctx->regs.edx = 0xdddddddd;
  ctx->regs.ebx = 0xbbbbbbbb;
  ctx->regs.ebp = 0x99999999;        
  ctx->regs.esi = 0x88888888;
  ctx->regs.edi = 0x77777777;
#endif  
  ctx->iframe.eip = pc;
  ctx->iframe.cs = (gdt_segment_selector(3, START_USER_INDEX + 2 * idx));
  /* Set only the reserved status flag, that should be set to 1. */
  ctx->iframe.flags = 0x2;
#ifdef DEBUG  
  ctx->iframe.esp = 0xacacacac;
#endif
  ctx->iframe.ss = (gdt_segment_selector(3, START_USER_INDEX + 2 * idx + 1));

  /* terminal_print("Init ctx is %x; ", ctx); */

  segment_descriptor_t * const gdt = user_tasks_image.low_level.system_gdt;
  gdt[START_USER_INDEX + 2 * idx] =
    create_code_descriptor(start_address, end_address - start_address,3,0,1,0,1,S32BIT);
  gdt[START_USER_INDEX + 2 * idx + 1] =
    create_data_descriptor(start_address, end_address - start_address,3,0,1,0,1,S32BIT);  
}

struct module_information {
  char *mod_start;
  char *mod_end;
  char *string;
  uint32_t reserved;
}  __attribute__((packed));

struct multiboot_information {
  uint32_t flags;
  /* If flags[0]. */
  uint32_t mem_lower;
  uint32_t mem_upper;
  /* If flags[1]. */
  uint32_t boot_device;
  /* If flags[2]. */
  uint32_t cmdline;
  /* If flags[3]. */
  uint32_t mods_count;
  struct module_information *mods_addr;
}  __attribute__((packed));


#include <stdarg.h>
#include "lib/fprint.h"
void __attribute__ ((format (printf, 1, 2)))
fatal(char * format,...){
  va_list ap;
  va_start(ap, format);  
  vfprint(terminal_putchar, format, ap);
  va_end(ap);
  error_infinite_loop();
}


void __attribute__((fastcall,used))
low_level_init(uint32_t magic_value, struct multiboot_information *mbi) 
{
  /* Initialize terminal interface */
  terminal_initialize();

  if(magic_value != 0x2BADB002)
    fatal("Not loaded by a multiboot loader");
  
  terminal_print("multiboot_information flags: %x\n", mbi->flags);

  #if 0  
  if((mbi->flags & 3) == 0)
    fatal("Multiboot information flags 3 is not present");

  if(mbi->mods_count != 1)
    fatal("This kernel must be loaded with exactly one module, here %d\n", mbi->mods_count);
  #endif
  
  terminal_writestring("Kernel start\n");
  
  /* Up to now we used the segments loaded by grub. Set up a new gdt
     and make sure that the segments use it. */
  {
    segment_descriptor_t *gdt = user_tasks_image.low_level.system_gdt;
    gdt[NULL_SEGMENT_INDEX] = null_descriptor;
    gdt[KERNEL_CODE_SEGMENT_INDEX] = kernel_code_descriptor;
    gdt[KERNEL_DATA_SEGMENT_INDEX] = kernel_data_descriptor;
    /* Initialization of TSS. */
    for(int i = 0; i < NUM_CPUS; i++){
      gdt[TSS_SEGMENTS_FIRST_INDEX + i] =
        create_tss_descriptor((uint32_t) &tss_array[i], sizeof(tss_array[i]), 3,0,0);
      tss_array[i].ss0 = gdt_segment_selector(0,KERNEL_DATA_SEGMENT_INDEX);
      tss_array[i].esp0 = (uint32_t) &kernel_stack[KERNEL_STACK_SIZE - sizeof(uint32_t)];
    }

    lgdt(gdt,sizeof(segment_descriptor_t) * (FIXED_SIZE_GDT + 2 * user_tasks_image.nb_tasks));
    terminal_writestring("After lgdt\n");

    load_code_segment(gdt_segment_selector(0,KERNEL_CODE_SEGMENT_INDEX));
    terminal_writestring("after load_cs\n");
    
    load_data_segments(gdt_segment_selector(0,KERNEL_DATA_SEGMENT_INDEX));
    terminal_writestring("after load data segments\n");
    
    load_tr(gdt_segment_selector(0,TSS_SEGMENTS_FIRST_INDEX));
  }

  /* Set-up the idt. */
  init_interrupts();

  terminal_writestring("Switching to userpsace\n");

  high_level_init();

  /* Now I should enter ring 3, and make a syscall. */
  /* https://wiki.osdev.org/SYSENTER // quite simple, should support segmentation, and also works on AMD */
  /* Bof, non il faut faire un iret je pense. */

    /* https://manybutfinite.com/post/cpu-rings-privilege-and-protection/ */
    /* => Explique les privilege levels: CPL, DPL, RPL */
    
  /* TODO: Fixed number of tasks. We could prove correctness without a configuration. */
  
  //  while(1);

  /* terminal_writestring("before reload data segments\n"); */
  
  //reload_data_segments(0,false,KERNEL_DATA_SEGMENT_INDEX);


 
  /* /\* Newline support is left as an exercise. *\/ */
  /* terminal_writestring("Hello, kernel World!\n Hello again!\n"); */
}

/* MAYBE: rename kernel.c into context-switch.c; this is the low-level part of the kernel. */
