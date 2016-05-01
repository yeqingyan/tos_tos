#ifndef __TIMER__
#define __TIMER__

/* Timer header file for Raspberry Pi */
/* Timer registers offset from BCM2835 Section 14.2 Timer Registers */
#define ARM_TIMER_BASE  0x2000B400
typedef volatile struct {
    LONG Load;
    LONG Value;
    LONG Control;
    LONG IRQ_Clear;
    LONG RAW_IRQ;
    LONG Masked_IRQ;
    LONG Reload;
    LONG Pre_Divider;
    LONG Free_Running_Counter;
} arm_timer_t;

#define INTR_TIMER_CTRL_23BIT           (1 << 1) // Use 23-bit counter(it should be 32-bit)
#define INTR_TIMER_CTRL_ENABLE          (1 << 7) // Timer Enabled
#define INTR_TIMER_CTRL_INT_ENABLE      (1 << 5) // Enable Timer interrupt
#define INTR_TIMER_CTRL_PRESCALE_1      (0 << 2) // Pre-scal is clock/1 (No pre-scale)

/* Based on BCM2835 Document Section 7.5 ARM peripherals interrupts table */
#define TIMER_IRQ                  0

void init_interrupts(void);

#endif
