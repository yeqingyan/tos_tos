#ifndef __KERNEL__
#define __KERNEL__

#include <assert.h>
#include <stdarg.h>
/* general */
#define TRUE    1
#define FALSE   0
#ifndef NULL
#define NULL    ((void *) 0)
#endif
typedef int BOOL;

/* main.c */
#define BIT_LENGTH 8
#define MSG_IDLE        0
#define MSG_SENT        1
#define MSG_RECEIVED    1
int main();

/* gpio.c */
#define GPIO_BASE       0x20200000
#define GPIO_LOW        0
#define GPIO_HIGH       1
#define GPIO_INPUT      0       /* GPIO Function 0: input */
#define GPIO_OUTPUT     1       /* GPIO Function 1: ouput */
#define SET_OFFSET      7       /* GPIO Pin Output Set Register         0x1c */
#define CLR_OFFSET      10      /* GPIO Pin Output Clear Register       0x28 */ 
#define GPLEV_OFFSET    13      /* GPIO Pin Level Register              0x34 */
#define GPPUD_OFFSET    37      /* GPIO Pull-Up/Down Register           0x94 */
#define GPPUDCLK_OFFSET 38      /* GPIO Pull-Up/Down Clock Register     0x98 */

#define PUD_OFF  0
#define PUD_DOWN 1
#define PUD_UP   2
int GetGpioAddress();
void SetGpioFunction(int, int);
void SetGpio(int, int);
int GetGpio(int);
void SetPullUpDn(int, int);
int ReadChar(int*, const int);
int WriteChar(int *, const int);
int WriteString(char *);
void GpioInputSetup(int, int, int);
void GpioOutputSetup(int, int, int);
void debug();
void error();

/* systemTimer.c */
int GetSystemTimerBase();
int GetTimeStamp();
void Wait(int);

/* mem.c */
typedef unsigned        MEM_ADDR;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned        LONG;

void poke_b(MEM_ADDR addr, BYTE value);
void poke_w(MEM_ADDR addr, WORD value);
void poke_l(MEM_ADDR addr, LONG value);
BYTE peek_b(MEM_ADDR addr);
WORD peek_w(MEM_ADDR addr);
LONG peek_l(MEM_ADDR addr);

/* process.c */
#define MAX_PROCS 20            /* Max number of processes */

#define STATE_READY             0
#define STATE_SEND_BLOCKED      1
#define STATE_REPLY_BLOCKED     2
#define STATE_RECEIVE_BLOCKED   3
#define STATE_MESSAGE_BLOCKED   4
#define STATE_INTR_BLOCKED      5

#define MAGIC_PCB       0x4321dcba
struct _PCB;
typedef struct _PCB* PROCESS;

struct _PORT_DEF;
typedef struct _PORT_DEF* PORT;

typedef struct _PCB {
        unsigned        magic;
        BOOL            used;
        unsigned short  priority;
        unsigned short  state;
        MEM_ADDR        sp;
        PROCESS         param_proc;
        void*           param_data;
        PORT            first_port;
        PROCESS         next_blocked;
        PROCESS         next;
        PROCESS         prev;
        char*           name;

} PCB;

extern PCB pcb[];
typedef unsigned PARAM;
void init_process();
PORT create_process();

/* dispatch.c */
#define MAX_READY_QUEUES 8
extern PROCESS active_proc;
extern PCB* ready_queue[];

void init_dispatcher();
void add_ready_queue(PROCESS proc);
void remove_ready_queue(PROCESS proc);
PROCESS dispatcher();
void resign();


/* stdlib.c */
void charToBin(char, int*, const int);
void vs_printf(char *, const char*, va_list);

/* intr.c */
#define ENABLE_INTR()   asm("mrs r0, cpsr"); \
                        asm("bic r0, r0, #0x80"); \
                        asm("msr cpsr_c, r0");

        
#define DISABLE_INTR()  asm("mrs r0, cpsr"); \
                        asm("orr r0, r0, #0x80"); \
                        asm("msr cpsr_c, r0");

/* Based on BCM2835 Document Section 7.5 Interrupt Registers Overview */
#define INTR_BASE       0x2000B200
typedef struct {
        volatile LONG IRQ_Basic_Pending;
        volatile LONG IRQ_Pending_1;
        volatile LONG IRQ_Pending_2;
        volatile LONG IRQ_FIQ_Control;
        volatile LONG Enable_IRQs_1;
        volatile LONG Enable_IRQs_2;
        volatile LONG Enable_Basic_IRQs;
        volatile LONG Disable_IRQs_1;
        volatile LONG Disable_IRQs_2;
        volatile LONG Disable_Basic_IRQs;
} irq_controller_t;
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
