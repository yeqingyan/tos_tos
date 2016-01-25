.section .init
        .global _start
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
undef_addr:     .word reset_handler
swi_addr:       .word reset_handler
prefetch_addr:  .word reset_handler
abort_addr:     .word reset_handler
reserved_addr:  .word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word reset_handler

_endvectors:

.section .text
reset_handler:
        mov r0, #0x8000
        mov r1, #0x0000
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        mov sp, #0xA0000
        b main
