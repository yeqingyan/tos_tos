/* Init timer */
void init_timer(void) {
    /* Setup timer interrupt service routine(ISR) */
    interrupts_table[INTR_ARM_TIMER] = isr_timer;

    // Enable receive timer interrupt IRQ
    enable_irq(INTR_ARM_TIMER);

    // Initalize Timer....
}


arm-none-eabi-gcc -
nostartfiles build
/process.
o build
/main.o -
o build
/output.elf -Wl,-L,.,-l,csud,--section-start,.
init = 0x10000,
--section-start,.
stack = 0xA00000,
-
Map = kernel.map
