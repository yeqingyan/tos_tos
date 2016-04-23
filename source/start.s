.section .init
.global _start

/*
    Setup stack pointer
*/
_start:        
_vectors:
        ldr pc, reset_addr
        ldr pc, undef_addr
        ldr pc, swi_addr
        ldr pc, prefetch_addr
        ldr pc, abort_addr
        ldr pc, reserved_addr
        ldr pc, irq_addr
        ldr pc, fiq_addr

reset_addr:     .word reset_handler
undef_addr:     .word undef_handler
swi_addr:       .word swi_handler
prefetch_addr:  .word prefetch_handler
abort_addr:     .word abort_handler
reserved_addr:  .word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word fiq_handler

_endvectors:

.section .text
reset_handler:
        /*
            Copy ARM Exception Table to address 0x0, 
            The vector address will be 0x10000 in QEMU, 0x8000 in Raspberry Pi
        */
        ldr r0, =_vectors
        mov r1, #0x0000
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        
        /* Change to IRQ mode(0x12) and disable interrupt */
        cpsid i, #0x12
        
        /* Setup Stack pointer for IRQ mode */
        mov sp, #0xB00000
        
        /* Change back to SYS mode(0x1F) */
        cpsid i, #0x1F
        
        /* Setup Stack pointer for SYS mode */
        mov sp, #0xA00000
        
        b main
        
undef_handler:
        nop
        b undef_handler   

swi_handler:
        nop
        b swi_handler
        
prefetch_handler:
        nop
        b prefetch_handler
        
abort_handler:
        nop
        b abort_handler
        
fiq_handler:
        nop
        b fiq_handler
