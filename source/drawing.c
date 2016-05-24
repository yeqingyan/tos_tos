#include <kernel.h>

// Ref. Raspberry Pi baremetal screen.
// http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen02.html
// http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen03.html

// Pointer to font binary, generate from binary file in makefile 
extern unsigned char *_binary_font_font_bin_start;

/**
 * Get 16bit foreground color
 * 
 * @return  foreground color
 */
WORD get_fore_colour() {
    return foreground_color;
}

/**
 * Set 16bit foreground color
 * 
 * @param   foreground colour
 */
void set_fore_colour(short colour) {
    if (colour > 0x10000) /* colour should lower than 0x10000 */
        return;
    else
        foreground_color = colour;
}

/**
 * Get 16bit background colour
 * 
 * @return  16bit background colour
 */
WORD get_back_colour() {
    return background_color;
}

/**
 * Set 16bit background color
 * @param   Background color
 */
void set_back_colour(short colour) {
    if (colour > 0x10000) /* colour should lower than 0x10000 */
        return;
    else
        background_color = colour;
}

/**
 * Store the framebuffer information into global variable
 * 
 * @param address   framebuffer structure address
 */
void set_graphics_address(FrameBufferInfo *address) {
    graphicsAddress = address;
}

/**
 * Draw a pixel at row y, column x. Only for high color(16bit)
 * 
 * @param x     x coordinate
 * @param y     y coordinate
 */
void set_pixel(int x, int y) {
    short *gpu_pointer;
    int height, width;
    // Compute the address of the pixel to write 
    gpu_pointer = (short *) graphicsAddress->gpu_pointer;

    height = graphicsAddress->p_height; // Get Height 
    height--;
    if (y > height) goto ERROR;

    width = graphicsAddress->p_width; // Get Width 
    width--;
    if (x > width) goto ERROR;

    width++;
    // Calculate pixel position in memory 
    *(gpu_pointer + (x + y * width)) = foreground_color;
ERROR:
    return;
}

/**
 * Clear a pixel at row y, column x.
 * This function only work for high color(16-bit)
 * 
 * @param x     x coordinate
 * @param y     y coordinate
 */
void clear_pixel(int x, int y) {
    int height, width;
    short *gpu_pointer;

    height = graphicsAddress->p_height; /* Get Height */
    height--;
    if (y > height) goto ERROR;

    width = graphicsAddress->p_width; /* Get Width */
    width--;
    if (x > width) goto ERROR;

    /* Compute the address of the pixel to write */
    gpu_pointer = (short *) graphicsAddress->gpu_pointer;
    width++;
    *(gpu_pointer + (x + y * width)) = background_color; /* Calculate pixel position in memory */
ERROR:
    return;
}

/**
 * Return pixel value(16bit) from location(x,y) on screen
 * 
 * @param x     x coordinate
 * @param y     y coordinate
 * @return      16 bit color
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

/**
 * Copy pixel from location(src_x, src_y) to location(des_x, des_y)
 * 
 * @param src_x     source x coordinate
 * @param src_y     source y coordinate
 * @param des_x     destination x coordinate
 * @param des_y     destination y coordinate
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

/**
 * Draw a line by using Bresenham's Algorithm
 * 
 * @param x0        source x coordinate
 * @param y0        source y coordinate
 * @param x1        destination x coordinate
 * @param y1        destination y coordinate
 * @param color     line color
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

    while (x0 != x1 || y0 != y1) {
        set_pixel(x0, y0);
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

/**
 * Draw character on screen
 * 
 * @param character     character to draw
 * @param x             character x coordinate
 * @param y             character y coordinate
 */
void draw_character(char character, int x, int y) {
    /* Get the font binary address by run objdump -t build/font.o */
    unsigned char *font_addr = (unsigned char *) &_binary_font_font_bin_start;
    int row;
    unsigned int bits, bit;

    if (character > 127) {
        return;
    }

    // Each character cost 16 bytes in font.bin 
    font_addr += CHARACTER_SIZE * character;

    for (row = 0; row < CHARACTER_HEIGHT; row += 1) {
        bits = (unsigned int) peek_b((MEM_ADDR) (font_addr + row));
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
