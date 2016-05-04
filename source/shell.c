#include <kernel.h>

#define SHELL_CMD_MAX_LEN 256 /* Max 256 characters per line */
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


/* 
 * Function: str_cmp
 * -----------------
 *  Compare two strings.
 * 
 *  first:  first string
 *  second: second string
 *
 *  returns: 0 if string are same, -1 if they are not.
 *  
 *  Note: str_cmp should be in stdlib.c, put it here avoid changing kernel.h 
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

/* 
 * Function: shell_buf_reset
 * -------------------------
 *  Reset the shell command buffer and print the promote
 */
void shell_buf_reset() {
    line_index = 0;
    wprintf(wnd_ptr, "tos:");
}

/* 
 * Function: read_command
 * ----------------------
 *  return the input command
 *
 *  returns: command string
 */
char *read_command() {
    shell_line_buf[line_index] = '\0';    /* add null character to the end of the string */
    return shell_line_buf;
}

/* Function: run_command
 * ---------------------
 *  Run tos command
 *
 *  cmd: command string
 */
void run_command(char *cmd) {
    if (str_cmp(cmd, "help") == 0) {
        wprintf(wnd_ptr, "clear  - Clear the window\n");
        wprintf(wnd_ptr, "ps     - Print process table\n");
        wprintf(wnd_ptr, "start_lines - Draw lines\n");
        wprintf(wnd_ptr, "stop_lines  - Stop draw lines\n");
//        wprintf(wnd_ptr, "train  - Train related\n");
//        wprintf(wnd_ptr, "switch - Change switch position\n");
        wprintf(wnd_ptr, "pacman - Start Pacman game\n");
        wprintf(wnd_ptr, "start_led - Start LED Program\n");
//        wprintf(wnd_ptr, "inc_sleep/dec_sleep - Increase/Decrease sleep time with 10 ticks\n");
//        wprintf(wnd_ptr, "inc_check/dec_check - Increase/Decrease check time with 10 ticks\n");
    } else if (str_cmp(cmd, "ps") == 0) {
        print_all_processes(wnd_ptr);
    } else if (str_cmp(cmd, "clear") == 0) {
        clear_window(kernel_window); /* clear window and reset the shell window pointer */
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
//    } else if (str_cmp(cmd, "train") == 0) {
//        init_train(wnd_ptr);
//    } else if (str_cmp(cmd, "inc_sleep") == 0) {
//        inc_sleep_time(wnd_ptr);
//    } else if (str_cmp(cmd, "dec_sleep") == 0) {
//        dec_sleep_time(wnd_ptr);
//    } else if (str_cmp(cmd, "inc_check") == 0) {
//        inc_check_time(wnd_ptr);
//    } else if (str_cmp(cmd, "dec_check") == 0) {
//        dec_check_time(wnd_ptr);
    } else if (str_cmp(cmd, "pacman") == 0) {
        clear_window(kernel_window);
        init_pacman(kernel_window, GHOST_NUMBER);
    //} else if (str_cmp(cmd, "") == 0) {
    //    return;
    } else {
        wprintf(wnd_ptr, "Invalid Command!\n");
    }
}

void shell_process(PROCESS self, PARAM param) {
    BYTE ch;
    char *command;
    Keyb_Message msg;

    while (1) {
        msg.key_buffer = &ch;
        send(keyb_port, &msg);

        if (ch == '\n') {
            wprintf(wnd_ptr, "\n");
            command = read_command();           /* read command */
            run_command(command);               /* execute command */
            shell_buf_reset();                  /* reset shell buffer */
        } else if (ch == KEY_BACKSPACE) {
            if (line_index != 0) {
                line_index--;                    /* move shell buffer pointer back if user press backspace */
                wprintf(wnd_ptr, "%c", '\b');
            } else {
                continue;
            }
        } else {
            wprintf(wnd_ptr, "%c", ch);
            shell_line_buf[line_index] = ch;    /* Record user input */
            line_index++;
            if (line_index > SHELL_CMD_MAX_LEN - 1) {   /* Prevent user input more than 256 characters */
                line_index = SHELL_CMD_MAX_LEN;
                wprintf(wnd_ptr, "\n");
                wprintf(wnd_ptr, "Input exceed the max length %d.", SHELL_CMD_MAX_LEN);
                line_index = 0;
            }
        }
    }
}

void init_shell() {
    int i;
    for (i = 0; i <= SHELL_CMD_MAX_LEN; i++) {
        shell_line_buf[i] = '\0';
    }

    clear_window(kernel_window);
    wprintf(wnd_ptr, "Welcome to tos shell!\nType \"help\" for more information\n");
    shell_buf_reset();
    create_process(shell_process, 5, 0, "Shell Process");
}
