.global dummy_interrupt_handler

dummy_interrupt_handler:
         pusha
        cld
        //jmp interrupt_handler2
        popa
        iret
        nop
interrupt_handler2:
        jmp interrupt_handler3
        nop
interrupt_handler3:
        hlt
