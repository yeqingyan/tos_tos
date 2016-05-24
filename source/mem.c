#include <kernel.h>

/**
 * store a byte
 * 
 * @param addr      memory address
 * @param value     value to store
 */
void poke_b(MEM_ADDR addr, BYTE value) {
    unsigned char *ptr = (unsigned char *) addr;
    *ptr = value;
}

/**
 * store a word(2 bytes)
 * 
 * @param addr      memory address
 * @param value     value to store
 */
void poke_w(MEM_ADDR addr, WORD value) {
    unsigned short *ptr = (unsigned short *) addr;
    *ptr = value;
}

/**
 * store a long value(4 bytes)
 * 
 * @param addr      memory address
 * @param value     value to store
 */
void poke_l(MEM_ADDR addr, LONG value) {
    unsigned *ptr = (unsigned *) addr;
    *ptr = value;
}

/**
 * Get a byte from an address
 * 
 * @param addr  memory address
 * @return      byte
 */
BYTE peek_b(MEM_ADDR addr) {
    unsigned char *ptr = (unsigned char *) addr;
    return *ptr;
}

/**
 * Get a byte from an address
 * 
 * @param addr  memory address
 * @return      word(2 bytes)
 */
WORD peek_w(MEM_ADDR addr) {
    unsigned short *ptr = (unsigned short *) addr;
    return *ptr;
}

/**
 * Get a byte from an address
 * 
 * @param addr  memory address
 * @return      long value(4 bytes)
 */
LONG peek_l(MEM_ADDR addr) {
    unsigned *ptr = (unsigned *) addr;
    return *ptr;
}
