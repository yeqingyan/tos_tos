#ifndef __TIMER__
#define __TIMER__

/* Timer header file for Raspberry Pi */
/* Timer registers offset from BCM2835 Section 14.2 Timer Registers */
#define ARM_TIMER_BASE  0x2000B400
typedef struct {
        volatile LONG Load;
        volatile LONG Value;
        volatile LONG Control;
        volatile LONG IRQ_Clear;
        volatile LONG RAW_IRQ;
        volatile LONG Masked_IRQ;
        volatile LONG Reload;
        volatile LONG Pre_Divider;
        volatile LONG Free_Running_Counter;
} arm_timer_t;
#define INTR_TIMER_CTRL_23BIT           (1 << 1) // Use 23-bit counter(it should be 32-bit)
#define INTR_TIMER_CTRL_ENABLE          (1 << 7) // Timer Enabled
#define INTR_TIMER_CTRL_INT_ENABLE      (1 << 5) // Enable Timer interrupt
#define INTR_TIMER_CTRL_PRESCALE_1      (0 << 2) // Pre-scal is clock/1 (No pre-scale)
/* Based on BCM2835 Document Section 7.5 ARM peripherals interrupts table */
#define INTR_ARM_TIMER                  (1 << 0)
#define CLK_FREQ                        1000000  // System Timer on BCM2835 is 1MHz.(Throught not mentioned in BCM2835 document)
void init_interrupts(void);
#endif
