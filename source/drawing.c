#include <kernel.h>

extern unsigned char* _binary_font_font_bin_start;
/*
 * SetForeColour
 * -------------
 * Parameters:
 * r0: Fore colour
 */
void SetForeColour(short colour) {
        if (colour > 0x10000)   /* colour should lower than 0x10000 */
                return;
        else
                foreColour = colour;
}

/*
 * SetGraphicsAddress
 * ------------------
 * Parameters:
 * r0: Graphic Address
 */
void SetGraphicsAddress(FrameBufferInfo* address) {
        graphicsAddress = address;
}

/*
 * SetPixel
 * --------
 * This function only work for high color(16-bit)
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 */
void SetPixel(int x, int y) {
        int height, width;
        short* gpu_pointer;

        height = graphicsAddress->p_height;     /* Get Height */
        height--;
        if (y > height) goto ERROR;

        width = graphicsAddress->p_width;       /* Get Width */
        width--;
        if (x > width) goto ERROR;

        /* Compute the address of the pixel to write */
        gpu_pointer = (short *)graphicsAddress->gpu_pointer;
        width++;
        *(gpu_pointer + (x + y * width)) = foreColour;  /* Calculate pixel position in memory */
ERROR:
        return ;
}

/*
 * DrawLine
 * --------
 * Draw a line by using Bresenham's Algorithm
 *
 * Parameters:
 * x0
 * y0
 * x1
 * y1
 */
void DrawLine(int x0, int y0, int x1, int y1) {
        int deltax, deltay, stepy, stepx, error, error2;

        if (x1 > x0) {
                deltax = x1 - x0;
                stepx = 1;
        } else {
                deltax = x0 - x1;
                stepx = -1;
        }

        if (y1 > y0) {
                deltay = y1 - y0;
                stepy = 1;
        } else {
                deltay = y0 - y1;
                stepy = -1;
        }

        if (deltax > deltay) {
                error = deltax;
        } else {
                error = -deltay;
        }
        error = error / 2;

        for(;;) {
                SetPixel(x0, y0);
                if (x0 == x1 && y0 == y1) break;
                error2 = error;
                if (error2 > (0-deltax) ) {
                        error -= deltay;
                        x0 += stepx;
                }
                if (error2 < deltay) {
                        error += deltax;
                        y0 += stepy;
                }
        }
}

/* Draw character on screen */
void DrawCharacter(char character, int x, int y) {
    /* Get the font binary address by run objdump -t build/font.o */
    unsigned char* font_addr = (unsigned char *)&_binary_font_font_bin_start;
    int row;
    unsigned int bits, bit;
    
    if (character > 127) {
        return;
    }
    
    /* Each character have 16 bytes in font.bin */
    font_addr += 16 * character;
    
    for(row = 0; row<=15; row += 1) {
        bits = (unsigned int)peek_b((MEM_ADDR)(font_addr+row));
        bit = 8;
        do {
            bits = bits << 1;
            bit -= 1;
            if ((bits & 0x100) != 0) {
                // Draw pixel
                SetPixel(x+bit, y);
            } 
        } while (bit != 0);
        y += 1;  
    }          
}