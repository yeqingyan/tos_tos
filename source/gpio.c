#include <kernel.h>
#define GPIO_PIN(P) (P)
#define GPIO_REG_SIZE  32
#define GPIO_TOTAL_PIN 54

/**
 * busy wait 150 cycles
 */
void short_wait(void) {
  int i;
  for (i = 0; i < 150; i++) {
    asm volatile("nop");
  }
}


/**
 *  wait 
 */
void delay(int time) {
  Wait(time*1000);
}

/**
 * Return GPIO base address.
 * @return GPIO base address
 */
int get_gpio_address() 
 { return GPIO_BASE;  //0x20200000
 }

/**
 * Setup Gpio Function. (Ref. BCM2835 6.1 GPIO Function Select)
 *
 * @param pin       GPIO pin number (Pin must between 0 and 53)
 * @param function  The function want to use. (Between 0 and 7)
 * 	000 = GPIO Pin is an GPIO_INPUT 
	001 = GPIO Pin is an GPIO_OUTPUT 
	100:010 = GPIO Pin takes alternate function  
 */
void set_gpio_function(int pin, int function) {
  if ((pin > GPIO_TOTAL_PIN) || (pin < 0))
    return;

  if ((function > 8) || (function < 0))
    return;

  int *gpio_addr = (int *)get_gpio_address();

  gpio_addr += pin / 10; // Find gpio address

  function = function << (pin % 10 * 3);
  int mask = 7 << (pin % 10 * 3); // Create mask
  mask = ~mask;

  int gpio_val = *gpio_addr; // Got old bits set
  gpio_val = gpio_val & mask;
  gpio_val = gpio_val ^ function; // Got final bits set
  *gpio_addr = gpio_val;
  return;
}

/**
 * Write GPIO_HIGH/GPIO_LOW value to gpio pins.
 * (Ref. BCM2835 6.1 GPIO Pin GPIO_OUTPUT Set/Clear)
 *
 * @param GPIO pin number
 * @param GPIO_HIGH/GPIO_LOW value
 */
void set_gpio(int pin, int pinvalue) {
  // Check if the pin between 0 to 53
  if ((pin > GPIO_TOTAL_PIN) || (pin < 0))
    return;

  if ((pinvalue != GPIO_LOW) && (pinvalue != GPIO_HIGH))
    return;

  int *gpio_addr = (int *)get_gpio_address();
  gpio_addr += pin / GPIO_REG_SIZE;
  int bitvalue = 1 << (pin % GPIO_REG_SIZE);

  if (pinvalue == 0) {
    *(gpio_addr + GPCLR) = bitvalue; // Turn off pin
  } else {
    *(gpio_addr + GPSET) = bitvalue; // Turn on pin
  }

  return;
}

/**
 * Setup pull-up/down resistor on GPIO pins
 * Note: GPIO pin 2,3 have physical pull-up resistor attach to them, set
 * pull-down resistor on those pins won't work.
 * (Ref. BCM2835 6.1 GPIO Pull-up/down Clock Registers )
 *
 * @param pin   GPIO pin number
 * @param pud   PUD_DOWN or PUD_UP
 */
void set_pull_up_down(int pin, int pud) {
  int *gpio_addr = (int *)get_gpio_address();

  // 1. Write to GPPUD(GPIO Pull-Up/Down Register)
  if (pud == PUD_DOWN)
    *(gpio_addr + GPPUD) = (*(gpio_addr + GPPUD) & ~3) | PUD_DOWN;
  else if (pud == PUD_UP)
    *(gpio_addr + GPPUD) = (*(gpio_addr + GPPUD) & ~3) | PUD_UP;
  else /* pud == PUD_OFF */
    *(gpio_addr + GPPUD) &= ~3;

  // 2. Wait 150 cycles
  short_wait();

  // 3. Write to GPPUDCLK(GPIO Pull-Up/Down Clock Register)
  int clk_offset = GPPUDCLK + (pin / GPIO_REG_SIZE);
  *(gpio_addr + clk_offset) = 1 << (pin % GPIO_REG_SIZE);

  // 4. Wait 150 cycles*/
  short_wait();

  // 5. Write to GPPUD to remove the control signal
  *(gpio_addr + GPPUD) &= ~3;

  // 6. Write to GPPUDCLK to remove the clock
  *(gpio_addr + clk_offset) = 0;
}

/**
 * Get value of GPIO pins. (Ref. BCM2835 6.1 GPIO Pin Level )
 *
 * @param pin   GPIO pin number
 * @return      GPIO pin value
 */
int read_gpio(int pin) {
  int *gpio_addr = (int *)get_gpio_address();
  int offset = GPLEV + (pin / GPIO_REG_SIZE);
  int mask = (1 << pin % GPIO_REG_SIZE);
  int value = *(gpio_addr + offset) & mask;
  if (value)
    return GPIO_HIGH;
  else
    return GPIO_LOW;
}

void gpio_test() 
{
  	set_gpio_function(GPIO_PIN(17), GPIO_OUTPUT);
	set_pull_up_down(GPIO_PIN(10),PUD_DOWN);
	set_pull_up_down(GPIO_PIN(15),PUD_UP);

	set_pull_up_down(GPIO_PIN(7),PUD_DOWN);
	set_pull_up_down(GPIO_PIN(18),PUD_UP);


    set_gpio_function(GPIO_PIN(14), GPIO_INPUT);
    set_gpio_function(GPIO_PIN(2), GPIO_INPUT);
    set_gpio_function(GPIO_PIN(23), GPIO_OUTPUT);
    set_gpio_function(GPIO_PIN(21), GPIO_OUTPUT);
	
    set_gpio(GPIO_PIN(14), GPIO_LOW);
    set_gpio(GPIO_PIN(2), GPIO_HIGH);
    set_gpio(GPIO_PIN(23), GPIO_LOW);
    set_gpio(GPIO_PIN(21), GPIO_HIGH);

  while (1) 
   {
	//delay(100);
	set_gpio(GPIO_PIN(17),GPIO_HIGH);
	delay(1000);
	set_gpio(GPIO_PIN(17),GPIO_LOW);
	delay(1000);
	if (read_gpio(GPIO_PIN(14)))
	   set_gpio(GPIO_PIN(23),GPIO_HIGH);
	else
	   set_gpio(GPIO_PIN(23),GPIO_LOW);

	if (read_gpio(GPIO_PIN(2)))
	   set_gpio(GPIO_PIN(21),GPIO_LOW);
	else
	   set_gpio(GPIO_PIN(21),GPIO_HIGH);  
  }
  return;

}
