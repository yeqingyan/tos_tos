Exception | Description
----|---
Reset|Occurs when the processor reset pin is asserted. This exception is only expected to occur for signalling power-up, or for resetting as if the processor has just powered up. A soft reset can be done by branching to the reset vector (0x0000).
Undefined Instruction | Occurs if neither the processor, or any attached coprocessor, recognizes the currently executing instruction.
Software Interrupt (SWI) | This is a user-defined synchronous interrupt instruction. The SWI instruction causes a SWI exception
Prefetch Abort | Occurs when the processor attempts to execute an instruction that was not fetched, because the address was illegal.
Data Abort | Occurs when a data transfer instruction attempts to load or store data at an illegal address.
IRQ | Occurs when the processor external interrupt request pin is asserted (LOW) and the I bit in the CPSR is clear.
FIQ | Occurs when the processor external fast interrupt request pin is asserted (LOW) and the F bit in the CPSR is clear.

Exceptions Address

Exception Type | CPU Mode | Address
---|---|---
Reset | Supervisor | 0x00000000
Undefined Instruction | Undefined | 0x00000004
Software Interrupt (SWI) | Supervisor | 0x00000008
Prefetch Abort | Abort | 0x0000000C
Data Abort | Abort | 0x00000010
IRQ(interrupt) | IRQ | 0x00000018
FIQ(fast interrupt) | FIQ | 0x0000001C

RESET exception 
```c
/* Pseudo when Reset exception happened */
CPSR [4:0] = 0b10011    /* Enter supervisor mode */
CPSR [6] = 1            /* Disable FIQs */
CPSR [7] = 1            /* Disable IRQs */
PC = 0x00000000         /* Jump to reset handler in 0x00000000 */

/* TOS Reset handler at 0x00000000 */
void reset_handler() {
    CPSR [4:0] = 0b1111 /* Change to system mode */
    kernel_main();             /* Go to main function */
}
```

IRQs exception 
```c
/* Pseudo when IRQs exception happened */
R14_irq = [address of the next instruction to be executed] + 4
SPSR_irq = CPSR         /* Save the CPSR before enter IRQ mode */
CPSR [4:0] = 0b10010    /* Enter supervisor mode */
CPSR [7] = 1            /* Disable IRQs */
PC = 0x00000018         /* Jump to irq handler in 0x00000018 */

/* TOS irq handler at 0x00000018 */
void irq_handler() {
    R14_irq -= 4            /* Change R14_irq to the next instruction to be executed */
    srsdb #0x1f!            /* SRS instruction will store R14_irq and SPSR_irq into 
                                R13_sys(Stack Pointer in SYS Mode) */
    CPSR [4:0] = 0b1111     /* Change to system mode */
    isr_dispatcher();       /* Process IRQs */
    rfeia sp!               /* RFE instruction will load R14, SPSR on R13_sys stack into 
                                PC and CPSR register, resume interrupted program */
}
```

```c
void master_isr(void) {
	// When CPU into IRQ mode, the Link Register(R14) will have value PC+4, where PC is the address 
	// of the instruction that was NOT executed because the IRQ took priority. In order to return to 
	// the right address(PC), we need to minus R14 by 4 
    asm("sub lr, lr, #4");

	// SRS (Store Return State) stores the LR and SPSR of the current mode(IRQ) to the stack in SYS mode 
	// 5 bits(0x1f) indicate the SYS mode. "db"(Decrement Before) suffix means cpu will decrement the stack 
	// pointer before store values into stack. "!" means after store R14 and SPSR of the IRQ mode into SYS 
	// mode stack, update the stack pointer in SYS mode.    
    asm("srsdb #0x1f!");   
    asm("cpsid i, #0x1f");			// Change CPSR to SYS mode, "i" means disable IRQ. "0x1f" means SYSTEM mode.    
    asm("push {r0-r12, r14}");	// Save registers
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);   // Save Stack pointer
    asm("bl irq_handler");			// handler IRQs
    asm("bl dispatcher_impl");	// Call dispatcher()
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));    // Get new process stack pointer

    asm("pop {r0-r12, r14}");		// Restore registers
    // RFE(Return From Exception) load LR, SPSR on SYS stack into PC and CPSR register, "ia" (increase after)   
	 // means increment stack pointer after read from stack. "!" means update value in sp after instruction. 
    asm("rfeia sp!");
}



```


```c
void isr_dispatcher(void) {
    //IRQ Base Pending address
    volatile unsigned int *irq_address = (unsigned int *) 0x2000B200;
    // Timer is IRQ 0
    unsigned int TIMER_IRQ = 1 << 0;
    
    // Handle Timer IRQ
    if (((*irq_address) & TIMER_IRQ == 1) && 
            (interrupt_table[TIMER_IRQ] != NULL)) {
        // isr_timer should clear Timter IRQ before return
        interrupt_table[TIMER_IRQ]();
    }
    
    // Handle other IRQs here
}
```

```c
/* Initialize Interrupts */
void init_interrupts(void) {
    int i;

    for (i = 0; i < INTERRUPTS_NUMBER; i++) {
        interrupts_table[i] = NULL;
    }
    init_timer();

    // Enable IRQ bit in CPSR
    asm("mrs r0, cpsr");
    asm("bic r0, r0, #0x80");	
    asm("msr cpsr_c, r0");
}

```

```c
#define INTR_ARM_TIMER      0
#define ARM_TIMER_BASE      0x2000B400
#define ENABLE_BASIC_IRQS   0x2000B218
#define INTR_TIMER_CTRL_23BIT           (1 << 1) // Use 23-bit counter(it should be 32-bit)
#define INTR_TIMER_CTRL_ENABLE          (1 << 7) // Timer Enabled
#define INTR_TIMER_CTRL_INT_ENABLE      (1 << 5) // Enable Timer interrupt
#define INTR_TIMER_CTRL_PRESCALE_1      (0 << 2) // Pre-scal is clock/1 (No pre-scale)

/* Init timer */
void init_timer(void) {
	 int * enable_basic_irqs = (int *) ENABLE_BASIC_IRQS;
	 int * arm_timer_load = (int *)(ARM_TIMER_BASE);
	 int * arm_timer_ctrl = (int *)(ARM_TIMER_BASE+0xC);
	 
    /* Setup tiemr interrupt service routine(ISR) */
    interrupts_table[INTR_ARM_TIMER] = isr_timer;

    // Enable receive timer interrupt IRQ
    *(enable_basic_irqs) = 1 << (INTR_ARM_TIMER);

    // Get Timer register address, based on BCM2835 document section 14.2
    // Setup Timer frequency around 1kHz
    // Get timer load to 1024
    *(arm_timer_load) = 0x400;

    // Enable Timer, send IRQ, no-prescale, use 32bit counter
    *(arm_timer_ctrl) =     INTR_TIMER_CTRL_23BIT |INTR_TIMER_CTRL_ENABLE |
                             INTR_TIMER_CTRL_INT_ENABLE | INTR_TIMER_CTRL_PRESCALE_1;
}
```

Instruction          | Usage
--- | ---
`add <value1>, <value2>` | ADD adds two values. The first value comes from a register. The second value can be either an immediate value or a value from a register
`b{l} <target_address>` | B (Branch) and BL (Branch and Link) cause a branch to a target address, and provide both conditional and unconditional changes to program flow. BL also stores a return address in the link register, R14
`cmp <value1>, <value2>` | CMP (Compare) compares two values. The first value comes from a register. The second value can be either an immediate value or a value from a register.
`mov <des>, <src>` | MOV (Move) writes a value to the destination register. The value can be either an immediate value or a value from a register
pop {r4, r5} | POP (Pop Multiple Registers) loads a subset (or possibly all) of the general-purpose registers and the PC from the stack.
push {r4, r5} | PUSH (Push Multiple Registers) stores a subset (or possibly all) of the general-purpose registers and the LR to the stack.


```assembly 
Addr	Machine Code        Assembly
0x0		e3 a0 10 00         mov r1, #0
0x4		eb 00 00 00			bl L2
0x8 	eb ff ff fe     L1: bl L1
0xC		e5 2d e0 04     L2: push {lr}
0x10	e2 81 10 01         add r1, r1, #1
0x14	eb 00 00 00         bl L3
0x18	e4 9d f0 04         pop {pc}
0x1c	e1 a0 f0 0e     L3: mov pc, lr
```

```c
int WriteChar(int *buf, const int len) {
    int i;
    /* Write char to terminal bit by bit */
    for (i = 0; i < len; i++) {
        /* Make sure terminal side is ready */
        while (get_gpio(TERMINAL_READ_STATUS) != MSG_IDLE)
            Wait(10);
        set_gpio(TOS_OUTPUT_BITS, *(buf + i));
        set_gpio(TOS_WRITE_STATUS, MSG_SENT);
        /* Wait terminal read msg */
        while (get_gpio(TERMINAL_READ_STATUS) != MSG_RECEIVED)
            Wait(10);
        set_gpio(TOS_WRITE_STATUS, MSG_IDLE);
    }
    return 0;
}
```

```python
reply_bin = []
for _ in xrange(8):
    # Wait Tos to write message 
    while GPIO.input(self.tos_write) == TosMsg.IDLE:
        continue
    # Construct message
    reply_bin.append(GPIO.input(self.input_bits))
    
    # Tell TOS received message
    GPIO.output(self.terminal_read, TosMsg.MSG_RECEIVED)
    while GPIO.input(self.tos_write) == TosMsg.MSG_SENT:
        continue
    # Finish read 
    GPIO.output(self.terminal_read, TosMsg.IDLE)
```

```c
/* Use resign_impl() helper function, so all code in resign are assembly 
 * then GCC compile won't add push/pop code around the function.    
 */
void resign_impl() {
    active_proc = dispatcher();
}

void resign() {
    asm("push {r0-r12, r14}");

    /* Set active process */
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);
    asm("bl resign_impl");
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));

    asm("pop {r0-r12, pc}");
}
```

Turn on LED

```c
int main() {
    int function = 1;       // Each GPIO pin have multiple functions. Function 1 for enable output 
    int pin = 4;            // We use GPIO Pin 4 for output 
        
    /* Step 1. Enable output to Pin 4 
     * Save value to address 0x20200000, set bit 12-14 to 001 to enable output to Pin 4 */     
    int *gpio_addr = (int *)0x20200000;    // This address for GPIO Pin 0-9 //
    
    function = function << (pin % 10 * 3);  // Set Bit 12-14 to 001 //
    int mask = 7 << (pin % 10 * 3);         // Create mask for Pin 4 //
    mask = ~mask;

    int gpio_val = *gpio_addr;          // Got old bit set //
    gpio_val = gpio_val & mask;         // Set bit 12-14 to 0
    gpio_val = gpio_val ^ function;     // Store 001 to bit 12-14   
    *gpio_addr = gpio_val;              // Enable output to Pin 14 //
        
    /* Step 2. Turn on Pin 4 
     * Save value to address 0x2020001c, set bit 4 to 1 to turn on pin 4 */        
    gpio_addr = (int *)0x2020001c;   // Address turn on pin 0 - 31
    int bit_value = 1 << pin;        // Set Bit 4 to 1 for Pin 4  
    *gpio_addr = bit_value;          // Turn Pin 4 on 
    while(1){};
}
```
GPIO INPUT/OUTPUT

```c
int *gpio_addr = (int *)0x20200000;       // GPIO Base address
const int PIN_IN=8, PIN_OUT=7, INPUT_FUNC = 0, OUTPUT_FUNC = 1;
// SET_OFFSET = 0x1C CLR_OFFSET = 0x28 ... etc 
const int SET_OFFSET=7, CLR_OFFSET=10, GPLEV_OFFSET=13, GPPUD_OFFSET=37, GPPUDCLK_OFFSET=38;

void set_gpio_func(int pin, int function) {     // Setup function for GPIO Pins. 
    function = function << (pin % 10 * 3);      // Function 0 for input, 1 for output 
    int mask = ~(7 << (pin % 10 * 3));          // Each pin using 3 bits.
    (*gpio_addr) = ((*gpio_addr) & mask ) ^ function;
}

void wait_150_cycles() {
    int i;
    for (i=0; i<150; i++) { asm volatile("nop"); }}
        
int main() {  
    // Step 1. Set default input pin 8 to high. 
    // GPIO Pull-up/down register: Set bit 1-0 to 10 to enable default input to HIGH  
    *(gpio_addr + GPPUD_OFFSET) = (*(gpio_addr + GPPUD_OFFSET) & ~3) | 2;
    wait_150_cycles();      // According BCM2835 manual, wait 150 cycles before next step.
    // GPIO Pull-up/down clock register: Write the previous contorl value to Pin 8 
    *(gpio_addr + GPPUDCLK_OFFSET) = 1 << PIN_IN;  
    wait_150_cycles();
    // Cleanup two registers  
    *(gpio_addr + GPPUD_OFFSET) &= ~3;
    *(gpio_addr + GPPUDCLK_OFFSET) = 0;
     
    // Sep 2. Enable input for pin 8, output for pin 7
    set_gpio_func(PIN_IN, INPUT_FUNC);    // Pin 8 for input, set bit 26-24 to 000 
    set_gpio_func(PIN_OUT, OUTPUT_FUNC);  // Pin 7 for output, set bit 23-21 to 001
    
    // Step 3. Using switch to turn on/off led. 
    while (1) {
        if ((*(gpio_addr + GPLEV_OFFSET) & (1 << PIN_IN)) == 0) { // Check Pin 8's level
            *(gpio_addr + SET_OFFSET) = (1 << PIN_OUT); // If Pin 8 is high, turn Pin 7 on to light on LED
        } else {
            *(gpio_addr + CLR_OFFSET) = (1 << PIN_OUT); // If Pin 8 is low, turn Pin 7 off to light off LED
        }
    }
}

```

Reset handler

```c
CPSR [4:0] = 0b10011 	// Enter supervisor mode 
CPSR [6] = 1 				// Disable FIQs
CPSR [7] = 1				// Disable IRQs

// Jump to 0x0, which store the assembly instrction to call reset_handler.
b 0x0						

```

```c
// Using ldr instead of b(Branch) since branch can only jump to +/-32MB of current address.
Address
0x0:             ldr pc, reset_addr	 

Reset_addr:      reset_handler		// Store the function address
Reset_handler:   b kernel_main();		// Jump to main function

```

Reset handler

```
_start:        
_vectors:
        ldr pc, reset_addr              // 0x0
        ldr pc, undef_addr              // 0x4
        ldr pc, swi_addr                // 0x8
        ldr pc, prefetch_addr           // 0xC
        ldr pc, abort_addr              // 0x10
        ldr pc, reserved_addr           // 0x14
        ldr pc, irq_addr                // 0x18
        ldr pc, fiq_addr                // 0x1C
        
reset_addr:     .word reset_handler
undef_addr:     .word undef_handler
swi_addr:       .word swi_handler
prefetch_addr:  .word prefetch_handler
abort_addr:     .word abort_handler
reserved_addr:  .word reset_handler
irq_addr:       .word irq_handler
fiq_addr:       .word fiq_handler
_endvectors:

.section .text
reset_handler:          
        ldr r0, =_vectors
        mov r1, #0x0000
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        ldmia r0!, {r2-r9}
        stmia r1!, {r2-r9}
        
        cpsid i, #0x1F          // Change back to SYS mode(0x1F)        
        mov sp, #0xA00000       // Setup Stack pointer for SYS mode 
        
        b kernel_main
```

improved c 

```c
int *gpio_addr = (int *)0x20200000;       // GPIO Base address
const int PIN_IN=8, PIN_OUT=7, INPUT_FUNC = 0, OUTPUT_FUNC = 1;
// SET_OFFSET = 0x1C CLR_OFFSET = 0x28 ... etc 
const int SET_OFFSET=7, CLR_OFFSET=10, GPLEV_OFFSET=13, GPPUD_OFFSET=37, GPPUDCLK_OFFSET=38;

void wait_150_cycles() {
    int i;
    for (i=0; i<150; i++) { asm volatile("nop"); }}

typedef struct {
    unsigned volatile int pud     : 2;
    unsigned volatile int unused  : 30;    
} GPPUD_REG;

typedef struct {
    unsigned volatile int pin0_pin7  : 8;
    unsigned volatile int pin8       : 1;
    unsigned volatile int pin9_pin31 : 23; 
} GPPUDCLK_REG;

typedef struct {
    unsigned volatile int pin0_pin6 : 21;
    unsigned volatile int pin7      : 3;
    unsigned volatile int pin8      : 3;
    unsigned volatile int pin9      : 5;
} GPFSEL_REG;

typedef struct {
    unsigned volatile int pin0_pin7  : 8;
    unsigned volatile int pin8       : 1;
    unsigned volatile int pin9_pin31 : 23;
} GPLEV_REG;

typedef struct {
    unsigned volatile int pin0_pin6  : 7;
    unsigned volatile int pin7       : 1;
    unsigned volatile int pin8_pin31 : 24;
} GPSET_GPCLR_REG;
    
int kernel_main() {  
    // Step 1. Set default input pin 8 to high. 
    // GPIO Pull-up/down register: Set bit 1-0 to 10 to enable default input to HIGH
    ((GPPUD_REG *)(gpio_addr + GPPUD_OFFSET))->pud = 2;    
    wait_150_cycles();      // According BCM2835 manual, wait 150 cycles before next step.
    // GPIO Pull-up/down clock register: Write the previous contorl value to Pin 8
    ((GPPUDCLK_REG *)(gpio_addr + GPPUDCLK_OFFSET))->pin8 = 1; 
    wait_150_cycles();
    // Cleanup two registers  
    ((GPPUD_REG *)(gpio_addr + GPPUD_OFFSET))->pud = 0;
    ((GPPUDCLK_REG *)(gpio_addr + GPPUDCLK_OFFSET))->pin8 = 0;
     
    // Step 2. Enable input for pin 8, output for pin 7
    ((GPFSEL_REG *)gpio_addr)->pin7 = 1;
    ((GPFSEL_REG *)gpio_addr)->pin8 = 0;
    
    // Step 3. Using switch to turn on/off led. 
    while (1) {
        if (((GPLEV_REG *)(gpio_addr + GPLEV_OFFSET))->pin8 == 0) { // Check Pin 8's level
            ((GPSET_GPCLR_REG *)(gpio_addr + SET_OFFSET))->pin7 = 1 ; // If Pin 8 is high, turn Pin 7 on to light on LED
        } else {
            ((GPSET_GPCLR_REG *)(gpio_addr + CLR_OFFSET))->pin7 = 1 ;// If Pin 8 is low, turn Pin 7 off to light off LED
        }
    }
}
```

Draw pixel

```c
typedef struct {
    int p_width;     // Physical Width
    int p_height;    // Physical Height
    int v_width;     // Virutal Width(Framebuffer width)
    int v_height;	 // Virtual Height(Framebuffer height)
    int gpu_pitch;   // GPU - Pitch
    int bit_depth;   // Bit Depth (High Colour)
    int x; // number of pixels to skip in the top left corner of the screen when copying the framebuffer to screen
    int y;
    int gpu_pointer; // Point to the frame buffer
    int gpu_size;    // GPU - Size
} FrameBufferInfo;
FrameBufferInfo *graphicsAddress;		// FranebufferInfo was initilized when tos boot.

short foreground_color = 0xFFFF; // Foreground white color
// Draw a pixel at row y, column x. This function only work for high color(16-bit)
void draw_pixel(int x, int y) {
    int width;
    short *gpu_pointer;		// Each pixel use 2 bytes.

    width = (graphicsAddress->p_width);       /* Get Width */

    /* Compute the address of the pixel to write */
    gpu_pointer = (short *) graphicsAddress->gpu_pointer;
    *(gpu_pointer + (x + y * width)) = foreground_color;  /* Calculate pixel position in memory */
	return;    
}
```

```c
void master_isr(void) {
    asm("sub lr, lr, #4");
    asm("srsdb #0x1f!");        // store lr and SPSR to stack
    asm("cpsid i, #0x1f");      // Switch to SYS mode
    asm("push {r0-r12, r14}");  // Save registers
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);
    asm("bl irq_handler");      // handler IRQs
    asm("bl dispatcher_impl");  // Find next available process
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));
    asm("pop {r0-r12, r14}");   // Restore registers
    asm("rfeia sp!");           // Return from IRQ
}
```