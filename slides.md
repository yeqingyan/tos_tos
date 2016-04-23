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
/* TOS irq handler at 0x00000018 */
void irq_handler() {
    R14_irq -= 4            /* Change R14_irq to the next instruction to be executed */
    srsdb #0x1f!            /* SRS instruction will store R14_irq and SPSR_irq into 
                                R13_sys(Stack Pointer in SYS Mode) */
    CPSR [4:0] = 0b1111     /* Change to system mode */
    push {r0-r3, lr}        /* store caller-save general purpose register, r0-r3 and lr */        
    isr_dispatcher();       /* Process IRQs */
    pop {r0-r3, lr}         /* Pop r0-r3, lr back */
    rfeia sp!               /* RFE instruction will load R14, SPSR on R13_sys stack into 
                                PC and CPSR register, resume interrupted program */
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
/* Initialize Interrupts table */
void init_interrupts(void){
        DISABLE_INTR();
        int i;
        
        for(i=0; i < INTERRUPTS_NUMBER; i++) {
            interrupts_table[i] = NULL;         
        }
        
        init_timer();
        ENABLE_INTR();
}
```

```c
/* Init timer */
void init_timer(void){
        /* Setup tiemr interrupt service routine(ISR) */
        interrupts_table[INTR_ARM_TIMER] = isr_timer;
                
        // Enable receive timer interrupt IRQ 
        enable_irq(INTR_ARM_TIMER);

        // Initalize Timer.... 
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