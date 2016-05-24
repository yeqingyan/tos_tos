#include <kernel.h>

#define SHELL_CMD_MAX_LEN 256 // Maximum characters per line is 256 
#define KEY_BACKSPACE 8
#define GHOST_NUMBER 4

WINDOW shell_wnd = {0, 26, 80, 15, 0, 0, '_'};
char shell_line_buf[SHELL_CMD_MAX_LEN + 1];
int line_index;

WINDOW *wnd_ptr = &shell_wnd;

int str_cmp(char *, char *);

void shell_buf_reset();

char *read_command();

void run_command(char *);

void shell_process(PROCESS, PARAM);

/**
 * Compare two strings.
 * 
 * @param first     first string
 * @param second    second string
 * @return          0 if string are same, -1 if they are not.
 */
int str_cmp(char *first, char *second) {
    while (*first == *second) {
        if (*first == '\0' || *second == '\0')
            break;
        first++;
        second++;
    }

    if (*first == '\0' && *second == '\0')
        return 0;
    else
        return -1;
}

/**
 * Reset the shell command buffer and print the promote
 */
void shell_buf_reset() {
    line_index = 0;
    wprintf(wnd_ptr, "tos:");
}

/**
 * return the input command
 * 
 * @return command string
 */
char *read_command() {
    // add null character to the end of the string
    shell_line_buf[line_index] = '\0';
    return shell_line_buf;
}

/**
 * For gdb debug only. The function do not need to disable interrupt.
 */
void ps() {
    debug_print_all_process(wnd_ptr);
}

/**
 * Run TOS commands
 * 
 * @param cmd       command string
 */
void run_command(char *cmd) {
    if (str_cmp(cmd, "help") == 0) {
        wprintf(wnd_ptr, "clear                 - Clear the window\n");
        wprintf(wnd_ptr, "ps                    - Print process table\n");
        wprintf(wnd_ptr, "start_lines           - Draw lines\n");
        wprintf(wnd_ptr, "stop_lines            - Stop draw lines\n");
        wprintf(wnd_ptr, "train                 - Train related\n");
        wprintf(wnd_ptr, "switch                - Change switch position\n");
        wprintf(wnd_ptr, "pacman                - Start Pacman game\n");
        wprintf(wnd_ptr, "train_test            - Test serial\n");
        wprintf(wnd_ptr, "start_led             - Start LED Program\n");
        wprintf(wnd_ptr, "inc_sleep/dec_sleep   - Inc/Dec sleep time \n");
        wprintf(wnd_ptr, "inc_check/dec_check   - Inc/Dec check time\n");
    } else if (str_cmp(cmd, "ps") == 0) {
        ps();
    } else if (str_cmp(cmd, "clear") == 0) {
        // clear window and reset the shell window pointer 
        clear_window(kernel_window); 
        clear_window(wnd_ptr);
        shell_wnd.x = 0;
        shell_wnd.y = 26;
        shell_wnd.cursor_x = 0;
        shell_wnd.cursor_y = 0;
    } else if (str_cmp(cmd, "start_lines") == 0) {
        lines_start();
    } else if (str_cmp(cmd, "stop_lines") == 0) {
        lines_stop();
    } else if (str_cmp(cmd, "start_led") == 0) {
        init_led();
    } else if (str_cmp(cmd, "train_test") == 0) {
        train_test();
    } else if (str_cmp(cmd, "train") == 0) {
        init_train(wnd_ptr);
    } else if (str_cmp(cmd, "inc_sleep") == 0) {
        inc_sleep_time(wnd_ptr);
    } else if (str_cmp(cmd, "dec_sleep") == 0) {
        dec_sleep_time(wnd_ptr);
    } else if (str_cmp(cmd, "inc_check") == 0) {
        inc_check_time(wnd_ptr);
    } else if (str_cmp(cmd, "dec_check") == 0) {
        dec_check_time(wnd_ptr);
    } else if (str_cmp(cmd, "pacman") == 0) {
        clear_window(kernel_window);
        init_pacman(kernel_window, GHOST_NUMBER);
    } else {
        wprintf(wnd_ptr, "Invalid Command '%s'!\n", cmd);
    }
}

/**
 * Shell process
 * 
 * @param self
 * @param param
 */
void shell_process(PROCESS self, PARAM param) {
    BYTE ch;
    char *command;
    Keyb_Message msg;

    while (1) {
        msg.key_buffer = &ch;
        send(keyb_port, &msg);
//        kprintf("[TOS] shell: got key %x\n", ch);
        if (ch == '\n') {
            wprintf(wnd_ptr, "\n");
            command = read_command();   // read command 
            run_command(command);       // execute command
            shell_buf_reset();          // reset shell buffer
        } else if (ch == KEY_BACKSPACE) {
            if (line_index != 0) {
                // move shell buffer pointer back if user press backspace 
                line_index--;
                wprintf(wnd_ptr, "%c", '\b');
            } else {
                continue;
            }
        } else {
            wprintf(wnd_ptr, "%c", ch);
            shell_line_buf[line_index] = ch;    // Record user input 
            line_index++;
            // Prevent user input more than 256 characters 
            if (line_index > SHELL_CMD_MAX_LEN - 1) { 
                line_index = SHELL_CMD_MAX_LEN;
                wprintf(wnd_ptr, "\n");
                wprintf(wnd_ptr, "Input exceed the max length %d.", SHELL_CMD_MAX_LEN);
                line_index = 0;
            }
        }
    }
}

/**
 * Init shell process
 */
void init_shell() {
    int i;
    for (i = 0; i <= SHELL_CMD_MAX_LEN; i++) {
        shell_line_buf[i] = '\0';
    }

    clear_window(kernel_window);
    wprintf(wnd_ptr, "Welcome to tos shell 1.0!\nType \"help\" for more information\n");
    shell_buf_reset();
    create_process(shell_process, 5, 0, "Shell Process");
}
