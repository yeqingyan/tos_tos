#include <kernel.h>
// TODO update comments
#define CONTACTS_NUMBER 16

WINDOW* train_wnd;
int contacts[CONTACTS_NUMBER + 1];

int zamboni;
int sleep_time = 1;
int check_ticks = 20;
// Array used for convert int to string 
char int_to_string[CONTACTS_NUMBER + 1][3] = {"0", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};

void inc_sleep_time(WINDOW *);
void dec_sleep_time(WINDOW *);
void inc_check_time(WINDOW *);
void dec_check_time(WINDOW *);
void cmd_sleep();
void clear_buf();
void change_direction(char);
void change_speed(char, int);
void change_switch(int, char);
int get_contact_status(int);
void run_train();
void train_process(PROCESS, PARAM);

/*
 * Function: inc_sleep_time
 * ------------------------
 *  increase command sleep time by 10
 *
 *  wnd: tos window pointer
 *
 */
void inc_sleep_time(WINDOW * wnd) {
    sleep_time += 10;
    wprintf(wnd, "Command sleep time now is %d\n", sleep_time);
}

/*
 * Function: dec_sleep_time
 * ------------------------
 *  decrease command sleep time by 10, time can not be negative.
 *
 *  wnd: tos window pointer
 *
 */
void dec_sleep_time(WINDOW * wnd) {
    if (sleep_time >= 10)
        sleep_time -= 10;
    wprintf(wnd, "Command sleep time now is %d\n", sleep_time);
}

/* 
 * Function: inc_check_time
 * ------------------------
 *  increase check zamboni time by 10
 *
 *  wnd: tos window pointer
 *
 */
void inc_check_time(WINDOW * wnd) {
    check_ticks += 10;
    wprintf(wnd, "Check ticks now is %d\n", check_ticks);
}

/* 
 * Function: dec_check_time
 * ------------------------
 *  decrease check zamboni time by 10, time can not be negative
 *
 *  wnd: tos window pointer
 *
 */
void dec_check_time(WINDOW * wnd) {
    if (check_ticks >= 10)
        check_ticks -= 10;
    wprintf(wnd, "Check ticks now is %d\n", check_ticks);
}

/* 
 * Function: cmd_sleep
 * ------------------------
 *  sleep between commands
 *
 */
void cmd_sleep() {
    //sleep(sleep_time);
    Wait(sleep_time);
}

/* 
 * Function: clear_buf
 * -------------------
 *  tos command 'R', clear s88 memory buffer
 */
void clear_buf() {
    Serial_Message msg;
    char out_buf[] = "R\015";

    msg.output_buffer = out_buf;
    msg.input_buffer = NULL;
    msg.len_input_buffer = 0;

    send(serial_port, &msg);
    cmd_sleep();
}

/* 
 * Function: change_direction
 * --------------------------
 *  Change the direction of the vehicle, tos command  'L20'
 */
void change_direction(char vehicle) {
    Serial_Message msg;
    char out_buf[6];

    out_buf[0] = 'L';
    switch (vehicle) {
        case 'T':
            out_buf[1] = '2';
            out_buf[2] = '0';
            break;
        default:
            break;
    }
    out_buf[3] = 'D';
    out_buf[4] = '\015';
    out_buf[5] = '\0';

    msg.output_buffer = out_buf;
    msg.input_buffer = NULL;
    msg.len_input_buffer = 0;

    send(serial_port, &msg);
    cmd_sleep();
}

/* 
 * Function change_speed
 * ---------------------
 *  Change speed of a vehicle, tos command 'L20S4''T'
 */
void change_speed(char vehicle, int speed) {
    Serial_Message msg;
    char out_buf[7];

    out_buf[0] = 'L';
    switch (vehicle) {
        case 'T':
            out_buf[1] = '2';
            out_buf[2] = '0';
            break;
        default:
            break;
    }
    out_buf[3] = 'S';
    out_buf[4] = int_to_string[speed][0];
    out_buf[5] = '\015';
    out_buf[6] = '\0';

    msg.output_buffer = out_buf;
    msg.input_buffer = NULL;
    msg.len_input_buffer = 0;

    send(serial_port, &msg);
    cmd_sleep();
}

/* 
 * Function change_switch
 * ----------------------
 *  Change the switch to specific color, tos command 'M8G'
 *  
 */
void change_switch(int switch_id, char color) {
    Serial_Message msg;
    char out_buf[5];

    out_buf[0] = 'M';
    out_buf[1] = int_to_string[switch_id][0];
    out_buf[2] = color;
    out_buf[3] = '\015';
    out_buf[4] = '\0';

    msg.output_buffer = out_buf;
    msg.input_buffer = NULL;
    msg.len_input_buffer = 0;

    send(serial_port, &msg);
    cmd_sleep();
}

/* 
 * Function change_switch
 * ----------------------
 *  get contact status, tos command 'C9'
 *
 *  return: contact status 
 *  
 */
int get_contact_status(int contact_id) {
    Serial_Message msg;
    char input_buf[4];
    char contact_buf[5];
    char *buf_ptr = contact_buf;

    *buf_ptr = 'C';
    buf_ptr++;
    /* Give more space if contact_id is bigger than 1 */
    if (contact_id > 9) {
        k_memcpy(buf_ptr, int_to_string[contact_id], 2);
        buf_ptr += 2;
    } else {
        k_memcpy(buf_ptr, int_to_string[contact_id], 1);
        buf_ptr += 1;
    }
    *(buf_ptr) = '\015';
    buf_ptr++;
    *(buf_ptr) = '\0';
    msg.output_buffer = contact_buf;
    msg.input_buffer = input_buf;
    msg.len_input_buffer = sizeof (input_buf);

    clear_buf();
    send(serial_port, &msg);
    cmd_sleep();
    /*
    int i = 0;
    kprintf("Got Message:");
    for (i = 0; i < 6; i++) {
      kprintf("%x ", input_buf[i]);
    }
    kprintf("\n");*/
    return input_buf[1] - '0';
}

/* 
 * Function run_train 
 * ------------------
 *  Get all contacts information 
 *  Config 1: contact[2]=1 contact[8]=1 
 *  Config 2: contact[2]=1 contact[8]=1 
 *  Config 3: contact[5]=1 contact[11]=1 
 *  Config 4: contact[5]=1 contact[16]=1
 */
void run_train() {
    int i;

    wprintf(train_wnd, "Checking contacts");
    /* Get contact id */
    contacts[8] = get_contact_status(8);
    contacts[11] = get_contact_status(11);
    contacts[16] = get_contact_status(16);

    if (contacts[8] == 1) {
        wprintf(train_wnd, "Configuration 1,2\n"); /* Process configuration 1&2 */
        wprintf(train_wnd, "Try trap Zamboni"); /* Following code try to trap zamboni */
        change_switch(4, 'G');
        change_switch(5, 'G');
        change_switch(8, 'R');
        change_switch(9, 'R');
        change_switch(1, 'R');
        change_switch(2, 'R');
        change_switch(7, 'R');

        for (i = 0; i < check_ticks; i++) { /* Looking for Zamboni*/
            wprintf(train_wnd, ".");
            if (get_contact_status(15) == 1) {
                wprintf(train_wnd, "Found Zamboni!\n");
                zamboni = 1;
                break;
            }
        }
        /* Using same strategy regardless of Zamobni */
        wprintf(train_wnd, "\n");
        change_switch(3, 'G');
        change_switch(4, 'R');
        change_switch(5, 'R');
        change_switch(6, 'R');
        change_speed('T', 4);
        while (get_contact_status(1) != 1);
        change_speed('T', 0);
        change_direction('T');
        change_speed('T', 4);
        while (get_contact_status(8) != 1);
        change_speed('T', 0);
    } else if (contacts[11] == 1) { /* Configuration 3 */
        wprintf(train_wnd, "Configuration 3\n"); /* Process configuration 1&2 */
        change_switch(4, 'G'); /* Try trap Zamboni */
        change_switch(5, 'G');
        change_switch(8, 'G');
        change_switch(9, 'R');
        change_switch(1, 'G');
        for (i = 0; i < check_ticks; i++) {
            if (get_contact_status(6) == 1) {
                wprintf(train_wnd, "Found Zamboni!\n");
                zamboni = 1;
                break;
            }
        }
        if (zamboni == 0) {
            wprintf(train_wnd, "Configuration 3 without zamboni\n");
            change_switch(3, 'R');
            change_switch(4, 'R');
            change_switch(5, 'R');
            change_switch(6, 'G');
            change_switch(7, 'G');
            change_speed('T', 4);
            while (get_contact_status(12) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(7, 'R');
            change_switch(8, 'R');
            change_speed('T', 4);
            while (get_contact_status(13) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(8, 'G');
            change_switch(5, 'G');
            change_speed('T', 4);
            while (get_contact_status(5) != 1);
            change_speed('T', 0);
        } else {
            wprintf(train_wnd, "Configuration 3 with zamboni\n");
            change_switch(3, 'R');
            change_switch(4, 'R');
            change_speed('T', 5);
            while (get_contact_status(10) != 1);
            change_switch(5, 'R');
            change_switch(6, 'G');
            while (get_contact_status(12) != 1);
            change_speed('T', 0);
            change_switch(5, 'G');
            change_switch(7, 'R');
            change_direction('T');
            while (get_contact_status(3) != 1); /* Wait for zamboni to pass */
            change_switch(1, 'R');
            change_switch(2, 'G');
            change_speed('T', 5);
            while (get_contact_status(1) != 1);
            change_speed('T', 0);
            change_switch(1, 'R');
            change_switch(2, 'R');
            change_switch(7, 'R');
            while (get_contact_status(12) != 1);
            change_speed('T', 4);
            while (get_contact_status(6) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(3, 'R');
            change_switch(4, 'R');
            change_speed('T', 4);
            while (get_contact_status(5) != 1);
            change_speed('T', 0);

        }
    } else if (contacts[16] == 1) { /* Configuration 4 */
        change_switch(4, 'G');
        change_switch(8, 'R');
        for (i = 0; i < check_ticks; i++) {
            if (get_contact_status(12) == 1) {
                wprintf(train_wnd, "Found Zamboni!\n");
                zamboni = 1;
                break;
            }
        }
        if (zamboni == 1) {
            wprintf(train_wnd, "Configuration 4 with Zamboni!\n");
            change_switch(3, 'R');
            change_switch(4, 'R');
            change_speed('T', 4);
            while (get_contact_status(6) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(4, 'G');
            change_switch(1, 'G');
            change_switch(9, 'R');
            change_speed('T', 4);
            while (get_contact_status(3) != 1);
            change_speed('T', 0);
            while (get_contact_status(14) != 1);
            change_switch(8, 'G');
            while (get_contact_status(10) != 1);
            change_switch(8, 'R');
            change_speed('T', 5);
            while (get_contact_status(12) != 1);
            change_speed('T', 0);
            change_switch(8, 'G');
            while (get_contact_status(14) != 1);
            change_switch(9, 'G');
            while (get_contact_status(10) != 1);
            change_speed('T', 5);
            while (get_contact_status(14) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_speed('T', 5);
            while (get_contact_status(14) != 0);
            change_speed('T', 4);
            change_speed('T', 3);
            change_speed('T', 2);
            change_speed('T', 1);
            change_speed('T', 0);
            change_direction('T');
            wprintf(train_wnd, "Got the cargo, hopefully\n");
            change_switch(8, 'R');
            wprintf(train_wnd, "Try trap Zamboni\n");
            while (get_contact_status(12) != 1);
            wprintf(train_wnd, "Zamboni trapped\n");
            while (get_contact_status(14) != 1);
            change_switch(8, 'G');
            wprintf(train_wnd, "Zamboni is out of the loop!\n");
            while (get_contact_status(6) != 1);
            change_speed('T', 5);
            while (get_contact_status(10) != 1);
            wprintf(train_wnd, "Start train with cargo\n");
            change_speed('T', 4);
            change_switch(8, 'R');
            while (get_contact_status(7) != 1);
            change_switch(4, 'R');
            change_switch(3, 'R');
            change_speed('T', 4);
            while (get_contact_status(5) != 1);
            change_speed('T', 0);
        } else {
            wprintf(train_wnd, "Configuration 4 without zamboni\n");
            change_switch(3, 'R');
            change_switch(4, 'R');
            change_speed('T', 4);
            while (get_contact_status(6) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(4, 'G');
            change_switch(1, 'G');
            change_switch(9, 'R');
            change_speed('T', 4);
            while (get_contact_status(14) != 1);
            change_speed('T', 0);
            change_direction('T');
            change_switch(9, 'G');
            change_speed('T', 5);
            while (get_contact_status(14) != 0);
            change_speed('T', 3);
            //sleep(20);
            Wait(20);
            change_speed('T', 0);
            change_direction('T');
            change_switch(9, 'G');
            change_switch(8, 'G');
            change_switch(5, 'G');
            change_switch(4, 'R');
            change_switch(3, 'R');
            change_speed('T', 4);
            while (get_contact_status(5) != 1);
            change_speed('T', 0);
        }
    }
}

void train_process(PROCESS self, PARAM param) {
    disable_keyboard();
    run_train();
    enable_keyboard();
    remove_ready_queue(self);
    resign();
}

void init_train(WINDOW* wnd) {
    int i;
    train_wnd = wnd;
    zamboni = 0;

    for (i = 0; i < CONTACTS_NUMBER; i++) {
        contacts[i] = 0;
    }
    create_process(train_process, 5, 0, "Train Process");
}

void train_test() {
    /* Check contact 2 should return 2 */
    disable_keyboard();
    Serial_Message msg;
    char input_buf[4];
    char contact_buf[5];
    char *buf_ptr = contact_buf;
    int contact_id = 2;

    *buf_ptr = 'C';
    buf_ptr++;
    /* Give more space if contact_id is bigger than 1 */
    if (contact_id > 9) {
        k_memcpy(buf_ptr, int_to_string[contact_id], 2);
        buf_ptr += 2;
    } else {
        k_memcpy(buf_ptr, int_to_string[contact_id], 1);
        buf_ptr += 1;
    }
    *(buf_ptr) = '\015';
    buf_ptr++;
    *(buf_ptr) = '\0';
    msg.output_buffer = contact_buf;
    msg.input_buffer = input_buf;
    msg.len_input_buffer = sizeof (input_buf);

    clear_buf();
    send(serial_port, &msg);
    cmd_sleep();
    int i = 0;
    kprintf("Got Message:");
    for (i = 0; i < sizeof (input_buf); i++) {
        kprintf("%x ", input_buf[i]);
    }
    kprintf("\n");
    kprintf("Result = %d, shoulde be 1", input_buf[1] - '0');
    enable_keyboard();
}