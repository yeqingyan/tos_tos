#include <kernel.h>

void _start() __attribute__ ((section (".init")));
int main();

void _start(){
        asm("mov sp, #0x8000");         /* Setup stack pointer */
        main();
}

int main(){
        int ret;
        
        char* test_str = "Valar morghulis";

        init_process();
        init_dispatcher();
           
        /* Setup GPIO pins */
        /* TERMINAL_BITS: 25, TERMINAL_READ: 7, TERMINAL_WRITE: 8 */
        GpioInputSetup(25, 7, 8);
        /* TOS_BITS: 14, TOS_READ: 18, TOS_WRITE: 15 */
        GpioOutputSetup(14, 18, 15);
        while(1){
                /*
                ret = ReadChar(value, BIT_LENGTH);
                if(ret == -1) {
                        error();
                }*/
                ret = WriteString(test_str);
                if(ret == -1) {
                        error();
                }
        }
}
