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
void charToBin(unsigned char, int*, const int);
void vs_printf(char *, const char*, va_list);

/* framebuffer.c */
typedef struct {
        int p_width;            /* #0 Physical Width */
        int p_height;           /* #4 Physical Height*/
        int v_width;            /* #8 Virutal Width(Framebuffer width) */
        int v_height;           /* #12 Virtual Height(Framebuffer height) */
        int gpu_pitch;          /* #16 GPU - Pitch */
        int bit_depth;          /* #20 Bit Depth (High Colour) */
        int x;                  /* #24 X (number of pixels to skip in the top left corner of the screen when copying the framebuffer to screen) */
        int y;                  /* #28 Y (same as X) */
        int gpu_pointer;        /* #32 GPU - Pointer */
        int gpu_size;           /* #36 GPU - Size */
} FrameBufferInfo;
extern FrameBufferInfo frameBufferInfo __attribute__ ((aligned (16), section(".data")));
FrameBufferInfo* InitialiseFrameBuffer(int, int, int );
void init_framebuffer(void);

/* mailbox.c */
int get_mailbox_base(void);
void mailbox_write(int, int);
int mailbox_read(int);

/* drawing.c */
#define CHARACTER_SIZE 16   /* Each character using 16 bytes */
#define CHARACTER_WIDTH 8   /* Character is 8x16 */
#define CHARACTER_HEIGHT 16
short foreground_color;
short background_color;
FrameBufferInfo* graphicsAddress;
WORD get_fore_colour();
void set_fore_colour(short);
WORD get_back_colour();
void set_back_colour(short);
void set_graphics_address(FrameBufferInfo*);
void set_pixel(int, int);
void clear_pixel(int, int);
WORD get_pixel_16bit(int, int);
void copy_pixel_16bit(int, int, int, int);
void DrawLine(int, int, int, int);
void draw_character(char, int, int);

/* window.c */
typedef struct {
  int  x, y;
  int  width, height;
  int  cursor_x, cursor_y;
  char cursor_char;
} WINDOW;

extern WINDOW* kernel_window;

void move_cursor(WINDOW* wnd, int x, int y);
void remove_cursor(WINDOW* wnd);
void show_cursor(WINDOW* wnd);
void clear_window(WINDOW* wnd);
void output_char(WINDOW* wnd, unsigned char ch);
void output_string(WINDOW* wnd, const char *str);
void wprintf(WINDOW* wnd, const char* fmt, ...);
void kprintf(const char* fmt, ...);
void copy_character(int, int, int, int);

/* pacman.c */
void init_pacman(WINDOW*, int);

/* usb.c */
// Library in libcsud.a
extern int UsbInitialise();
extern void UsbCheckForChange();
extern unsigned int KeyboardGetAddress();
extern int KeyboardGetKeyDown(unsigned int, int);
extern int KeyboardPoll(unsigned int);
extern BYTE KeyboardGetModifiers(unsigned int);
BOOL key_was_down(int);
void keyboard_update();
extern int KeyboardCount();
extern BYTE keys_normal[104];
extern BYTE  keys_shift[104];
int keyboard_get_char();
void init_usb();
extern unsigned int keyboard_address;
extern short old_keys[6];

/* ipc.c */
#define MAX_PORTS (MAX_PROCS * 2)
#define MAGIC_PORT 0x1234abcd

typedef struct _PORT_DEF {
    unsigned magic;             
    unsigned used;              /* Port slot used? */
    unsigned open;              /* Port open? */
    PROCESS owner;              /* Owner of this port */
    PROCESS blocked_list_head;  /* First local blocked process */
    PROCESS blocked_list_tail;  /* Last local blocked process */
    struct _PORT_DEF *next;     /* Next port */
} PORT_DEF;

PORT create_port();
PORT create_new_port (PROCESS proc);
void open_port (PORT port);
void close_port (PORT port);
void send (PORT dest_port, void* data);
void message (PORT dest_port, void* data);
void* receive (PROCESS* sender);
void reply (PROCESS sender);
void init_ipc();

/* intr.c */                        
void init_interrupts(void);  
void irq_handler(void);
#define ENABLE_INTR()   asm("mrs r0, cpsr"); \
                        asm("bic r0, r0, #0x80"); \
                        asm("msr cpsr_c, r0");

        
#define DISABLE_INTR()  asm("mrs r0, cpsr"); \
                        asm("orr r0, r0, #0x80"); \
                        asm("msr cpsr_c, r0");


#endif
