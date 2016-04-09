#include <kernel.h>

extern unsigned char* _binary_font_font_bin_start;


WORD get_fore_colour() {
    return foreground_color;
}

/*
 * set_fore_colour
 * -------------
 * Parameters:
 * r0: Fore colour
 */
void set_fore_colour(short colour) {
    if (colour > 0x10000)   /* colour should lower than 0x10000 */
        return;
    else
        foreground_color = colour;
}

WORD get_back_colour() {
    return background_color;
}

/*
 * set_back_colour
 * -------------
 * Parameters:
 * r0: Back colour
 */
void set_back_colour(short colour) {
    if (colour > 0x10000)   /* colour should lower than 0x10000 */
        return;
    else
        background_color = colour;
}

/*
 * set_graphics_address
 * ------------------
 * Parameters:
 * r0: Graphic Address
 */
void set_graphics_address(FrameBufferInfo* address) {
        graphicsAddress = address;
}

/*
 * set_pixel
 * --------
 * This function only work for high color(16-bit)
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 */
void set_pixel(int x, int y) {
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
        *(gpu_pointer + (x + y * width)) = foreground_color;  /* Calculate pixel position in memory */
ERROR:
        return ;
}

/*
 * clear_pixel
 * --------
 * This function only work for high color(16-bit)
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 */
void clear_pixel(int x, int y) {
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
        *(gpu_pointer + (x + y * width)) = background_color;  /* Calculate pixel position in memory */
ERROR:
        return ;
}

/*
 * get_pixel_16bit
 * ---------------
 * Return pixel value(16bit) from location(x,y) on screen
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate 
*/
WORD get_pixel_16bit(int x, int y){
        WORD* gpu_pointer;    
    
        if (y > (graphicsAddress->p_height-1)) goto ERROR;
        if (x > (graphicsAddress->p_width-1)) goto ERROR; 
        
        gpu_pointer = (WORD *)graphicsAddress->gpu_pointer;
        
        return *(gpu_pointer + (x + y * graphicsAddress->p_width));
ERROR:
        return 0;               
}

/* copy_pixel_16bit */
void copy_pixel_16bit(int src_x, int src_y, int des_x, int des_y) {
        if (src_y > (graphicsAddress->p_height-1)) goto ERROR;
        if (des_y > (graphicsAddress->p_height-1)) goto ERROR;
        if (src_x > (graphicsAddress->p_width-1)) goto ERROR;
        if (des_x > (graphicsAddress->p_width-1)) goto ERROR;
        WORD* gpu_pointer;
        
        gpu_pointer = (WORD *)graphicsAddress->gpu_pointer;
        
        *(gpu_pointer + (des_x + des_y * graphicsAddress->p_width)) = *(gpu_pointer + (src_x + src_y * graphicsAddress->p_width));
        return;
ERROR:
        return;             
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
                set_pixel(x0, y0);
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
void draw_character(char character, int x, int y) {
    /* Get the font binary address by run objdump -t build/font.o */
    unsigned char* font_addr = (unsigned char *)&_binary_font_font_bin_start;
    int row;
    unsigned int bits, bit;
    
    if (character > 127) {
        return;
    }
    
    /* Each character have 16 bytes in font.bin */
    font_addr += CHARACTER_SIZE * character;
    
    for(row = 0; row < CHARACTER_HEIGHT; row += 1) {
        bits = (unsigned int)peek_b((MEM_ADDR)(font_addr+row));
        bit = CHARACTER_WIDTH;
        do {
            bits = bits << 1;
            bit -= 1;
            if ((bits & 0x100) != 0) {
                // Draw pixel
                set_pixel(x+bit, y);
            } else {
                clear_pixel(x+bit, y);
            }
        } while (bit != 0);
        y += 1;  
    }          
}

//void DrawString()