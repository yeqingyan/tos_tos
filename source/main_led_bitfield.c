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
