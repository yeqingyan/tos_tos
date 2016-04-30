#include <kernel.h>
#include <test.h>
//#include <lib.h>

// This two address are reversed in BCM2835, we use it to connect ttc
#define TTC_OUT 0x20200018
#define TTC_IN 0x20200024
#define TTC_Ready 0x20200030

int test_result;
unsigned check_sum;
char test_results[MAX_NUM_TESTS];
WINDOW report_window = {0, 23, 80, 2, 0, 0, ' '};
//extern WORD lib_default_color;

void test_reset() {
    volatile unsigned int cpsr_flag;
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    // TODO 
    //interrupts_initialized = FALSE;
    clear_window(kernel_window);

    test_result = 0;
    init_process();
    init_dispatcher();
    init_ipc();

    /*
     * Normally we would call the following init_*(), but the way some
     * test cases are written, this would disrupt them. So we manually
     * call them when we know it is OK.
     */
//    init_interrupts();
//    init_null_process();
//    init_timer();
//    init_com();

}

/*
 * Check if a process is on ready queue.
 */
BOOL is_on_ready_queue(PROCESS proc) {
    BOOL on = FALSE;
    PROCESS first_process, current_process;
    first_process = current_process = ready_queue[proc->priority];
    if (first_process != NULL) {
        if (current_process == proc) {
            on = TRUE;
        } else {
            do {
                current_process = current_process->next;
                if (current_process == proc) {
                    on = TRUE;
                    break;
                }

            } while (current_process != first_process);
        }
    }

    return on;
}

/*
 * Passes execution to boot process.
 * Precondition:
 *    1. pcb[0] is used for boot process.
 *    2. boot process must be on ready queue 
 */
void return_to_boot() {

    int i;
    for (i = 1; i < MAX_PROCS; i++) {
        if (pcb[i].used) if (is_on_ready_queue(&pcb[i]))
            remove_ready_queue(&pcb[i]);
    }

    resign();
}

/*
* Write test report to the screen
*/
void write_test_report(unsigned short color) {
    char ch = '-'; // Horizontal line
    int i;
    short fg_color = 0;
    volatile unsigned int cpsr_flag;
    
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    //lib_default_color = attr;
    fg_color = get_fore_colour();
    set_fore_colour(color);

    clear_window(&report_window);
    for (i = 0; i < 5; i++)
        wprintf(&report_window, "%c", ch);
    wprintf(&report_window, "TEST REPORT");
    for (i = 0; i < 80 - 16; i++)
        wprintf(&report_window, "%c", ch);

    set_fore_colour(fg_color);
}


void send_to_test_center(char *cmd) {
    while (*cmd != '\0') {
        /*
        * Wait for the ttc to accept the next byte
        */
        //while (!(recv_from_ttc() & (1 << 5))) ;
        while (recv_from_ttc() != (unsigned char) 0);
        send_to_ttc(*cmd);
        cmd++;
    }
}


/*
 * test_failed_impl is called when a test fails. It prints error messages and
 * then goes into an endless loop.
 */
void test_failed_impl(int code, char *file, int line) {
    write_test_report(COLOR_RED);
    wprintf(&report_window, "%s failed at line %d. Error code: %d",
            file, line, code);

    if (test_results[0] == '\0')
        // the TOS Test Center isn't running. Stop right here
        while (42);

    send_to_test_center("RESULT=");
    char buf[20];
    char *b = buf;
    b = printnum(b, code, 10, 0, 0, 0, 0, 0);
    *b++ = ',';
    b = printnum(b, line, 10, 0, 0, 0, 0, 0);
    *b++ = ',';
    *b = '\0';
    send_to_test_center(buf);
    send_to_test_center(test_results);
    while (42);
}

/*
 *  This function is used to check screen output.
 *  It checks if the characters on the screen matches a given array of strings.
 *  contents  
 *      -- an array of strings that corresponds to consecutive lines
 *         of the screen output. Each string should correspond to 
 *         one line. No '\n' is needed at the end of a string. 
 */
/*
void check_screen_output(char **contents) {
    int i = 0;
    while (contents[i] != NULL) {
        unsigned position = 0xb8000 + 80 * 2 * i;

        int j;
        for (j = 0; contents[i][j] != '\0'; j++) {
            char ch = contents[i][j];
            if (peek_b(position) != ch) {
                // We also accept 0x00 for ' '
                if (!(ch == ' ' && peek_b(position) == 0x00)) {
                    test_result = 2;
                    return;
                }
            }

            // Check that the character attribute is bright white (0x0f)
            // For a blank ' ' we don't check the attribute since it
            // doesn't matter
            if (peek_b(position + 1) != 0x0f && ch != ' ') {
                test_result = 3;
                return;
            }
            position += 2;
        }
        i++;
    }
}
*/

/*
 *  string_compare() returns 1 if the two strings are the same; 0 otherwise.
 */
int string_compare(char *str1, char *str2) {
    int equal = 1;

    int i;
    for (i = 0; str1[i] != '\0' && str2[i] != '\0'; i++) {
        if (str1[i] != str2[i]) {
            equal = 0;
            break;
        }
    }

    if (str1[i] != '\0' || str2[i] != '\0')
        equal = 0;

    return equal;
}

/*
 *  Given the name of a process, find its PCB entry 
 */
PROCESS find_process_by_name(char *name) {
    PROCESS this_process = NULL;

    //find the process in the PCB array
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        if (string_compare(pcb[i].name, name)) {
            this_process = &pcb[i];
            break;
        }
    }
    return this_process;
}

/*
 * Check number of processes in the pcb array. 
 * This function can be used to check if there are extra pcb entries.
 */
void check_num_of_pcb_entries(int num) {
    int counter = 0;
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        if (pcb[i].used == 1)
            counter++;
    }

    if (counter != num)
        test_result = 9;
}


/*
 * This function is used to check if a process is created correctly.
 * It checks for the PCB entry and process stack
 */
void check_create_process(char *name,
                          int priority,
                          void (*entry_point)(PROCESS, PARAM),
                          PARAM param) {
    //find the PCB entry for the process
    PROCESS this_process = find_process_by_name(name);

    //if the process is not in the pcb array, it is incorrect.
    if (this_process == NULL) {
        test_result = 10;
        return;
    }

    //check if the PCB entry is initialized correctly
    if (this_process->magic != MAGIC_PCB ||
        this_process->used != TRUE ||
        this_process->state != STATE_READY ||
        this_process->priority != priority) {
        test_result = 11;
        return;
    }

    //check new process's stack
    if (string_compare(name, boot_name)) {
        //no need to check boot process's stack
        return;
    }

    unsigned stack_pointer = this_process->sp;

    if (peek_l(stack_pointer + 28) != (LONG) entry_point) {
        test_result = 12;
        return;
    }
}


/*
 * This function checks the number of processes on the ready queues.
 * It can be used to ensure that there is no extra process on the ready queues.
 * This function works properly only when the double link lists 
 * are created correcty, i.e. a traversal that start with the first process 
 * will eventurally  return to the first process.
 */
void check_num_proc_on_ready_queue(int num) {
    int counter = 0;

    int i;
    for (i = 0; i < MAX_READY_QUEUES; i++) {
        PROCESS first_process, current_process;
        first_process = current_process = ready_queue[i];
        if (first_process != NULL) {
            do {
                counter++;
                current_process = current_process->next;
            } while (current_process != first_process);
        }
    }

    if (counter != num)
        test_result = 18;
}

/*
 * This function checks if the state of a process is correct and if 
 * the process is on ready queue or not.
 * This function works properly only when the double link lists 
 * are created correcty, i.e. a traversal that start with the first process 
 * will eventurally  return to the first process.
 */
void check_process(char *name, int state, BOOL should_be_on_ready_queue) {
    //find the PCB entry for the process
    PROCESS this_process = find_process_by_name(name);

    //if the process is not in the pcb array, it is incorrect.
    if (this_process == NULL) {
        test_result = 10;
        return;
    }

    //check the process's state
    if (this_process->state != state) {
        test_result = 13;
        return;
    }

    BOOL on = is_on_ready_queue(this_process);

    //If should_be_on_ready_queue, the process should be on ready queue and
    //state should be STATE_READY.
    //Otherwise, the process should not be on the ready queue. (state can
    //be STATE_READY or other states.)
    if (should_be_on_ready_queue == 0 && on == TRUE) {
        test_result = 14;
    } else if (should_be_on_ready_queue == 1 && on == FALSE) {
        test_result = 15;
    } else if (on == TRUE && state != STATE_READY) {
        test_result = 16;
    }
}


/*
 * Checks if a port is correct. 
 * Parameters:
 *    the_port   -- the port to be check
 *    owner_name -- the name of the the owner of the port
 *    should_be_open    -- whether the port is open
 */
void check_port(PORT the_port, char *owner_name, BOOL should_be_open) {
    //find the owner process
    PROCESS owner = find_process_by_name(owner_name);

    //check the port entry
    if (the_port->magic != MAGIC_PORT ||
        the_port->used != TRUE ||
        the_port->owner != owner) {
        test_result = 31;
        return;
    }

    if (the_port->open == TRUE && should_be_open == FALSE) {
        test_result = 34;
        return;
    }

    if (the_port->open == FALSE && should_be_open == TRUE) {
        test_result = 33;
        return;
    }

    //check if the port is in the owner's port list
    BOOL found = FALSE;
    PORT temp = owner->first_port;
    while (temp != NULL) {
        if (temp == the_port) {
            found = TRUE;
            break;
        }
        temp = temp->next;
    }

    if (!found)
        test_result = 32;
}

unsigned char recv_from_ttc(void) {
    unsigned char *ttc_read = (unsigned char *) TTC_OUT;
    unsigned char *ttc_ready = (unsigned char *) TTC_Ready;
    while (*(ttc_ready) != 1);
    return *(ttc_read);
}

void send_to_ttc(unsigned char value) {
    unsigned char *ttc_write = (unsigned char *) TTC_IN;
    *(ttc_write) = value;
}