#include <kernel.h>

extern unsigned char *_binary_font_font_bin_start;

/*
 * get_fore_colour()
 * -----------------
 *  Get 16bit foreground color
 *
 *  Return:
 *  Foreground color
 */
WORD get_fore_colour() {
    return foreground_color;
}

/*
 * set_fore_colour()
 * -----------------
 *  Set 16bit foreground color
 *
 * Parameters:
 * colour: Fore colour
 */
void set_fore_colour(short colour) {
    if (colour > 0x10000)   /* colour should lower than 0x10000 */
        return;
    else
        foreground_color = colour;
}

/*
 * get_back_colour()
 * -----------------
 *  Get 16bit background colour
 *
 *  Return:
 *  16bit background colour
 */
WORD get_back_colour() {
    return background_color;
}

/*
 * set_back_colour()
 * -----------------
 *  Set 16bit background colour
 *
 * Parameters:
 *  colour: Back ground colour
 */
void set_back_colour(short colour) {
    if (colour > 0x10000)   /* colour should lower than 0x10000 */
        return;
    else
        background_color = colour;
}

/*
 * set_graphics_address()
 * ----------------------
 *  Store the framebuffer information into global variable
 *
 * Parameters:
 *  address: framebuffer struct Address
 */
void set_graphics_address(FrameBufferInfo *address) {
    graphicsAddress = address;
}

/*
 * set_pixel()
 * -----------
 * Draw a pixel at row y, column x.
 * This function only work for high color(16-bit)
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 */
void set_pixel(int x, int y) {
    int height, width;
    short *gpu_pointer;

    height = graphicsAddress->p_height;     /* Get Height */
    height--;
    if (y > height) goto ERROR;

    width = graphicsAddress->p_width;       /* Get Width */
    width--;
    if (x > width) goto ERROR;

    /* Compute the address of the pixel to write */
    gpu_pointer = (short *) graphicsAddress->gpu_pointer;
    width++;
    *(gpu_pointer + (x + y * width)) = foreground_color;  /* Calculate pixel position in memory */
    ERROR:
    return;
}

/*
 * clear_pixel()
 * -------------
 * Clear a pixel at row y, column x.
 * This function only work for high color(16-bit)
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 */
void clear_pixel(int x, int y) {
    int height, width;
    short *gpu_pointer;

    height = graphicsAddress->p_height;     /* Get Height */
    height--;
    if (y > height) goto ERROR;

    width = graphicsAddress->p_width;       /* Get Width */
    width--;
    if (x > width) goto ERROR;

    /* Compute the address of the pixel to write */
    gpu_pointer = (short *) graphicsAddress->gpu_pointer;
    width++;
    *(gpu_pointer + (x + y * width)) = background_color;  /* Calculate pixel position in memory */
    ERROR:
    return;
}

/*
 * get_pixel_16bit()
 * -----------------
 * Return pixel value(16bit) from location(x,y) on screen
 *
 * Parameters:
 * x: x coordinate
 * y: y coordinate
 *
 * Return:
 * 16 bit colour
*/
WORD get_pixel_16bit(int x, int y) {
    WORD *gpu_pointer;

    if (y > (graphicsAddress->p_height - 1)) goto ERROR;
    if (x > (graphicsAddress->p_width - 1)) goto ERROR;

    gpu_pointer = (WORD *) graphicsAddress->gpu_pointer;

    return *(gpu_pointer + (x + y * graphicsAddress->p_width));
    ERROR:
    return 0;
}

/* copy_pixel_16bit()
 * ------------------
 * Copy pixel from location(src_x, src_y) to location(des_x, des_y)
 *
 * Parameters:
 * src_x: source x coordinate
 * src_y: source y coordinate
 * des_x: destination x coordinate
 * des_y: destination y coordinate
 */
void copy_pixel_16bit(int src_x, int src_y, int des_x, int des_y) {
    if (src_y > (graphicsAddress->p_height - 1)) goto ERROR;
    if (des_y > (graphicsAddress->p_height - 1)) goto ERROR;
    if (src_x > (graphicsAddress->p_width - 1)) goto ERROR;
    if (des_x > (graphicsAddress->p_width - 1)) goto ERROR;
    WORD *gpu_pointer;

    gpu_pointer = (WORD *) graphicsAddress->gpu_pointer;

    *(gpu_pointer + (des_x + des_y * graphicsAddress->p_width)) = *(gpu_pointer +
                                                                    (src_x + src_y * graphicsAddress->p_width));
    return;
    ERROR:
    return;
}

/*
 * draw_line()
 * -----------
 * Draw a line by using Bresenham's Algorithm
 *
 * Parameters:
 * x0: source x coordinate
 * y0: source y coordinate
 * x1: destination x coordinate
 * y1: destination y coordinate
 */
void draw_line(int x0, int y0, int x1, int y1, unsigned short color) {
    volatile unsigned int cpsr_flag;
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    
    int deltax, deltay, stepy, stepx, error, error2, fore_color;

    fore_color = get_fore_colour();
    set_fore_colour(color);
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

    for (; ;) {
        set_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        error2 = error;
        if (error2 > (0 - deltax)) {
            error -= deltay;
            x0 += stepx;
        }
        if (error2 < deltay) {
            error += deltax;
            y0 += stepy;
        }
    }
    set_fore_colour(fore_color);
    RESUME_CPSR(cpsr_flag);
}

/*
 * draw_character()
 * ----------------
 *  Draw character on screen
 *
 *  Parameters:
 *  character:      character to draw
 *  x:              character x coordinate
 *  y:              character y coordinate
 */
void draw_character(char character, int x, int y) {
    /* Get the font binary address by run objdump -t build/font.o */
    unsigned char *font_addr = (unsigned char *) &_binary_font_font_bin_start;
    int row;
    unsigned int bits, bit;

    if (character > 127) {
        return;
    }

    /* Each character have 16 bytes in font.bin */
    font_addr += CHARACTER_SIZE * character;

    for (row = 0; row < CHARACTER_HEIGHT; row += 1) {
        bits = (unsigned int) peek_b((MEM_ADDR)(font_addr + row));
        bit = CHARACTER_WIDTH;
        do {
            bits = bits << 1;
            bit -= 1;
            if ((bits & 0x100) != 0) {
                set_pixel(x + bit, y);
            } else {
                clear_pixel(x + bit, y);
            }
        } while (bit != 0);
        y += 1;
    }
}
