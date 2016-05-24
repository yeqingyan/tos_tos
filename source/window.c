#include <kernel.h>

// TODO add more comments
/*
 * Write a character into framebuffer
*/
void poke_screen(int x, int y, char ch) {
    draw_character(ch, x * CHARACTER_WIDTH, y * CHARACTER_HEIGHT);
}

/*
 * Copy character from (src_x,src_y) to (des_x, des_y)
*/
void copy_character(int src_x, int src_y, int des_x, int des_y) {
    int row, col;
    int src_pixel_x, src_pixel_y, des_pixel_x, des_pixel_y;

    src_pixel_x = src_x * CHARACTER_WIDTH;
    src_pixel_y = src_y * CHARACTER_HEIGHT;
    des_pixel_x = des_x * CHARACTER_WIDTH;
    des_pixel_y = des_y * CHARACTER_HEIGHT;

    for (row = 0; row < CHARACTER_HEIGHT; row += 1) {
        for (col = 0; col < CHARACTER_WIDTH; col += 1) {
            copy_pixel_16bit(src_pixel_x, src_pixel_y, des_pixel_x, des_pixel_y);
            src_pixel_x += 1;
            des_pixel_x += 1;
        }
        src_pixel_y += 1;
        des_pixel_y += 1;
        src_pixel_x = src_x * CHARACTER_WIDTH;
        des_pixel_x = des_x * CHARACTER_WIDTH;
    }
}

void scroll_window(WINDOW *wnd) {
    int x, y;
    int wx, wy;
    volatile unsigned int cpsr_flag;
    //volatile int flag;

    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    // Scroll up one line
    for (y = 0; y < wnd->height - 1; y++) {
        wy = wnd->y + y;
        for (x = 0; x < wnd->width; x++) {
            wx = wnd->x + x;
            copy_character(wx, wy + 1, wx, wy);
        }
    }
    wy = wnd->y + wnd->height - 1;
    for (x = 0; x < wnd->width; x++) {
        wx = wnd->x + x;
        poke_screen(wx, wy, ' ');
    }
    wnd->cursor_x = 0;
    wnd->cursor_y = wnd->height - 1;
    RESUME_CPSR(cpsr_flag);
}


void move_cursor(WINDOW *wnd, int x, int y) {
    assert(x < wnd->width && y < wnd->height);
    wnd->cursor_x = x;
    wnd->cursor_y = y;
}


void remove_cursor(WINDOW *wnd) {
    poke_screen(wnd->x + wnd->cursor_x,
                wnd->y + wnd->cursor_y, ' ');
}


void show_cursor(WINDOW *wnd) {
    poke_screen(wnd->x + wnd->cursor_x,
                wnd->y + wnd->cursor_y,
                wnd->cursor_char);
}


void clear_window(WINDOW *wnd) {
    int x, y;
    int wx, wy;

    volatile int cpsr_flag;

    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    wnd->cursor_x = 0;
    wnd->cursor_y = 0;
    for (y = 0; y < wnd->height; y++) {
        wy = wnd->y + y;
        for (x = 0; x < wnd->width; x++) {
            wx = wnd->x + x;
            poke_screen(wx, wy, ' ');
        }
    }
    show_cursor(wnd);
    RESUME_CPSR(cpsr_flag);
}


void output_char(WINDOW *wnd, unsigned char c) {
    //volatile int flag;
    remove_cursor(wnd);
    switch (c) {
        case '\n':
        case 13:
            wnd->cursor_x = 0;
            wnd->cursor_y++;
            break;
        case '\b':
            if (wnd->cursor_x != 0) {
                wnd->cursor_x--;
            } else {
                if (wnd->cursor_y != 0) {
                    wnd->cursor_x = wnd->width - 1;
                    wnd->cursor_y--;
                }
            }
            break;
        default:
            poke_screen(wnd->x + wnd->cursor_x,
                        wnd->y + wnd->cursor_y,
                        (short unsigned int) c);
            wnd->cursor_x++;
            if (wnd->cursor_x == wnd->width) {
                wnd->cursor_x = 0;
                wnd->cursor_y++;
            }
            break;
    }
    if (wnd->cursor_y == wnd->height)
        scroll_window(wnd);
    show_cursor(wnd);
}

void output_string(WINDOW *wnd, const char *str) {
    volatile int cpsr_flag;
    SAVE_CPSR_DIS_IRQ(cpsr_flag);

    while (*str != '\0')
        output_char(wnd, *str++);
    RESUME_CPSR(cpsr_flag);
}

static WINDOW kernel_window_def = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, ' '};
WINDOW *kernel_window = &kernel_window_def;

void kprintf(const char *fmt, ...) {
    va_list argp;
    char buf[160];

    va_start(argp, fmt);
    vs_printf(buf, fmt, argp);
    output_string(kernel_window, buf);
    va_end(argp);
}

void wprintf(WINDOW *wnd, const char *fmt, ...) {
    va_list argp;
    char buf[160];

    va_start(argp, fmt);
    vs_printf(buf, fmt, argp);
    output_string(wnd, buf);
    va_end(argp);
    
}

void debugprintf(WINDOW *wnd, const char *fmt, ...) {
    va_list argp;
    char buf[160];

    va_start(argp, fmt);
    vs_printf(buf, fmt, argp);
    debug_output_string(wnd, buf);
    va_end(argp);
}

void debug_output_string(WINDOW *wnd, const char *str) {
    while (*str != '\0')
        output_char(wnd, *str++);
}
