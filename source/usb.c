#include <kernel.h>
// TOOD add comments
PORT usb_notifier_port;

// In usb.c or keyb.c, if the function name is CamelCase, that function is 
// from libcsud 

// busy wait or interrupt usb. Used by interrupt init_interrupts()          
int intr_usb;

/**
 * Initialize usb busy wait version
 */
void init_usb_busy_wait() {
    intr_usb = 0;
    UsbInitialise();
    keyboard_address = 0;
    init_keyb();
}

/**
 * Initialize usb interrupt version
 */
void init_usb_intr() {
    intr_usb = 1;
    UsbInitialise();
    init_usbkeyb();
}

/**
 * Enable usb interrupts.
 */
//void init_usb_interrupts() {
////    usb_transfer_result = USB_TRANSFER_NULL;
//    EnableUSBInterrupt(); // Call func in libcsud to enable usb.
//    isr_table[USB_IRQ] = isr_usb;
//    enable_irq(USB_IRQ);
//}

/**
 * usb isr
 */
//void isr_usb(void) {
//    int result;
//    USBIRQHandler();
//    result = USBIRQHandler();
//    switch (result) {
//            /* Receive unknown irq */
//        case 0:
//            kprintf("[TOS][isr_usb]Got unknown interrupt\n");
//            assert(0);
//            break;
//        case 1:
//            usb_transfer_result = USB_TRANSFER_COMPLETE;
//            break;
//        case 2:
//            usb_transfer_result = USB_TRANSFER_NAK;
//            break;
//        case 3:
//            usb_transfer_result = USB_TRANSFER_RESTART;
//            break;
//        default:
//            kprintf("Got unknow result %d!\n", result);
//            usb_transfer_result = USB_TRANSFER_NULL;
//    }
//}
