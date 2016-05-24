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

int kernel_main();

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

int get_gpio_address();

void set_gpio_function(int, int);

void set_gpio(int, int);

int GetGpio(int);

void set_pull_up_down(int, int);

int ReadChar(int *, const int);

int WriteChar(int *, const int);

int write_string(char *);

void GpioInputSetup(int, int, int);

void GpioOutputSetup(int, int, int);

void debug();

void error();

/* systemTimer.c */
int GetSystemTimerBase();

int GetTimeStamp();

void Wait(int);

/* mem.c */
typedef unsigned MEM_ADDR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned LONG;

void poke_b(MEM_ADDR addr, BYTE value);

void poke_w(MEM_ADDR addr, WORD value);

void poke_l(MEM_ADDR addr, LONG value);

BYTE peek_b(MEM_ADDR addr);

WORD peek_w(MEM_ADDR addr);

LONG peek_l(MEM_ADDR addr);

/* window.c */
typedef struct {
    int x, y;
    int width, height;
    int cursor_x, cursor_y;
    char cursor_char;
} WINDOW;

extern WINDOW *kernel_window;

void move_cursor(WINDOW *wnd, int x, int y);

void remove_cursor(WINDOW *wnd);

void show_cursor(WINDOW *wnd);

void clear_window(WINDOW *wnd);

void output_char(WINDOW *wnd, unsigned char ch);

void output_string(WINDOW *wnd, const char *str);

void wprintf(WINDOW *wnd, const char *fmt, ...);
void debugprintf(WINDOW *wnd, const char *fmt, ...);    // Do not disable interrupt when print
void debug_output_string(WINDOW *wnd, const char *str); // Do not disable interrupt when print

void kprintf(const char *fmt, ...);

void copy_character(int, int, int, int);

/* process.c */
#define boot_name "Boot process"
#define MAX_PROCS 20            /* Max number of processes */

typedef enum {
    STATE_READY = 0,
    STATE_SEND_BLOCKED = 1,
    STATE_REPLY_BLOCKED = 2,
    STATE_RECEIVE_BLOCKED = 3,
    STATE_MESSAGE_BLOCKED = 4,
    STATE_INTR_BLOCKED = 5,
} PROCESS_STATUS; 

//#define STATE_READY             0
//#define STATE_SEND_BLOCKED      1
//#define STATE_REPLY_BLOCKED     2
//#define STATE_RECEIVE_BLOCKED   3
//#define STATE_MESSAGE_BLOCKED   4
//#define STATE_INTR_BLOCKED      5

#define MAGIC_PCB       0x4321dcba
struct _PCB;
typedef struct _PCB *PROCESS;

struct _PORT_DEF;
typedef struct _PORT_DEF *PORT;

typedef struct _PCB {
    unsigned magic;
    BOOL used;
    unsigned short priority;
    PROCESS_STATUS state;
    MEM_ADDR sp;
    PROCESS param_proc;
    void *param_data;
    PORT first_port;
    PROCESS next_blocked;
    PROCESS next;
    PROCESS prev;
    char *name;

} PCB;

extern PCB pcb[];
typedef unsigned PARAM;

void init_process();

void print_process(WINDOW *, PROCESS);

void print_all_processes(WINDOW *);

PORT create_process();

/* dispatch.c */
#define MAX_READY_QUEUES 8
extern PROCESS active_proc;
extern PCB *ready_queue[];

void init_dispatcher();

void add_ready_queue(PROCESS proc);

void remove_ready_queue(PROCESS proc);

PROCESS dispatcher();
void dispatcher_impl();
void resign();


/* stdlib.c */
void charToBin(unsigned char, int *, const int);

void vs_printf(char *, const char *, va_list);

int k_strlen(const char*);

void* k_memcpy(void* , const void* , int);

/* framebuffer.c */
typedef struct {
    int p_width;
    /* #0 Physical Width */
    int p_height;
    /* #4 Physical Height*/
    int v_width;
    /* #8 Virutal Width(Framebuffer width) */
    int v_height;
    /* #12 Virtual Height(Framebuffer height) */
    int gpu_pitch;
    /* #16 GPU - Pitch */
    int bit_depth;
    /* #20 Bit Depth (High Colour) */
    int x;
    /* #24 X (number of pixels to skip in the top left corner of the screen when copying the framebuffer to screen) */
    int y;
    /* #28 Y (same as X) */
    int gpu_pointer;
    /* #32 GPU - Pointer */
    int gpu_size;           /* #36 GPU - Size */
} FrameBufferInfo;
extern FrameBufferInfo frameBufferInfo __attribute__ ((aligned (16), section(".data")));

FrameBufferInfo *init_framebuffer(int, int, int);

void init_video(void);

/* mailbox.c */
int get_mailbox_base(void);

void mailbox_write(int, int);

int mailbox_read(int);

/* drawing.c */
// 16-bit color
#define COLOR_BLACK     0x0000     
#define COLOR_GREEN     0x07E0
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800

#define CHARACTER_SIZE 16   /* Each character using 16 bytes */
#define CHARACTER_WIDTH 8   /* Character is 8x16 */
#define SCREEN_WIDTH     80
#define SCREEN_HEIGHT    25

#define CHARACTER_HEIGHT 16
short foreground_color;
short background_color;
FrameBufferInfo *graphicsAddress;

WORD get_fore_colour();

void set_fore_colour(short);

WORD get_back_colour();

void set_back_colour(short);

void set_graphics_address(FrameBufferInfo *);

void set_pixel(int, int);

void clear_pixel(int, int);

WORD get_pixel_16bit(int, int);

void copy_pixel_16bit(int, int, int, int);

void DrawLine(int, int, int, int);

void draw_character(char, int, int);

void draw_line(int, int, int, int, unsigned short);
/* pacman.c */
void init_pacman(WINDOW *, int);

int random();

/* keyb.c */
typedef struct _Keyb_Message {
    BYTE *key_buffer;
} Keyb_Message;
extern void CSUD_TEST();
int SendKeyboardPollingMessage(int);
extern void EnableInterrupt();
extern PORT keyb_port;
void usbkeyb_process(PROCESS, PARAM);
void init_keyb();
PORT init_usbkeyb();

extern void UsbCheckForChange();

extern unsigned int KeyboardGetAddress();

extern int KeyboardGetKeyDown(unsigned int, int);

extern int KeyboardPoll(unsigned int);
extern int KeyboardPollIntr(unsigned int);

extern BYTE KeyboardGetModifiers(unsigned int);

BOOL key_was_down(int);

void keyboard_update();

extern int KeyboardCount();

extern BYTE keys_normal[104];
extern BYTE keys_shift[104];

int keyboard_get_char();

void init_usb();
void usb_notifier_process(PROCESS, PARAM);

extern unsigned int keyboard_address;
extern unsigned int keyboard_address_intr;
extern short old_keys[6];

void disable_keyboard();
void enable_keyboard();
/* usb.c */
extern int intr_usb;      
//enum USB_TRANSFER_RESULT {
//    USB_TRANSFER_NULL,   // Not got any result yet.
//    USB_TRANSFER_COMPLETE,
//    USB_TRANSFER_NAK,
//    USB_TRANSFER_RESTART,
//};
//volatile enum USB_TRANSFER_RESULT usb_transfer_result;
typedef struct _Usb_Message {
    int *result_buffer;
} Usb_Message;
extern int UsbInitialise();
//extern int UsbInitialisePart1();
//extern int UsbInitialisePart2();
void isr_usb(void);
/* ipc.c */
#define MAX_PORTS (MAX_PROCS * 2)
#define MAGIC_PORT 0x1234abcd

typedef struct _PORT_DEF {
    unsigned magic;
    unsigned used;
    /* Port slot used? */
    unsigned open;
    /* Port open? */
    PROCESS owner;
    /* Owner of this port */
    PROCESS blocked_list_head;
    /* First local blocked process */
    PROCESS blocked_list_tail;
    /* Last local blocked process */
    struct _PORT_DEF *next;     /* Next port */
} PORT_DEF;

PORT create_port();

PORT create_new_port(PROCESS proc);

void open_port(PORT port);

void close_port(PORT port);

void send(PORT dest_port, void *data);

void message(PORT dest_port, void *data);

void *receive(PROCESS *sender);

void reply(PROCESS sender);

void init_ipc();

/* intr.c */
#define MAX_INTERRUPTS   64
/* Based on BCM2835 Document Section 7.5 ARM peripherals interrupts table */
//#define TIMER_IRQ           0
#define USB_IRQ             9
void init_interrupts(void);
void wait_for_interrupt (int);
void irq_handler(void);
void master_isr(void);
void isr_timer(void);
void enable_irq(int);
void disable_irq(int);
extern int USBIRQHandler();
PROCESS interrupt_table [MAX_INTERRUPTS];
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



void (*isr_table[MAX_INTERRUPTS])(void);


void isr_dispatcher(void);

// Restore previous saved flag
// Note: If the previous flag already disable IRQ, the IRQ won't enabled.
#define RESUME_CPSR(cpsr_flag) asm("msr cpsr_c, %[saved_flag]" :: [saved_flag] "r"(cpsr_flag));

// Save the CPSR
#define SAVE_CPSR_DIS_IRQ(cpsr_flag)  asm("mrs r0, cpsr"); \
                        asm("mov %[cpsr], r0" : [cpsr] "=r"(cpsr_flag) :); \
                        asm("orr r0, r0, #0x80"); \
                        asm("msr cpsr_c, r0");

/* shell.c */
int str_cmp(char *, char *);

void shell_buf_reset();

char *read_command();

void run_command(char *);

void shell_process(PROCESS, PARAM);

void init_shell();

/* video_test.c */
void init_lines_test();
void lines_proc(PROCESS, PARAM);
void lines_start();
void lines_stop();

/* led.c */
void led_proc(PROCESS , PARAM );
void init_led();
void wait_150_cycles();

/* serial.c */
extern int SendToUSB(char *buf, int bufSize);
extern int RecvFromUSB(char *buf, int bufSize);
extern PORT serial_port;

typedef struct _Serial_Message 
{
    char* output_buffer;
    char* input_buffer;
    int   len_input_buffer;
} Serial_Message;

void init_serial();


/* train.c */
void init_train(WINDOW* wnd);
void set_train_speed(char* speed);
void train_test();
void inc_sleep_time(WINDOW * );
void dec_sleep_time(WINDOW * );
void inc_check_time(WINDOW * );
void dec_check_time(WINDOW * );


/* 
 * timer.c 
 * 
 * BCM2835 document do not say the system clock frequency. 
 * From http://xinu.mscs.mu.edu/BCM2835_System_Timer. It seems the clock 
 * frequency is 1 MHz.
 * 
 * System Timer
 * 
 * BCM2835 have 4 system timer (C0-C3). (Ref BCM2835 Manual Section 12)
 * C0, C2 used by GPU. We can use C1, C3. We use C3 here as timer. 
 * 
 * BCM2835 also have another timer on ARM side, which is based on ARM AP804. 
 * (Ref BCM2835 Manual Section 14).
 * This timer is derived from the system timer. This clock can change 
 * dynamically e.g. if the system goes into reduced power or in low power mode. 
 * Thus the clock speed adapts to the overal system performance capabilities. 
 * For accurate timing it is recommended to use the system timers.
 * 
 * Each timer have their own IRQs. 
 * For system timer C0 - C3:    IRQ 0 - IRQ 3 (IRQ_Pending_1)
 * For Arm side timer:          IRQ 0         (IRQ_basic_pending)
 * 
 * BCM2835 document do not say the system clock frequency. 
 * From http://xinu.mscs.mu.edu/BCM2835_System_Timer. It seems the clock 
 * frequency is 1 MHz.
 * 
 * Ref.
 * BCM2835 Manual Section 12/14
 * http://xinu.mscs.mu.edu/BCM2835_System_Timer
 * http://xinu.mscs.mu.edu/BCM2835_Interrupt_Controller
 */ 
#define CLOCK_FREQ          1000000
#define CLKTICKS_PER_SEC    1000   // Our timer ticks per millisecond
#define TIMER_IRQ           3

// Ref BCM2835 Manual Section 12 (Page 172)
#define SYS_TIMER_BASE      0x20003000
typedef struct {
    unsigned volatile int M0        : 1;    // System Timer Match 0
    unsigned volatile int M1        : 1;    // System Timer Match 1
    unsigned volatile int M2        : 1;    // System Timer Match 2
    unsigned volatile int M3        : 1;    // System Timer Match 3
    unsigned volatile int RSERVED   : 28;   // Reserved
} SYS_TIMER_CS_REGISTER;

typedef struct {
    SYS_TIMER_CS_REGISTER   CS;    // System Timer Control/Status
    unsigned int            CLO;   // System Timer Counter Lower 32 bits 
    unsigned int            CHI;   // System Timer Counter Higher 32 bits
    unsigned int            C0;    // System Timer Compare 0.  Used by GPU.
    unsigned int            C1;    // System Timer Compare 1 
    unsigned int            C2;    // System Timer Compare 2.  Used by GPU.
    unsigned int            C3;    // System Timer Compare 3
} system_timer_t;


// Above is for system timer
void sleep(int num_of_ticks);
void init_timer();
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


void init_interrupts(void);
extern PORT timer_port;
typedef struct _Timer_Message 
{
    int num_of_ticks;
} Timer_Message;
int GetSystemTimerBase();
int GetTimeStamp();
arm_timer_t *GetARMTimer(void); 

/* null.c */
void init_null_process();
void null_proc(PROCESS, PARAM);

#endif
