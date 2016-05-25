#include <kernel.h>
// TODO
int kernel_main();

//void test_process_1(PROCESS, PARAM);
//
//void test_1_foo() {
//    asm("nop");
//}
//
//void test_process_1(PROCESS self, PARAM param) {
//    char *test_str = "This is process A\n";
//    int ret;
//    /*
//    while (1) {
//        test_1_foo();
//    }
//    */
//    while (1) {
//        kprintf(test_str);
//        //ret = WriteString(test_str);
//        if (ret == -1) {
//            assert(0);
//        }
//        //   resign();
//    }
//}
//
//void test_2_foo() {
//    asm("nop");
//}
//
//void test_process_2(PROCESS self, PARAM param) {
//    char *test_str = "This is process B\n";
//    int ret;
//    /*
//    while(1) {
//        test_2_foo();
//    }*/
//
//    while (1) {
//        kprintf(test_str);
//        //ret = WriteString(test_str);
//        if (ret == -1) {
//            assert(0);
//        }
//        //resign();
//    }
//}
int kernel_main() {
    //int i;
    //error();
    extern unsigned char __bss_start__;
    extern unsigned char __bss_end__;
    unsigned char *dst;
    // zero out bss
    dst = &__bss_start__;
    while (dst < &__bss_end__) *dst++ = 0;
    /* Setup GPIO pins */
    /* TERMINAL_BITS: 25, TERMINAL_READ: 7, TERMINAL_WRITE: 8 */
    //GpioInputSetup(25, 7, 8);
    /* TOS_BITS: 14, TOS_READ: 18, TOS_WRITE: 15 */
    //GpioOutputSetup(14, 18, 15);
    //error();
    init_video();
    //kprintf("Init frame buffer Done\n");

    init_process();
    //kprintf("Init Process Done\n");

    init_dispatcher();
    //kprintf("Init Dispatcher Done\n");

    init_ipc();
    kprintf("Init ipc Done\n");
    
//    create_process(test_process_1, 5, 42, "TEST PROCESS A");
//    create_process(test_process_2, 5, 42, "TEST PROCESS B");
//    init_interrupts();
//    while (1) {};
    
    init_null_process();
    init_usb_busy_wait();

    init_shell();
    init_lines_test();
    init_interrupts();

    while (1);
    return 0;
}
