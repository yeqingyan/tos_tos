#ifndef __TIMER__
#define __TIMER__

/* Timer SP804 header file for QEMU Versatilepb */
/* From SP804 Manual */
#define ARM_TIMER_BASE  0x101E2000
typedef volatile struct {
        LONG Load;             /* Load Register */
        LONG Value;            /* Current Value Register */
        LONG Control;          /* Control Register */
        LONG IntClr;           /* Interrupt Clear Register */
        LONG RIS;              /* Raw Interrupt Status Register */
        LONG MIS;              /* Masked Interrupt Status Register */
        LONG BGload;           /* Background Load Register */
} arm_timer_t;

/* Timer control registers, based on SP804 Manual 3.2.3 */
#define SP804_TIMER_EN          (1 << 7) // Enable Timer Module
#define SP804_TIMER_MODE        (1 << 6) // Timer module in periodic mode
#define SP804_TIMER_INTENABLE   (1 << 5) // Timer module Interrupt enabled
#define SP804_TIMER_PRE_1       (0 << 2) // 0 stages of persclale, clock is divid by 1(default)
#define SP804_TIMER_SIZE        (1 << 1) // 32-bit counter
#define SP804_TIMER_ONESHOT     (0 << 0) // Disalbe one-shot use wrappping mode

void init_interrupts(void);
#endif
