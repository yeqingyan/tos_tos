/*
 * Based on BCM2835 document section 7.5 
 */
#include <kernel.h>
#include <intr.h>
#include <timer.h>

/* Store enabled basic IRQs */
static unsigned int enabled_basic_irqs;
static unsigned int temp_enabled_basic_irqs;
static unsigned int current_irq;

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

/* Enable IRQ */
void enable_irq(int intr_no) {
    enabled_basic_irqs |= (1 << intr_no);    
    GetIRQController()->Enable_Basic_IRQs = (1 << intr_no);        
}
/* clear IRQ */ 
void clear_irq(int intr_no) {
    enabled_basic_irqs &= ~(1 << intr_no);
    GetIRQController()->Disable_Basic_IRQs |= (1 << intr_no);
}

/* 
    ISR dispatcher 
    Dispatch enabled interrupt routine
    
    TODO: This part of code better written in Assembly. To make sure no register is used by error.
      
*/                                
void isr_dispatcher(void) {  
        // Interrupt alreay disabled in irq_handler
        temp_enabled_basic_irqs = enabled_basic_irqs;
        
        current_irq = 0;
        while (temp_enabled_basic_irqs)
        {
            // Find enable irq
            if ((temp_enabled_basic_irqs & 1) == 1) {
                // Check IRQ is pending or not
                if ((((GetIRQController()->IRQ_Basic_Pending) & (1 << current_irq)) == 1) && (interrupts_table[current_irq] != NULL)) {
                    interrupts_table[current_irq]();    
                }
            }
            temp_enabled_basic_irqs = temp_enabled_basic_irqs >> 1;
            current_irq += 1;
        }
}

/* timer isr */                       
void isr_timer(void) {    
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        // Acknowledged we handled the irq
        GetARMTimer()->IRQ_Clear = 1;
        resign();
}

/* Init timer */
void init_timer(void){
        /* Setup tiemr interrupt service routine(ISR) */
        interrupts_table[INTR_ARM_TIMER] = isr_timer;
                
        // Enable receive timer interrupt IRQ 
        enable_irq(INTR_ARM_TIMER);

        // Get Timer register address, based on BCM2835 document section 14.2 
        // Setup Timer frequency around 1kHz 
        // Get timer load to 1024 
        GetARMTimer()->Load = 0x400;
        
        // Enable Timer, send IRQ, no-prescale, use 32bit counter
        GetARMTimer()->Control = INTR_TIMER_CTRL_23BIT | 
                INTR_TIMER_CTRL_ENABLE |
                INTR_TIMER_CTRL_INT_ENABLE |
                INTR_TIMER_CTRL_PRESCALE_1;
}

/* Initialize Interrupts */
void init_interrupts(void){
        DISABLE_INTR();
        int i;
        
        for(i=0; i < INTERRUPTS_NUMBER; i++) {
            interrupts_table[i] = NULL;         
        }
        
        init_timer();
        ENABLE_INTR();
}
