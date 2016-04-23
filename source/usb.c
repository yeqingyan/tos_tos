#include <kernel.h>

/*
 * init_usb()
 *
 * Initalize usb
 */
void init_usb() {
    int ret;

    ret = UsbInitialise();
    if (ret != 0) {
        kprintf("USB initialise meet error! \n", ret);
    }

}

