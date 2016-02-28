#include <kernel.h>
/*
 * InitialiseFrameBuffer
 * ---------------------
 * Parameters
 * width
 * height
 * bit depth
 *
 * Return
 * Pointer to framebuffer info. Return 0 if failed.
 */
FrameBufferInfo frameBufferInfo;

FrameBufferInfo* InitialiseFrameBuffer(int width, int height, int bit_depth){
        if (width > 4096) goto ERROR;           /* Width should less than 4096 */
        if (height > 4096) goto ERROR;          /* Height should less than 4096 */
        if (bit_depth > 32) goto ERROR;         /* Bit depth should less than 32 bits */
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
        MailboxWrite(frameBufferAddr, 1);       /* Write framebuffer to channel 1 */
        response = MailboxRead(1);              /* Read response */

        if (response != 0)                      /* If response is not 0, return 0 indicate failure. */
                return 0;

        return frameBuffer;
ERROR:
        return 0;
}

void init_framebuffer(void) {
        FrameBufferInfo *p_framebuffer;
        p_framebuffer = InitialiseFrameBuffer(1024, 768, 16);
        SetGraphicsAddress(p_framebuffer);    
        SetForeColour(65535);
}