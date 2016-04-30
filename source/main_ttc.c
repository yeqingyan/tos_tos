#include <kernel.h>
#include <test.h>

typedef void (*function_pointer)();

function_pointer tests[] = {
        test_dummy_1,
/*
    test_window_1,
    test_window_2, 
    test_window_3, 
    test_window_4, 
    test_create_process_1,
    test_create_process_2,
    test_create_process_3,
    test_create_process_4,
    test_create_process_5,
    test_dispatcher_1,
    test_dispatcher_2,
    test_dispatcher_3,
    test_dispatcher_4,
    test_dispatcher_5,
    test_dispatcher_6,
    test_dispatcher_7,
    test_ipc_1,
    test_ipc_2,
    test_ipc_3,
    test_ipc_4,
    test_ipc_5,
    test_ipc_6,
    test_isr_1,
    test_isr_2,
    test_isr_3,
    test_timer_1,
    test_com_1,
    //test_fork_1,*/
        NULL
};


void run_all_tests() {
    int i;

    test_results[0] = '\0';

    for (i = 0; tests[i] != NULL; i++) {
        // Execute the test
        tests[i]();
    }
    write_test_report(COLOR_GREEN);
    wprintf(&report_window,
            "Number of tests: %d. All tests succeeded!", i);
    while (42);
}


void run_selected_tests() {
    //init_com2();
    send_to_test_center("INIT\n");

    int i = 0;

    do {
        test_results[i] = recv_from_ttc();
    } while (test_results[i++] != '\n');
//    do {
//        int counter = 400000;
//        //while (!(inportb(COM2_PORT + 5) & 1) && --counter) ;
//        if (counter == 0) {
//            // Something went wrong receiving data from
//            // the TTC. Just run all tests.
//            run_all_tests();
//        }
//        // TODO get result
//        //test_results[i] = inportb(COM2_PORT);
//    } while (test_results[i++] != '\n');
//    test_results[i] = '\0';

    int num_tests_run = 0;
    for (i = 0; tests[i] != NULL && test_results[i] != '\n'; i++) {
        if (test_results[i] == '0')
            // Don't run this test
            continue;
        num_tests_run++;
        // Run this test. First be pessimistic and assume the
        // test is going to fail 'F'
        test_results[i] = 'F';
        // Execute the test
        tests[i]();
        // If we got here, this means that the test succeeded
        // Write 'P' for passed.
        test_results[i] = 'P';
    }
    write_test_report(COLOR_GREEN);
    wprintf(&report_window,
            "Number of tests: %d. All tests succeeded!", num_tests_run);
    send_to_test_center("RESULT=0,0,");
    send_to_test_center(test_results);
    while (42);
}

int main() {
    run_selected_tests();
    return 0;
}
