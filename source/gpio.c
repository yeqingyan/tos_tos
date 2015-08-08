#include <kernel.h>

/*
 * Wait 150 cycles
 */
void short_wait(void) {
        int i;
        
        for (i=0; i<150; i++) {
                asm volatile("nop");
        }
}

int GetGpioAddress() {
        return 0x20200000;
}

/*
 * SetGpioFunction()
 * -----------------
 *  Choose Gpio Function
 *
 *  Parameters:
 *  pin: GPIO pin number
 *  function: the function want to use. 0: input, 1: output
 */ 
void SetGpioFunction(int pin, int function) {
        if(pin > 53)    goto ERROR;     /* Check pin between 0 to 53 */
        if(pin < 0)     goto ERROR;
        if(function > 8) goto ERROR;    /* Check function between 0 to 7 */
        if(function < 0) goto ERROR;    
        
        int* gpio_addr = (int *)GetGpioAddress();
        
        gpio_addr += pin/10;  /* Find gpio address */

        function = function << (pin % 10 * 3);
        int mask = 7 << (pin % 10 * 3); /* Create mask */
        mask = ~mask;
        
        int gpio_val = *gpio_addr;      /* Got old bit set */
        gpio_val = gpio_val & mask;
        gpio_val = gpio_val ^ function; /* Got final bit set */
        
        *gpio_addr = gpio_val;

ERROR:  return;
}

void SetGpio(int pin, int pinVal){
        if(pin > 53) goto ERROR;
        if(pin < 0) goto ERROR;

        int* gpio_addr = (int *)GetGpioAddress();

        gpio_addr += pin/32;
        
        int bitValue;
        bitValue = 1 << (pin%32);

        if (pinVal == 0) {
                *(gpio_addr+CLR_OFFSET) = bitValue;   /* Turn off pin */
        } else {        
                *(gpio_addr+SET_OFFSET) = bitValue;   /* Turn on pin */
        }
         
ERROR:  return;
}

void SetPullUpDn(int pin, int pud) {
        int* gpio_addr = (int *)GetGpioAddress();
        
        /* 1. Write to GPPUD(GPIO Pull-Up/Down Register) */
        if (pud == PUD_DOWN)
                *(gpio_addr+GPPUD_OFFSET) = (*(gpio_addr+GPPUD_OFFSET) & ~3) | PUD_DOWN;
        else if (pud == PUD_UP)
                *(gpio_addr+GPPUD_OFFSET) = (*(gpio_addr+GPPUD_OFFSET) & ~3) | PUD_UP;
        else    /* pud == PUD_OFF */
                *(gpio_addr+GPPUD_OFFSET) &= ~3;

        /* 2. Wait 150 cycles */         
        short_wait();
        
        /* 3. Write to GPPUDCLK(GPIO Pull-Up/Down Clock Register) */
        int clk_offset = GPPUDCLK_OFFSET + (pin/32);
        *(gpio_addr+clk_offset) = 1 << (pin%32);

        /* 4. Wait 150 cycles*/     
        short_wait();

        /* 5. Write to GPPUD to remove the control signal */
        *(gpio_addr+GPPUD_OFFSET) &= ~3;

        /* 6. Write to GPPUDCLK to remove the clock */
        *(gpio_addr+clk_offset) = 0;
} 

int GetGpio(int pin) {
        int* gpio_addr = (int *)GetGpioAddress();
        int offset = GPLEV_OFFSET+(pin/32);
        int mask = (1 << pin%32);
        int value;
        value = *(gpio_addr+offset) & mask;
        if (value)
                return GPIO_HIGH;
        else
                return GPIO_LOW;
}
