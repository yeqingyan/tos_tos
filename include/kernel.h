#ifndef __KERNEL__
#define __KERNEL__

/* general */
#define TRUE    1
#define FALSE   0
#ifndef NULL
#define NULL    ((void *) 0)
#endif

/* main.c */
#define BIT_LENGTH 8
#define MSG_IDLE        0
#define MSG_SENT        1
#define MSG_RECEIVED    1

/* gpio.c */
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

/* process.c */

/* Max number of processes */
#define MAX_PROCS 20

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
        unsigned        used;
        unsigned short  priority;
        unsigned short  state;
        MEM_ADDR        esp;
        PROCESS         param_proc;
        void*           param_data;
        PORT            first_port;
        PROCESS         next_blocked;
        PROCESS         next;
        PROCESS         prev;
        char*           name;

} PCB;

extern PCB pcb[];
void init_process();

/* dispatch.c */
#define MAX_READY_QUEUES 8
extern PROCESS active_proc;
extern PCB* ready_queue[];

void init_dispatcher();
void add_ready_queue(PROCESS proc);

/* stdlib.c */
void charToBin(char, int*, const int);
#endif
