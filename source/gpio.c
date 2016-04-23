#include <kernel.h>

/* TOS Input/Output Channel */
int TERMINAL_INPUT_BITS = 0;
int TERMINAL_WRITE_STATUS = 0;
int TERMINAL_READ_STATUS = 0;
int TOS_OUTPUT_BITS = 0;
int TOS_WRITE_STATUS = 0;
int TOS_READ_STATUS = 0;

/*
 * short_wait()
 * ------------
 *  Wait 150 cycles
 */
void short_wait(void) {
    int i;

    for (i = 0; i < 150; i++) {
        asm volatile("nop");
    }
}

/*
 * get_gpio_address()
 * ------------------
 *  Return GPIO base address.
 * 
 * Return:
 *  GPIO base address
 */
int get_gpio_address() {
    return GPIO_BASE;
}

/*
 * set_gpio_function()
 * -----------------
 *  Choose Gpio Function
 *
 *  Parameters:
 *  pin: GPIO pin number (Pin must between 0 and 53)
 *  function: the function want to use. (Between 0 and 7)
 */
void set_gpio_function(int pin, int function) {
    if ((pin > 53) || (pin < 0))
        error();

    if ((function > 8) || (function < 0))
        error();

    int *gpio_addr = (int *) get_gpio_address();

    gpio_addr += pin / 10;  /* Find gpio address */

    function = function << (pin % 10 * 3);
    int mask = 7 << (pin % 10 * 3);     /* Create mask */
    mask = ~mask;

    int gpio_val = *gpio_addr;          /* Got old bit set */
    gpio_val = gpio_val & mask;
    gpio_val = gpio_val ^ function;     /* Got final bit set */

    *gpio_addr = gpio_val;
    return;
}

/*
 * set_gpio()
 * ----------
 *  Write High/Low value to gpio pins
 *
 *  Parameters:
 *  pin: GPIO pin number
 *  pinVal: High/Low value
 */
void set_gpio(int pin, int pinVal) {
    /* Check pin between 0 to 53 */
    if ((pin > 53) || (pin < 0))
        error();

    if ((pinVal != 0) && (pinVal != 1))
        error();

    int *gpio_addr = (int *) get_gpio_address();

    gpio_addr += pin / 32;

    int bitValue;
    bitValue = 1 << (pin % 32);

    if (pinVal == 0) {
        *(gpio_addr + CLR_OFFSET) = bitValue; /* Turn off pin */
    } else {
        *(gpio_addr + SET_OFFSET) = bitValue; /* Turn on pin */
    }

    return;
}

/*
 * set_pull_up_down()
 * ------------------
 *  Setup pull-up/down resistor on GPIO pins
 *
 *  Parameters:
 *  pin: GPIO pin number
 *  pud: PUD_DOWN or PUD_UP
 *  
 *  Note: GPIO pin 2,3 have physical pull-up resistor attach to them, set pull-down 
 *  resistor on those pins won't work.
 */
void set_pull_up_down(int pin, int pud) {
    int *gpio_addr = (int *) get_gpio_address();

    /* 1. Write to GPPUD(GPIO Pull-Up/Down Register) */
    if (pud == PUD_DOWN)
        *(gpio_addr + GPPUD_OFFSET) = (*(gpio_addr + GPPUD_OFFSET) & ~3) | PUD_DOWN;
    else if (pud == PUD_UP)
        *(gpio_addr + GPPUD_OFFSET) = (*(gpio_addr + GPPUD_OFFSET) & ~3) | PUD_UP;
    else /* pud == PUD_OFF */
        *(gpio_addr + GPPUD_OFFSET) &= ~3;

    /* 2. Wait 150 cycles */
    short_wait();

    /* 3. Write to GPPUDCLK(GPIO Pull-Up/Down Clock Register) */
    int clk_offset = GPPUDCLK_OFFSET + (pin / 32);
    *(gpio_addr + clk_offset) = 1 << (pin % 32);

    /* 4. Wait 150 cycles*/
    short_wait();

    /* 5. Write to GPPUD to remove the control signal */
    *(gpio_addr + GPPUD_OFFSET) &= ~3;

    /* 6. Write to GPPUDCLK to remove the clock */
    *(gpio_addr + clk_offset) = 0;
}

/*
 * GetGpio()
 * ---------
 *  Get value of GPIO pins
 *
 *  Paramters:
 *  pin: GPIO pin number
 *
 *  Return:
 *  GPIO pin value
 */
int GetGpio(int pin) {
    int *gpio_addr = (int *) get_gpio_address();
    int offset = GPLEV_OFFSET + (pin / 32);
    int mask = (1 << pin % 32);
    int value;
    value = *(gpio_addr + offset) & mask;
    if (value)
        return GPIO_HIGH;
    else
        return GPIO_LOW;
}

/* 
 * ReadChar()
 * ----------
 *  Read char from Terminal 
 *
 *  Parameters:
 *  buf: Int array to store char in binary format 
 *  len: Array length
 *
 *  Return:
 *  -1: Error
 */
int ReadChar(int *buf, const int len) {
    int i;
    for (i = 0; i < len; i++) {
        /* Check Error */
        if (GetGpio(TOS_READ_STATUS) != MSG_IDLE) {
            error();
            return -1;
        }
        /* Wait until Terminal Status change to MSG_SENT */
        while (GetGpio(TERMINAL_WRITE_STATUS) == MSG_IDLE)
            Wait(10);

        /* Read bits */
        *(buf + i) = GetGpio(TERMINAL_INPUT_BITS);
        set_gpio(TOS_READ_STATUS, MSG_RECEIVED);
        /* Make sure terminal side is finished */
        while (GetGpio(TERMINAL_WRITE_STATUS) != MSG_IDLE)
            Wait(10);
        set_gpio(TOS_READ_STATUS, MSG_IDLE);
    }
    return 0;
}

/* 
 * WriteChar()
 * -----------
 *  Write char to Terminal 
 *
 *  Parameters:
 *  buf: Int array write to teminal
 *  len: Array length
 *
 *  Return:
 *  -1: Error
 */
int WriteChar(int *buf, const int len) {
    int i;
    for (i = 0; i < len; i++) {
        /* Check error */
        if (GetGpio(TOS_WRITE_STATUS) != MSG_IDLE) {
            error();
            return -1;
        }
        /* Make sure terminal side is ready */
        while (GetGpio(TERMINAL_READ_STATUS) != MSG_IDLE)
            Wait(10);
        set_gpio(TOS_OUTPUT_BITS, *(buf + i));
        set_gpio(TOS_WRITE_STATUS, MSG_SENT);
        /* Wait terminal read msg */
        while (GetGpio(TERMINAL_READ_STATUS) != MSG_RECEIVED)
            Wait(10);
        set_gpio(TOS_WRITE_STATUS, MSG_IDLE);

    }
    return 0;
}

/* 
 * GpioInputSetup()
 * ----------------
 *  Setup gpio input pins
 *
 *  Parameters:
 *  terminal_bits: pin to receive bits from terminal
 *  terminal_read: pin to get terminal read status
 *  terminal_write: pin to get terminal write status
 */
void GpioInputSetup(int terminal_bits, int terminal_read, int terminal_write) {
    /* Setup Input (Follow the instruction in the manual)*/
    set_pull_up_down(terminal_bits, PUD_DOWN);
    set_gpio_function(terminal_bits, GPIO_INPUT); /* Enable GPIO Input Pin */
    set_pull_up_down(terminal_read, PUD_DOWN);
    set_gpio_function(terminal_read, GPIO_INPUT);
    set_pull_up_down(terminal_write, PUD_DOWN);
    set_gpio_function(terminal_write, GPIO_INPUT);
    /* Setup Channel */
    TERMINAL_INPUT_BITS = terminal_bits;
    TERMINAL_READ_STATUS = terminal_read;
    TERMINAL_WRITE_STATUS = terminal_write;
}

/* 
 * GpioOutputSetup()
 * -----------------
 *  Setup gpio output pins
 *
 *  Parameters:
 *  tos_bits: pin to send bits to terminal
 *  tos_read: pin to indicate TOS read status
 *  tos_write: pin to indicate TOS write status
 */
void GpioOutputSetup(int tos_bits, int tos_read, int tos_write) {
    /* Setup Output */
    set_gpio_function(tos_bits, GPIO_OUTPUT); /* Enable GPIO Output Pin */
    set_gpio(tos_bits, GPIO_LOW); /* Initial output low voltage */
    set_gpio_function(tos_read, GPIO_OUTPUT);
    set_gpio(tos_read, GPIO_LOW);
    set_gpio_function(tos_write, GPIO_OUTPUT);
    set_gpio(tos_write, GPIO_LOW);
    TOS_OUTPUT_BITS = tos_bits;
    TOS_READ_STATUS = tos_read;
    TOS_WRITE_STATUS = tos_write;
}

/**
 * write_string()
 * --------------
 * Write a string to monitor
 * 
 * @param str string. 
 * @return 0: Success, -1: Error.
 */
int write_string(char *str) {
    //volatile unsigned int cpsr_flag;
    /* When write to share memory, context swtich should be disabled */
    //SAVE_CPSR_DIS_IRQ(cpsr_flag);
    int value[BIT_LENGTH], ret;
    char *ptr = str;
    while (*ptr != '\0') {
        charToBin((unsigned char) *ptr, value, BIT_LENGTH);
        ret = WriteChar(value, BIT_LENGTH);
        if (ret == -1) {
            error();
            return -1;
        }
        ptr += 1;
    }
    charToBin((unsigned char) '\0', value, BIT_LENGTH);
    ret = WriteChar(value, BIT_LENGTH);
    if (ret == -1) {
        error();
        return -1;
    }
    //RESUME_CPSR(cpsr_flag);
    return 0;
}

/**
 * debug()
 * -------
 *  Turn on the ACT LED, won't return
 */
void debug() {
    while (1) {
        set_gpio_function(16, 1);
        set_gpio(16, 0);
        Wait(250000);
    }
}

/**
 * error()
 * -------
 *  TODO Turn on a red LED, won't return
 */
void error() {
    while (1) {
        set_gpio_function(16, 1);
        set_gpio(16, 0);
        Wait(100000);
        set_gpio(16, 1);
        Wait(100000);
    }
}
