#include <kernel.h>

static WINDOW error_window = {0, 41, 80, 1, 0, 0, ' '};

void panic_mode(const char *msg, const char *file, int line) {
    DISABLE_INTR();
    clear_window(&error_window);
    wprintf(&error_window, "PANIC: '%s' at line %d of %s",
            msg, line, file);
    while (1);
}