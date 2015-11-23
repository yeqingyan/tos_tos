#include <kernel.h>

//void _start() __attribute__ ((section (".init")));
int main();
void test_process_1(PROCESS , PARAM );

/*
void _start(){
        asm("mov sp, #0xA0000");         // Setup stack pointer 
        main();
}
*/

void test_process_1(PROCESS self, PARAM param) {
        char* test_str = "This is process A";
        int ret;
        while(1) {
                ret = WriteString(test_str);
                if(ret == -1) {
                        error();
                }
                resign();
        }
}

void test_process_2(PROCESS self, PARAM param) {
        char* test_str = "This is process B";
        int ret;
        while(1) {
                ret = WriteString(test_str);
                if(ret == -1) {
                        error();
                }
                resign();
        }
}

int main(){
   
        /* Setup GPIO pins */
        /* TERMINAL_BITS: 25, TERMINAL_READ: 7, TERMINAL_WRITE: 8 */
        //GpioInputSetup(25, 7, 8);
        /* TOS_BITS: 14, TOS_READ: 18, TOS_WRITE: 15 */
        //GpioOutputSetup(14, 18, 15);

        init_process();
        //WriteString("Init Process Done");
        init_dispatcher();
        //WriteString("Init Dispatcher Done");
        init_interrupts();
        //create_process(test_process_1, 5, 42, "TEST PROCESS A");
        //create_process(test_process_2, 5, 42, "TEST PROCESS B");
        while(1) {
        }
        //resign();
        return 0;
}
