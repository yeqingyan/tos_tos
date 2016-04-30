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
    
int kernel_main() {  
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
     
    // Step 2. Enable input for pin 8, output for pin 7
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
