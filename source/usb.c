#include <kernel.h>

unsigned int keyboard_address;
short old_keys[6];
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

BYTE keys_shift[104] = { 0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
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


void LogPrint(const char* message, int messageLength) {
    while (messageLength != 0) {
        output_char(kernel_window, *message++);
        messageLength --;
    }
}
/*
 * Init USB
 */
void init_usb(){
    int ret, ascii_char;
    
    keyboard_address = 0;
    
    //1. Call UsbInitialise
    ret = UsbInitialise();
    if (ret != 0) {
        kprintf("USB initialise meet error! \n", ret);
    }
    while (1) {
        keyboard_update();
        ascii_char = keyboard_get_char();
        if( ascii_char != 0) {
            output_char(kernel_window, ascii_char);
        }
    }
}    
    
/* 
    Keyboard Update 
    Based on http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/input01.html
    
    1.  Retrieve a stored keyboard address (initially 0).
    2.  If this is not 0, go to 9.
    3.  Call UsbCheckForChange to detect new keyboards.
    4.  Call KeyboardCount to detect how many keyboards are present.
    5.  If this is 0 store the address as 0 and return; we can't do anything with no keyboard.
    6.  Call KeyboardGetAddress with parameter 0 to get the first keyboard's address.
    7.  Store this address.
    8.  If this is 0, return; there is some problem.
    9.  Call KeyboardGetKeyDown 6 times to get each key currently down and store them
    10. Call KeyboardPoll
    11. If the result is non-zero go to 3. There is some problem (such as disconnected keyboard).
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
                return ;                
            }
            
            keyboard_address = KeyboardGetAddress(0);
            if (keyboard_address == 0) {
                kprintf("Error! Keyboard address is %d\n", keyboard_address);
                return ;
            }
        }
        // Call KeyboardGetKeyDown 6 times to get each key currently down and store them
        for(i=0; i<6; i+=1) {
            old_keys[i] = KeyboardGetKeyDown(keyboard_address, i);
        }
    } while(KeyboardPoll(keyboard_address) != 0);
    return;
}

/*
    1. Check if KeyboardAddress is 0. If so, return 0.
    2. Call KeyboardGetKeyDown up to 6 times. Each time:
        2.1 If key is 0, exit loop.
        2.2 Call KeyWasDown. If it was, go to the next key.
        2.3 If the scan code is more than 103, go to the next key.
        2.4 Call KeyboardGetModifiers
        2.5 If shift is held, load the address of KeysShift. Otherwise load KeysNormal.
        2.6 Read the ASCII value from the table.
        2.7 If it is 0, go to the next key otherwise return this ASCII code and exit.
    3. Return 0.
*/
int keyboard_get_char() {
    int i, index, key, ascii_char;
    BYTE modifiers; 
    
    index=0;
    if (keyboard_address == 0) {
        return 0;
    }
    for (i=0; i<6; i+=1) {
        key = KeyboardGetKeyDown(keyboard_address, index);
        if (key == 0) {
            return 0;
        }
        if ((key > 103) || (key_was_down(key) == TRUE)){
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

/* Check if the key already hold down */
BOOL key_was_down(int key) {
    int i;
    for (i=0; i<6; i+=1) {
        if (old_keys[i] == key) {
            return TRUE;
        } 
    }
    return FALSE;
}
