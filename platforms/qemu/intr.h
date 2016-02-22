#ifndef __INTR__
#define __INTR__

/* Interrupt header file for QEMU */
/* Get INTR base address in QEMU by running "info qtree" command */
#define INTR_BASE       0x10140000
/* From PL 190 Manual 3.2 */
typedef volatile struct {
        LONG VICIRQSTATUS;            /* 0x000 */
        LONG VICFIQSTATUS;            /* 0x004 */
        LONG VICRAWINTR;              /* 0x008 */
        LONG VICINTSELECT;            /* 0x00C */
        LONG VICINTENABLE;            /* 0x010 */
        LONG VICINTENCLEAR;           /* 0x014 */
        LONG VICSOFTINT;              /* 0x018 */
        LONG VICSOFTINTCLEAR;         /* 0x01C */
        LONG VICPROTECTION;           /* 0x020 */
        LONG VIC_0x024_reserved[3];   /* 0x024 */
        LONG VICVECTADDR;             /* 0x030 */
        LONG VICDEFVECTADDR;          /* 0x034 */
        LONG VIC_0x038_reserved[50];  /* 0x038 */
        LONG VICVECTADDR_REGS[16];    /* 0x100 */
        LONG VIC_0x140_reserved[48];  /* 0x140 */
        LONG VICVECTCNTL_REGS[16];    /* 0x200 */
        LONG VIC_0x240_reserved[872]; /* 0x240 */
        LONG VICPERIPHID_REGS[4];     /* 0xFE0 */
        LONG VICPCELLID_REGS[4];      /* 0xFF0 */
} irq_controller_t;

/* Based on PL 190 Manual 2.1 Table 2-1 */
#define INTR_ARM_TIMER                4

/* PL 190 Support 32 standard interrupts */


void (*interrupts_table[INTERRUPTS_NUMBER])(void);

#endif