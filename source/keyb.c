#include <kernel.h>

// Not done yet.
unsigned int keyboard_address;
unsigned int keyboard_address_intr;
PORT usb_notifier_port;

// TODO kntest
int kntest = 0;
short old_keys[6];
short keys_buf1[6];
short keys_buf2[6];
short *old_keys_ptr;
short *new_keys_ptr;
short old_keys_intr[6];
BYTE new_key;
PORT keyb_port;
BYTE keys_normal[104] = {0x0, 0x0, 0x0, 0x0, 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0',
    '\n', 0x0, '\b', '\t', ' ', '-', '=', '[',
    ']', '\\', '#', ';', '\'', '`', ',', '.',
    '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
    '\n', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '\\', 0x0, 0x0, '='};

BYTE keys_shift[104] = {0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
    'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
    '#', '$', '%', '^', '&', '*', '(', ')',
    '\n', 0x0, '\b', '\t', ' ', '_', '+', '{',
    '}', '|', '~', ':', '@', '~', '<', '>',
    '?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
    '\n', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '|', 0x0, 0x0, '='};

void keyb_process(PROCESS, PARAM);
void keyb_notifier(PROCESS, PARAM);
PROCESS keyb_notifier_proc;

void LogPrint(const char *message, int messageLength) {
    while (messageLength != 0) {
        output_char(kernel_window, *message++);
        messageLength--;
    }
}

unsigned int get_keyboard(){
    int keyboard_count;
    unsigned int keyboard_addr = 0;
    UsbCheckForChange();
    keyboard_count = KeyboardCount();
    assert(keyboard_count > 0);

    keyboard_addr = KeyboardGetAddress(0);
    assert(keyboard_addr != 0);
    return keyboard_addr;
}

void init_keyb() {
    keyb_port = create_process(keyb_process, 5, 0, "Keyboard Process");
}

PORT init_usbkeyb() {
    keyboard_address_intr = get_keyboard();
    assert(keyboard_address_intr != 0);
    int i;
    for (i=0; i<6; i++) {
        keys_buf1[i] = '\0';
        keys_buf2[i] = '\0';
    }
    old_keys_ptr = keys_buf1;
    new_keys_ptr = keys_buf2;
    keyb_port = create_process(usbkeyb_process, 6, 0, "USB Keyboard Process");
    return keyb_port;
}

/*
 * keyb_notifier
 *
 * Notify keyboard_process, the keyboard was pressed.
 */
void keyb_notifier(PROCESS self, PARAM param) {
    volatile unsigned int cpsr_flag;
    Keyb_Message msg;
    BYTE key;
//    kprintf("Keyb notifier start\n");
    assert(keyboard_address == 0);
    
    while (1) {
        SAVE_CPSR_DIS_IRQ(cpsr_flag);
//        kprintf("Call keyboard_update\n");
        keyboard_update();
        key = keyboard_get_char();
//        kprintf("Got new key %x\n", key);
        RESUME_CPSR(cpsr_flag);
        if (key != 0) {
            new_key = key;
            msg.key_buffer = (BYTE *) & new_key;
//            kprintf("Got new key %c\n", *(msg.key_buffer));
            message(keyb_port, &msg);
        }
    }
}

/*
 * keyboard_update
 * Based on http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/input01.html
 *
 * 1.  Retrieve a stored keyboard address (initially 0).
 * 2.  If this is not 0, go to 9.
 * 3.  Call UsbCheckForChange to detect new keyboards.
 * 4.  Call KeyboardCount to detect how many keyboards are present.
 * 5.  If this is 0 store the address as 0 and return; we can't do anything with no keyboard.
 * 6.  Call KeyboardGetAddress with parameter 0 to get the first keyboard's address.
 * 7.  Store this address.
 * 8.  If this is 0, return; there is some problem.
 * 9.  Call KeyboardGetKeyDown 6 times to get each key currently down and store them
 * 10. Call KeyboardPoll
 * 11. If the result is non-zero go to 3. There is some problem (such as disconnected keyboard).
 */
void keyboard_update() {
    int keyboard_count;
    int i;

    do {
        if (keyboard_address == 0) {
            UsbCheckForChange();
            keyboard_count = KeyboardCount();
            kprintf("Got %d keyboard\n", keyboard_count);
            if (keyboard_count == 0) {
                keyboard_address = 0;
                kprintf("No keyboard found\n", keyboard_count);
                return;
            }

            keyboard_address = KeyboardGetAddress(0);
            if (keyboard_address == 0) {
                kprintf("Error! Keyboard address is %d\n", keyboard_address);
                return;
            }
        }
        // Call KeyboardGetKeyDown 6 times to get each key currently down and store them
        for (i = 0; i < 6; i += 1) {
            old_keys[i] = KeyboardGetKeyDown(keyboard_address, i);
        }
    } while (KeyboardPoll(keyboard_address) != 0);
    return;
}

/*
 * keyboard_get_char
 *
 * 1. Check if KeyboardAddress is 0. If so, return 0.
 * 2. Call KeyboardGetKeyDown up to 6 times. Each time:
 *      2.1 If key is 0, exit loop.
 *      2.2 Call KeyWasDown. If it was, go to the next key.
 *      2.3 If the scan code is more than 103, go to the next key.
 *      2.4 Call KeyboardGetModifiers
 *      2.5 If shift is held, load the address of KeysShift. Otherwise load KeysNormal.
 *      2.6 Read the ASCII value from the table.
 *      2.7 If it is 0, go to the next key otherwise return this ASCII code and exit.
 * 3. Return 0.
 */
int keyboard_get_char() {
    int i, index, key;
    BYTE modifiers, ascii_char;

    index = 0;
    assert(keyboard_address != 0);

    for (i = 0; i < 6; i += 1) {
        key = KeyboardGetKeyDown(keyboard_address, index);
        if (key == 0) {
            return 0;
        }
        if ((key > 103) || (key_was_down(key) == TRUE)) {
            continue;
        }
        modifiers = KeyboardGetModifiers(keyboard_address);
        /* 0x22: 0b00100010: Right shift and Left Shift */
        if (modifiers & 0x22) {
            ascii_char = keys_shift[key];
        } else {
            ascii_char = keys_normal[key];
        }
        if (ascii_char != 0) {
            return ascii_char;
        }
    }
    return 0;
}

/*
 * keyboard_get_char
 *
 * 1. Check if KeyboardAddress is 0. If so, return 0.
 * 2. Call KeyboardGetKeyDown up to 6 times. Each time:
 *      2.1 If key is 0, exit loop.
 *      2.2 Call KeyWasDown. If it was, go to the next key.
 *      2.3 If the scan code is more than 103, go to the next key.
 *      2.4 Call KeyboardGetModifiers
 *      2.5 If shift is held, load the address of KeysShift. Otherwise load KeysNormal.
 *      2.6 Read the ASCII value from the table.
 *      2.7 If it is 0, go to the next key otherwise return this ASCII code and exit.
 * 3. Return 0.
 */

void reset_keys_buffer(short * ptr) {
    int i;
    for (i=0; i<6; i++) {
        *(ptr+i) = '\0';
    }
}
int keyboard_get_char_intr(volatile unsigned int kbd_address) {
    int i, index, key;
    short *tmp;
    BYTE modifiers, ascii_char;

    index = 0;
    assert(kbd_address != 0);
    
    for (i = 0; i < 6; i += 1) {
        key = KeyboardGetKeyDown(kbd_address, index);
        if (key == 0) {
            reset_keys_buffer(new_keys_ptr);
            ascii_char = 0;
            break;
        }
        if ((key > 103) || (key_was_down_intr(key) == TRUE)) {
            *(new_keys_ptr+i) = key;
            continue;
        }
        modifiers = KeyboardGetModifiers(kbd_address);
        /* 0x22: 0b00100010: Right shift and Left Shift */
        if (modifiers & 0x22) {
            ascii_char = keys_shift[key];
        } else {
            ascii_char = keys_normal[key];
        }
        if (ascii_char != 0) {
            *(new_keys_ptr+i) = key;
            break;
        }
    }
        
    tmp = old_keys_ptr;
    old_keys_ptr = new_keys_ptr;
    new_keys_ptr = tmp;
    
    return ascii_char;
}

/*
 * key_was_down()
 *
 * Check if the key already hold down
 *
 * key: Pressed key got from keyboard
 * return: True if key already been press down in previous loop, otherwise return False.
 */
BOOL key_was_down(int key) {
    int i;
    for (i = 0; i < 6; i += 1) {
        if (old_keys[i] == key) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL key_was_down_intr(int key) {
    int i;
    for (i = 0; i < 6; i += 1) {
        if (*(old_keys_ptr+i) == key) {
            return TRUE;
        }
    }
    return FALSE;
}

void disable_keyboard() {
    remove_ready_queue(keyb_notifier_proc);
}

void enable_keyboard() {
    add_ready_queue(keyb_notifier_proc);
}


void keyboard_update_intr(unsigned int keyboard_address_intr) {
    int res;
//    res = KeyboardPollNoWait(keyboard_address_intr);
    //kprintf("Send Keyboard Polling meg return %d\n", res);



    //    
    //    do {
    //        for (i = 0; i < 6; i += 1) {
    //            old_keys_intr[i] = KeyboardGetKeyDown(keyboard_address_intr, i);
    //        }
    //    } while (KeyboardPollIntr(keyboard_address_intr) != 0);

}

/*
 * keyb_process
 *
 * Generate keyboard process
 */
void keyb_process(PROCESS self, PARAM param) {
    int key_waiting;
    char key_buffer;
    PORT keyb_notifier_port;

    PROCESS sender_proc;
    PROCESS client_proc;
    Keyb_Message *msg;
    Keyb_Message *client_msg;

    keyboard_address = 0;

    keyb_notifier_port = create_process(keyb_notifier, 4, 0, "Keyboard Notifier");   
    keyb_notifier_proc = keyb_notifier_port->owner;

    client_proc = NULL;
    client_msg = NULL;
    key_buffer = '\0';
    key_waiting = FALSE;

    while (1) {
        msg = (Keyb_Message *) receive(&sender_proc);
        if (sender_proc == keyb_notifier_proc) {
            // the notifier has sent us a new keystroke 
//            kprintf("Got key %x\n", *msg->key_buffer);
            if (client_proc != NULL) {
                /* and there is a client waiting. Just return
                   the keystroke to the client. */
                *client_msg->key_buffer = *msg->key_buffer;
                //kprintf("keyb reply to wait client\n");
                reply(client_proc);
                client_proc = NULL;
            } else {
                /* no client is waiting. Save the keystroke.
                   Note that we should use a queue here. */
                key_buffer = *msg->key_buffer;
                key_waiting = TRUE;
            }
        } else {
            /* a user process asks for a keystroke */
            assert(client_proc == NULL);
            if (key_waiting) {
                //kprintf("keyb reply to client immediately\n");
                /* there is a keystroke waiting. Reply it to the user */
                *msg->key_buffer = key_buffer;
                reply(sender_proc);
                key_waiting = FALSE;
            } else {
                /* No keystroke pending. Block this client. */
                client_proc = sender_proc;
                client_msg = msg;
            }
        }
    }
}

void reset_usb_transfer_result() {
//    volatile unsigned int cpsr_flag;
//    SAVE_CPSR_DIS_IRQ(cpsr_flag); // Makesure isr do not change usb_transfer_result at the same time.
//    //usb_transfer_result = USB_TRANSFER_NULL;    
//    RESUME_CPSR(cpsr_flag);

}

void usbkeyb_process(PROCESS self, PARAM param) {
    BYTE key;
    Keyb_Message * msg;
    PROCESS sender_proc;


    keyboard_address = 0;
    
    key = keyboard_get_char_intr(keyboard_address_intr);    // Get old key
    //kprintf("Old key 0x%x\n", key);

    while (1) {    
        // Wait until key have bene pressed
//        reset_usb_transfer_result();
//        // Start Split
//        KeyboardProb(keyboard_address_intr);    
//        kprintf("[TOS][USBKEYBOARD] WHILE loop\n");
//        while(1);
//        while ((usb_transfer_result == USB_TRANSFER_NULL) || (usb_transfer_result == USB_TRANSFER_RESTART)) {
//            sleep(1);
//        }
//        if (usb_transfer_result == USB_TRANSFER_NAK) {
//            kprintf("Got NAK message\n");
//            sleep(10);
//            continue;
//        } else if (usb_transfer_result == USB_TRANSFER_COMPLETE) {
//            reset_usb_transfer_result();
//        } else {
//            kprintf("[KEYB] Got error transfer result %d\n", usb_transfer_result);
//            assert(0);
//        }
//        kprintf("[TOS] keyboard got message, WHILE LOOP\n");
//        while(1);
//        KeyboardGetKeyStats(keyboard_address_intr);
//        key = keyboard_get_char_intr(keyboard_address_intr);        
//        //kprintf("[TOS] Got Key 0x%x\n", key);

        /* Block usb keyboard until shell receive the key */
        if (key != 0) {
            msg = (Keyb_Message *) receive(&sender_proc);
            *msg->key_buffer = key;
            reply(sender_proc);
        }

    }
}

