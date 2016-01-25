/*
 * Based on BCM2835 document section 7.5 
 */
#include <kernel.h>

/*
 * Get IRQ Controller
 */
irq_controller_t* GetIRQController(void) {
        return (irq_controller_t*)INTR_BASE;
}

/*
 * Get ARM Timer
 */
arm_timer_t* GetARMTimer(void) {
        return (arm_timer_t*)ARM_TIMER_BASE;
}

/* Interrupt Handler */                       
void __attribute__((interrupt("IRQ"))) irq_handler(void) {
        DISABLE_INTR();
        // Acknowledged we handled the irq
        WriteString("Meet IRQ!");
        GetARMTimer()->IRQ_Clear = 1;
        //debug();
        ENABLE_INTR();
}

/* Initialize Interrupts */
void init_interrupts(void){
        DISABLE_INTR();
        // Enable receive timer interrupt IRQ 
        GetIRQController()->Enable_Basic_IRQs = INTR_ARM_TIMER;

        // Get Timer register address, based on BCM2835 document section 14.2 
        // Setup Timer frequency around 1kHz 
        GetARMTimer()->Load = 0x400;
        // Enable Timer, send IRQ, no-prescale, use 32bit counter
        GetARMTimer()->Control = INTR_TIMER_CTRL_23BIT | 
                INTR_TIMER_CTRL_ENABLE |
                INTR_TIMER_CTRL_INT_ENABLE |
                INTR_TIMER_CTRL_PRESCALE_1;
        ENABLE_INTR();
}
