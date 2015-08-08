#include <kernel.h>

void _start() __attribute__ ((section (".init")));
int main();
const int INPUT_CHANNELS[CHANNEL_NUMBER] = {14, 15, 18, 23, 24, 25, 8}; 
const int TERMINAL_STATUS_CHANNEL = 7; 
const int OUTPUT_CHANNELS[CHANNEL_NUMBER] = {2, 3, 4, 17, 27, 22, 10}; 
const int TOS_STATUS_CHANNEL = 9; 

void _start(){
        asm("mov sp, #0x8000");         /* Setup stack pointer */
        main();
}

int main(){
        //while(1) {
        //        SetGpioFunction(16, 1);
        //        SetGpio(16, 0);
        //        Wait(250000);
        //}
        int i, value[CHANNEL_NUMBER];
        
        /* Setup Output */
        for(i=0; i<CHANNEL_NUMBER; i++) {
                SetGpioFunction(OUTPUT_CHANNELS[i], GPIO_OUTPUT);       /* Enable GPIO Output Pin */
                SetGpio(OUTPUT_CHANNELS[i], GPIO_LOW);                  /* Initial output low voltage */
        }
        SetGpioFunction(TOS_STATUS_CHANNEL, GPIO_OUTPUT);       /* Enable Tos Status Pin */ 
        SetGpio(TOS_STATUS_CHANNEL, GPIO_LOW);                  /* Initial output low voltage */

        /* Setup Input (Follow the instruction in the manual)*/
        for(i=0; i<CHANNEL_NUMBER; i++) {
                SetPullUpDn(INPUT_CHANNELS[i], PUD_DOWN);
                SetGpioFunction(INPUT_CHANNELS[i], GPIO_INPUT);       /* Enable GPIO Input Pin */ 
        }
        SetPullUpDn(TERMINAL_STATUS_CHANNEL, PUD_DOWN);
        SetGpioFunction(TERMINAL_STATUS_CHANNEL, GPIO_INPUT);       /* Enable Terminal Status Pin */ 
        
        while(1){
                /* Wait until Terminal Status change to MSG_SENT */
                while (GetGpio(TERMINAL_STATUS_CHANNEL) != MSG_SENT) 
                       Wait(10);
                /* Read Message */
                for(i=0; i<CHANNEL_NUMBER; i++) {
                        value[i] = GetGpio(INPUT_CHANNELS[i]);
                }
                /* Set TOS Status to MSG_RECEIVED */
                SetGpio(TOS_STATUS_CHANNEL, MSG_RECEIVED);
                //while(1) {
                //        SetGpioFunction(16, 1);
                //        SetGpio(16, 0);
                //        Wait(250000);
                //}
                /* Wait until Terminal Status change to MSG_CLEAR */
                while (GetGpio(TERMINAL_STATUS_CHANNEL) != MSG_CLEAR) 
                        Wait(10);
                /* Sent Message back */
                for(i=0; i<CHANNEL_NUMBER; i++) {
                        SetGpio(OUTPUT_CHANNELS[i], value[i]);
                }
                /* Set TOS Status to MSG_REPLIED */
                SetGpio(TOS_STATUS_CHANNEL, MSG_REPLIED);
        }
}
