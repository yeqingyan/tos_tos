#include <kernel.h>

void charToBin(char c, int* buf, const int length) {
        int i, mask;
        mask = 1;
        for (i=0; i<length; i++) {
                *(buf+i) = ((int)c>>i)&mask; 
        }
}
