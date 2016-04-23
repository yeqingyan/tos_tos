#ifndef __INTR__
#define __INTR__

/* Based on BCM2835 Document Section 7.5 Interrupt Registers Overview */
#define INTR_BASE       0x2000B200
typedef volatile struct {
    LONG IRQ_Basic_Pending;
    LONG IRQ_Pending_1;
    LONG IRQ_Pending_2;
    LONG IRQ_FIQ_Control;
    LONG Enable_IRQs_1;
    LONG Enable_IRQs_2;
    LONG Enable_Basic_IRQs;
    LONG Disable_IRQs_1;
    LONG Disable_IRQs_2;
    LONG Disable_Basic_IRQs;
} irq_controller_t;

#define INTERRUPTS_NUMBER       64

void (*interrupts_table[INTERRUPTS_NUMBER])(void);

void isr_dispatcher(void);

#endif