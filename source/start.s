.section .init   
.global _start 
_start:    
        ldr r0, =_vectors
        mov r1, #0x0000
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        
        cpsid i, #0x1F          // Change back to SYS mode(0x1F)
        
        mov sp, #0xA00000       // Setup Stack pointer for SYS mode 
        
        b kernel_main

.section .text
_vectors:
        ldr pc, reset_addr              // 0x0
        ldr pc, undef_addr              // 0x4
        ldr pc, swi_addr                // 0x8
        ldr pc, prefetch_addr           // 0xC
        ldr pc, abort_addr              // 0x10
        ldr pc, reserved_addr           // 0x14        
        ldr pc, irq_addr                // 0x18
        ldr pc, fiq_addr                // 0x1C
        
reset_addr:     .word reset_handler
undef_addr:     .word undef_handler
swi_addr:       .word swi_handler
prefetch_addr:  .word prefetch_handler
abort_addr:     .word abort_handler
reserved_addr:  .word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word fiq_handler
_endvectors:

reset_handler:
        nop 
        b reset_handler
        
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
