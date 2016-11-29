.section .init   
.global _start 
_start:                 // CPU in Supervisor mode
  ldr r0, =_vectors     // Set up the vector table, the following code copy the   
  mov r1, #0x0000       // memory start at address _vector to 0x0000 
  ldmia r0!, {r2-r9}
  stmia r1!, {r2-r9}
  ldmia r0!, {r2-r9}
  stmia r1!, {r2-r9}
  
  cpsid i, #0x1F        // Switch to SYS mode(0b11111)
  
  mov sp, #0xA00000     // Set up Stack pointer for SYS mode 
  
  b kernel_main         // Call kernel_main() function

.section .text
_vectors:                 // vector table
  ldr pc, reset_addr      // 0x0
  ldr pc, undef_addr      // 0x4
  ldr pc, swi_addr        // 0x8
  ldr pc, prefetch_addr   // 0xC
  ldr pc, abort_addr      // 0x10
  ldr pc, reserved_addr   // 0x14        
  ldr pc, irq_addr        // 0x18
  ldr pc, fiq_addr        // 0x1C
  
reset_addr:     .word reset_handler
undef_addr:     .word undef_handler
swi_addr:       .word swi_handler
prefetch_addr:  .word prefetch_handler
abort_addr:     .word abort_handler
reserved_addr:  .word reset_handler
irq_addr:       .word master_isr
fiq_addr:       .word fiq_handler
_endvectors: