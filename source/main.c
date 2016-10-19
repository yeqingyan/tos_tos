#include <kernel.h>
// TODO
int kernel_main();

// void test_process_1(PROCESS, PARAM);
//
// void test_1_foo() {
//    asm("nop");
//}
//
// void test_process_1(PROCESS self, PARAM param) {
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
// void test_2_foo() {
//    asm("nop");
//}
//
// void test_process_2(PROCESS self, PARAM param) {
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
  extern unsigned char __bss_start__;
  extern unsigned char __bss_end__;
  unsigned char *dst;

  // Rapsberry Pi won't zero out bss segment, we need to zero out them by
  // ourselves.
  dst = &__bss_start__;
  while (dst < &__bss_end__) {
    *dst++ = 0;
  }

  init_video(); // Screen initilize
  // kprintf("Init frame buffer Done\n");

  init_process();
  // kprintf("Init Process Done\n");

  init_dispatcher();
  // kprintf("Init Dispatcher Done\n");

  init_ipc();
  kprintf("Init ipc Done\n");

  init_null_process();
  init_usb_busy_wait();

  init_shell();
  init_lines_test();
  init_serial();
  init_interrupts();

  while (1) {
  };

  return 0;
}
