#ifndef __KERNEL__
#define __KERNEL__

/* main.c */
#define CHANNEL_NUMBER 7
#define MSG_CLEAR    0
#define MSG_SENT     1
#define MSG_REPLIED  0
#define MSG_RECEIVED 1

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
/* systemTimer.c */
int GetSystemTimerBase();
int GetTimeStamp();
void Wait(int);
#endif
