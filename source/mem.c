#include <kernel.h>

/*
 * poke_b()
 * --------
 *  store a byte
 *
 *  Paramters:
 *  addr: memory address
 *  value: value to store
 */
void poke_b(MEM_ADDR addr, BYTE value) {
    unsigned char *ptr = (unsigned char *) addr;
    *ptr = value;
}

/*
 * poke_w()
 * --------
 *  store a word
 *
 *  Paramters:
 *  addr: memory address
 *  value: value to store
 */
void poke_w(MEM_ADDR addr, WORD value) {
    unsigned short *ptr = (unsigned short *) addr;
    *ptr = value;
}

/*
 * poke_l()
 * --------
 *  store a long value
 *
 *  Paramters:
 *  addr: memory address
 *  value: value to store
 */
void poke_l(MEM_ADDR addr, LONG value) {
    unsigned *ptr = (unsigned *) addr;
    *ptr = value;
}

/*
 * peek_b()
 * --------
 *  Get a byte from an address
 *
 *  Parameters:
 *  addr: memory address
 */
BYTE peek_b(MEM_ADDR addr) {
    unsigned char *ptr = (unsigned char *) addr;
    return *ptr;
}

/*
 * peek_w()
 * --------
 *  Get a word from an address
 *
 *  Parameters:
 *  addr: memory address
 */
WORD peek_w(MEM_ADDR addr) {
    unsigned short *ptr = (unsigned short *) addr;
    return *ptr;
}

/*
 * peek_l()
 * --------
 *  Get a long from an address
 *
 *  Parameters:
 *  addr: memory address
 */
LONG peek_l(MEM_ADDR addr) {
    unsigned *ptr = (unsigned *) addr;
    return *ptr;
}
