#include <kernel.h>

void _start() __attribute__ ((section (".init")));
int main();
void debug();
/* Tos Input/Output Channel setup */
const int TERMINAL_INPUT_BITS   = 25;
const int TERMINAL_WRITE_STATUS = 8;
const int TERMINAL_READ_STATUS  = 7;
const int TOS_OUTPUT_BITS       = 14;
const int TOS_WRITE_STATUS      = 15;
const int TOS_READ_STATUS       = 18;


void _start(){
        asm("mov sp, #0x8000");         /* Setup stack pointer */
        main();
}

int main(){
        int i, value[BIT_LENGTH];
        
        /* Setup Output */
        SetGpioFunction(TOS_OUTPUT_BITS, GPIO_OUTPUT);              /* Enable GPIO Output Pin */
        SetGpio(TOS_OUTPUT_BITS, GPIO_LOW);                         /* Initial output low voltage */
        SetGpioFunction(TOS_WRITE_STATUS, GPIO_OUTPUT);
        SetGpio(TOS_WRITE_STATUS, GPIO_LOW);
        SetGpioFunction(TOS_READ_STATUS, GPIO_OUTPUT);
        SetGpio(TOS_READ_STATUS, GPIO_LOW);

        /* Setup Input (Follow the instruction in the manual)*/
        SetPullUpDn(TERMINAL_INPUT_BITS, PUD_DOWN);
        SetGpioFunction(TERMINAL_INPUT_BITS, GPIO_INPUT);                /* Enable GPIO Input Pin */ 
        SetPullUpDn(TERMINAL_WRITE_STATUS, PUD_DOWN);
        SetGpioFunction(TERMINAL_WRITE_STATUS, GPIO_INPUT);       
        SetPullUpDn(TERMINAL_READ_STATUS, PUD_DOWN);
        SetGpioFunction(TERMINAL_READ_STATUS, GPIO_INPUT);       
        
        while(1){

                /* Read */
                for (i=0; i<BIT_LENGTH; i++) {
                        /* Check Error */
                        if (GetGpio(TOS_READ_STATUS) != IDLE) { 
                                debug();
                        }
                        /* Wait until Terminal Status change to MSG_SENT */
                        while (GetGpio(TERMINAL_WRITE_STATUS) == IDLE)
                                Wait(10);
                        /* Read bits */
                        value[i] = GetGpio(TERMINAL_INPUT_BITS);
                        SetGpio(TOS_READ_STATUS, MSG_RECEIVED);

                        /* Make sure terminal side is finished */
                        while (GetGpio(TERMINAL_WRITE_STATUS) != IDLE)
                                Wait(10);
                        SetGpio(TOS_READ_STATUS, IDLE);
                }

                /* Write */
                for (i=0; i<BIT_LENGTH; i++) {
                        /* Check error */
                        if (GetGpio(TOS_WRITE_STATUS) != IDLE)
                                debug();

                        /* Make sure terminal side is ready */
                        while (GetGpio(TERMINAL_READ_STATUS) != IDLE)
                                Wait(10);

                        SetGpio(TOS_OUTPUT_BITS, value[i]);
                        SetGpio(TOS_WRITE_STATUS, MSG_SENT);
                        /* Wait terminal read msg */
                        while (GetGpio(TERMINAL_READ_STATUS) != MSG_RECEIVED)
                                Wait(10);
                        SetGpio(TOS_WRITE_STATUS, IDLE);

                }
                
                ///* Wait until Terminal Status change to MSG_SENT */
                //while (GetGpio(TERMINAL_STATUS_CHANNEL) != MSG_SENT) 
                //       Wait(10);
                ///* Read Message */
                //for(i=0; i<CHANNEL_NUMBER; i++) {
                //        value[i] = GetGpio(INPUT_CHANNELS[i]);

                //}
                ///* Set TOS Status to MSG_RECEIVED */
                //SetGpio(TOS_STATUS_CHANNEL, MSG_RECEIVED);
                ////}
                ///* Wait until Terminal Status change to MSG_CLEAR */
                //while (GetGpio(TERMINAL_STATUS_CHANNEL) != MSG_CLEAR) 
                //        Wait(10);
                ///* Sent Message back */
                //for(i=0; i<CHANNEL_NUMBER; i++) {
                //        SetGpio(OUTPUT_CHANNELS[i], value[i]);
                //}
                ///* Set TOS Status to MSG_REPLIED */
                //SetGpio(TOS_STATUS_CHANNEL, MSG_REPLIED);
        }
}

void debug() {
        while(1) {      
                SetGpioFunction(16, 1);
                SetGpio(16, 0);
                Wait(250000);
        }
}
