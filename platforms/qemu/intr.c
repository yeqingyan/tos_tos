/*
 * QEMU PL190 interrupt controller 
 */
#include <kernel.h>
#include <intr.h>
#include <timer.h>

BOOL interrupt_initialized = FALSE;
void irq_handler(void);

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
    GetIRQController()->VICINTENABLE |= (1 << intr_no);    
    
}
/* clear IRQ */ 
void clear_irq(int intr_no) {
    GetIRQController()->VICINTENCLEAR |= (1 << intr_no);
}

/* Interrupt Handler */
/* According to ARM1176JZFS Manual 2.12.2 Exception entry and exit summary :
    When CPU into IRQ mode, the Link Register(R14) will have value PC+4, where PC is the address 
    of the instruction that was NOT executed because the FIQ took priority. In order to return to 
    the right address(PC), we need to minus R14 by 4 
    
    Note by Yeqing Yan, */                        
void __attribute__((interrupt("IRQ"))) irq_handler(void) {
        DISABLE_INTR();
        
        unsigned int status = GetIRQController()->VICIRQSTATUS;
        int irq = 0;

        while (status)
        {
            if (((status & 1) == 1) && (interrupts_table[irq] != NULL)) {
                interrupts_table[irq]();
            }
            status = status >> 1;
            irq += 1;
        }
        
        //debug();
        ENABLE_INTR();
}

/* dummy isr */                       
void dummy_isr(void) {
        while(1) {
        }
}

/* timer isr */                       
void isr_timer(void) {    
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        GetARMTimer()->IntClr = 1;
}

/* Init timer */
void init_timer(void){
        /* Setup tiemr interrupt service routine(ISR) */
        interrupts_table[INTR_ARM_TIMER] = isr_timer;
                
        // Enable receive timer interrupt IRQ 
        enable_irq(INTR_ARM_TIMER);
        //GetIRQController()->VICINTENABLE = INTR_ARM_TIMER;

        // Get timer load to 1024 
        GetARMTimer()->Load = 0x400;
        
        // Enable Timer, send IRQ, no-prescale, use 32bit counter
        GetARMTimer()->Control = SP804_TIMER_EN | SP804_TIMER_MODE | 
            SP804_TIMER_INTENABLE | SP804_TIMER_SIZE | SP804_TIMER_PRE_1 | SP804_TIMER_ONESHOT;        
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

