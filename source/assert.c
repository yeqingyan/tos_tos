#include <kernel.h>

static WINDOW error_window = {0, 41, 80, 1, 0, 0, ' '};

void panic_mode(const char *msg, const char *file, int line) {
    volatile unsigned int cpsr_flag;
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    clear_window(&error_window);
    wprintf(&error_window, "PANIC: '%s' at line %d of %s",
            msg, line, file);
    while (1);
}