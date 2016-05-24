#include <kernel.h>

// Ref. Raspberry Pi baremetal screen.
// http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/screen01.html

FrameBufferInfo frameBufferInfo;
/**
 * Set up frame buffer.
 * 
 * @param width         Frame buffer width(less than 4096)
 * @param height        Frame buffer height(less than 4096)
 * @param bit_depth     less than 32 bits
 * @return              Pointer to frame buffer info. Return 0 if failed.
 */
FrameBufferInfo *init_framebuffer(int width, int height, int bit_depth) {
    if (width > 4096) goto ERROR;
    if (height > 4096) goto ERROR;
    if (bit_depth > 32) goto ERROR;
    int response;

    FrameBufferInfo *frameBuffer = &frameBufferInfo;
    frameBuffer->p_width = width;
    frameBuffer->p_height = height;
    frameBuffer->v_width = width;
    frameBuffer->v_height = height;
    frameBuffer->gpu_pitch = 0;
    frameBuffer->bit_depth = bit_depth;
    frameBuffer->x = 0;
    frameBuffer->y = 0;
    frameBuffer->gpu_pointer = 0;
    frameBuffer->gpu_size = 0;

    int frameBufferAddr = (int) frameBuffer;
    frameBufferAddr += 0x40000000;
    // Write frame buffer to channel 1
    mailbox_write(frameBufferAddr, 1);      
    response = mailbox_read(1);

    if (response != 0)
        return 0;

    return frameBuffer;
ERROR:
    return 0;
}

/**
 * Setup frame buffer, background color, foreground color.
 */
void init_video(void) {
    FrameBufferInfo *p_framebuffer;
    p_framebuffer = init_framebuffer(1024, 768, 16);
    set_graphics_address(p_framebuffer);
    set_fore_colour(COLOR_WHITE);
    set_back_colour(COLOR_BLACK);
}