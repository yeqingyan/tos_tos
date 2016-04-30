#ifndef __TEST_H__
#define __TEST_H__

#define COLOR_RED   0x0C
extern int test_result;
extern unsigned check_sum;
extern WINDOW report_window;

#define MAX_NUM_TESTS 100
extern char test_results[MAX_NUM_TESTS];

void test_reset();

void return_to_boot();

void write_test_report(unsigned short);

void send_to_test_center(char *cmd);

void test_failed_impl(int code, char *file, int line);

#define test_failed(code) test_failed_impl(code, __FILE__, __LINE__)

void check_screen_output(char **contents);

int string_compare(char *str1, char *str2);

void check_num_of_pcb_entries(int num);

void check_create_process(char *name,
                          int priority,
                          void (*entry_point)(PROCESS, PARAM),
                          PARAM param);

void check_process(char *name, int state, BOOL is_on_ready_queue);

void check_num_proc_on_ready_queue(int num);

PROCESS find_process_by_name(char *name);

BOOL is_on_ready_queue(PROCESS pro);

void check_port(PORT the_port, char *owner_name, BOOL is_open);

void test_dummy_1();

/*
void test_window_1();
void test_window_2();
void test_window_3();
void test_window_4();

void test_create_process_1();
void test_create_process_2();
void test_create_process_3();
void test_create_process_4();
void test_create_process_5();

void test_dispatcher_1();
void test_dispatcher_2();
void test_dispatcher_3();
void test_dispatcher_4();
void test_dispatcher_5();
void test_dispatcher_6();
void test_dispatcher_7();

void test_ipc_1();
void test_ipc_2();
void test_ipc_3();
void test_ipc_4();
void test_ipc_5();
void test_ipc_6();

void test_isr_1();
void test_isr_2();
void test_isr_3();

void test_timer_1();
void test_com_1();
void test_fork_1();
*/

unsigned char recv_from_ttc(void);

void send_to_ttc(unsigned char);

#endif
