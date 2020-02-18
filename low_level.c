#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "segment.h"
#include "low_level.h"

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

/**************** Boot ****************/

#define USER_STACK_SIZE 1024
#define KERNEL_STACK_SIZE 1024
/* System V ABI mandates that stacks are 16-byte aligned. */
static char user_stack[USER_STACK_SIZE] __attribute__((aligned(16)));
static char kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));

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

/* A segment descriptor is an entry in a GDT or LDT. */
typedef uint64_t segment_descriptor_t;

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
  (( (uint64_t) limit & 0x000F0000) |                                   \
   ((base >> 16) & 0x000000FF) |                                        \
   (base & 0xFF000000) |                                        \
   ((accessed | (permissions << 1) | (normal_segment << 4) | (privilege << 5) | (present << 7)) << 8) | \
   (((size << 2) | (granularity << 3)) << 20)) << 32 |                  \
  (base << 16) |                                                        \
  (limit & 0x0000FFFF)

#define create_code_descriptor(base,limit,privilege,conforming,readable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(4 | (conforming << 1) | readable),accessed,granularity,size)

#define create_data_descriptor(base,limit,privilege,growdown,writable,accessed,granularity,size) \
  create_descriptor(base,limit,1,privilege,1,(0 | (growdown << 1) | writable),accessed,granularity,size)

#define create_tss_descriptor(base,limit,privilege,busy,granularity)    \
  create_descriptor(base,limit,1,privilege,0,(4 | 0 << 1 | busy),1,granularity,0)


static const segment_descriptor_t null_descriptor = create_descriptor(0,0,0,0,0,0,0,0,0);

/* Full access to all addresses for the kernel code and data. */
static const segment_descriptor_t kernel_code_descriptor = create_code_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);
static const segment_descriptor_t kernel_data_descriptor = create_data_descriptor(0,0xFFFFFFFF,0,0,1,0,1,S32BIT);

/* Does not work because &tss0 is not a compile-time constant, it is split which cannot be done by the linker. */
//static const descriptor_t tss0_descriptor = create_tss_descriptor((&tss0),sizeof(tss0),0,1,0);

enum gdt_indices {
 NULL_SEGMENT_INDEX,
 KERNEL_CODE_SEGMENT_INDEX,
 KERNEL_DATA_SEGMENT_INDEX,
 USER_CODE_SEGMENT_INDEX, 
 USER_DATA_SEGMENT_INDEX, 
 TSS_SEGMENTS_FIRST_INDEX,
 NEXT = TSS_SEGMENTS_FIRST_INDEX + NUM_CPUS,
 SIZE_GDT
};

static segment_descriptor_t gdt[SIZE_GDT] = {
                              
 [NULL_SEGMENT_INDEX] = null_descriptor,
 [KERNEL_CODE_SEGMENT_INDEX] = kernel_code_descriptor,
 [KERNEL_DATA_SEGMENT_INDEX] = kernel_data_descriptor,
 [USER_CODE_SEGMENT_INDEX] = create_code_descriptor(0,0xFFFFFFFF,3,0,1,0,1,S32BIT),
 [USER_DATA_SEGMENT_INDEX] = create_data_descriptor(0,0xFFFFFFFF,3,0,1,0,1,S32BIT),
 /* We perform software task switching, but still need one TSS by processor. */
 // [TSS0_SEGMENT_INDEX] = 0       /* cannot be a compile-time constant. */
 
};

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


/* Because of how we manipulate the address, create_tss_descriptor
   cannot be a compile-time constant in C. */
static void register_tss_in_gdt(int idx, struct tss *tss, size_t size){
  segment_descriptor_t tss_descriptor = create_tss_descriptor((uint32_t) tss,
                                                       (size <= 0x67? 0x67 : size),
                                                       3,0,0);
  /* The descriptor should not be busy if we want to load the tss. */
  gdt[idx] = tss_descriptor;
}

/**************** Interrupts ****************/

/* mov $(kernel_stack +" XSTRING(KERNEL_STACK_SIZE) "), %esp\n \ */

/* Do we need CLD in all interrupt handlers? static analysis will tell! */
asm("\
.global interrupt_handler\n\t\
interrupt_handler:\n\
	pusha\n\
	cld\n\
        mov %esp, %ecx\n\
	mov $(kernel_stack +" XSTRING(KERNEL_STACK_SIZE) "), %esp\n\
	call c_interrupt_handler\n\
        jmp error_infinite_loop\n\
");

extern void interrupt_handler(void);


struct hw_context hw_ctx0;

static inline void __attribute__((noreturn))
hw_context_switch(struct hw_context* ctx);


void __attribute__((fastcall)) 
c_interrupt_handler(struct hw_context *cur_ctx, int num) {
  /* TODO: load DS too.  */
  terminal_writestring("Calling interrupt\n");
  terminal_write_uint32(num);
  hw_context_switch(cur_ctx);
}

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



struct hw_context hw_ctx0;

struct hw_context *cur_ctx;
static inline __attribute__((noreturn)) void hw_context_switch(struct hw_context* ctx){
  /* We will save the context in the context structure. */
  tss_array[current_cpu()].esp0 = (char *)ctx + sizeof(struct hw_context);
  /* TODO: also load ds when we load a hardware context. */  
  /* Load the context. */
  asm volatile ("mov %0,%%esp \n\
                 popa        \n\
                 iret" : : "r"(ctx) : "memory");
}


void hw_context_init(struct hw_context* ctx, uint32_t stack, uint32_t pc){
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
  ctx->iframe.cs = (gdt_segment_selector(3, USER_CODE_SEGMENT_INDEX));
  /* Set only the reserved status flag, that should be set to 1. */
  ctx->iframe.flags = 0x2;
  ctx->iframe.esp = stack;
  ctx->iframe.ss = (gdt_segment_selector(3, USER_DATA_SEGMENT_INDEX));
}

void low_level_init(void) 
{

  /* Initialize terminal interface */
  terminal_initialize();

  terminal_writestring("Kernel start\n");

  /* Up to now we used the segments loaded by grub. Set up a new gdt
     and make sure that the segments use it. */
  {
    lgdt(gdt,sizeof(gdt));
    terminal_writestring("After lgdt\n");

    load_code_segment(gdt_segment_selector(0,KERNEL_CODE_SEGMENT_INDEX));
    terminal_writestring("after load_cs\n");
    
    load_data_segments(gdt_segment_selector(0,KERNEL_DATA_SEGMENT_INDEX));
    terminal_writestring("after load data segments\n");
  }

  /* Set-up the tss and load the task register for CPU 0. */
  {
    for(int i = 0; i < NUM_CPUS; i++){
      register_tss_in_gdt(i + TSS_SEGMENTS_FIRST_INDEX, &tss_array[i], sizeof(tss_array[i]));

      tss_array[i].ss0 = gdt_segment_selector(0,KERNEL_DATA_SEGMENT_INDEX);
      tss_array[i].esp0 = &kernel_stack[KERNEL_STACK_SIZE - sizeof(uint32_t)];
      
    }
    load_tr(gdt_segment_selector(0,TSS_SEGMENTS_FIRST_INDEX));
  }

  /* Set-up the idt. */
  init_interrupts();

  terminal_writestring("Switching to userpsace\n");

  extern void test_userspace(void);
  hw_context_init(&hw_ctx0,&user_stack[KERNEL_STACK_SIZE - sizeof(uint32_t)],&test_userspace);
  hw_context_switch(&hw_ctx0);

  


  
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
