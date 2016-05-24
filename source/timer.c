#include <kernel.h>

// TODO Need to change GPU timer to another timer

PORT timer_port;
int ticks_remaining[MAX_PROCS];

/*
 * GetSystemTimerBase
 * ------------------
 * return timer base address.
 */
int GetSystemTimerBase() {
    return 0x20003000;
}

/*
 * GetTimeStamp
 * ------------
 *  return lower 32 bits ticks
 */
int GetTimeStamp() {
    int *timerBase = (int *) GetSystemTimerBase();
    return *(timerBase + 1);
}

/*
 * Get ARM Timer
 */
//arm_timer_t *GetARMTimer(void) {
//    return (arm_timer_t *) ARM_TIMER_BASE;
//}

system_timer_t* get_system_timer(void) {
    return (system_timer_t *) SYS_TIMER_BASE;
}

/*
 * Wait
 * ----
 *  wait n ticks.
 *
 *  Paramters:
 *  n: ticks number.
 */
void Wait(int n) {
    int start, end;

    start = GetTimeStamp();

    /* Loop if time laspe is lower than wait time */
    while (1) {
        end = GetTimeStamp();
        if ((end - start) > n) {
            break;
        }
    }

}

void timer_notifier(PROCESS self, PARAM param) {
    while (42) {
        //kprintf("I am running!\n");
        wait_for_interrupt(TIMER_IRQ);
        //kprintf("Got timer IRQ\n");
        message(timer_port, NULL);
    }
}

void timer_process(PROCESS self, PARAM param) {
    PROCESS sender;
    int proc_index;
    int i;

    //    kprintf("Create Timer notifier\n");
    create_process(timer_notifier, 7, 0, "Timer notifier");
    while (42) {
        Timer_Message *msg = receive(&sender);
        if (msg != NULL) {
            // Message from client
            proc_index = sender - pcb;
            assert(sender == &pcb[proc_index]);
            ticks_remaining[proc_index] = msg->num_of_ticks;
        } else {
            // Message from timer notifier
            for (i = 0; i < MAX_PROCS; i++) {
                if (ticks_remaining[i] != 0) {
                    ticks_remaining[i]--;
                    if (ticks_remaining[i] == 0) {
                        assert(pcb[i].state == STATE_REPLY_BLOCKED);
                        reply(&pcb[i]);
                    }
                }
            }
        }
    }
}

void sleep(int ticks) {
    Timer_Message msg;
    msg.num_of_ticks = ticks;
    send(timer_port, &msg);
}

/* timer isr */
//void isr_timer(void) {
//    //asm volatile("nop");
//    //asm volatile("nop");
//    //asm volatile("nop");
//    PROCESS p = interrupt_table[TIMER_IRQ];
//    if (p != NULL && p->state == STATE_INTR_BLOCKED) {
//        // Add event handler to read queue
//        add_ready_queue(p);
//    }
//    // Acknowledged we handled the irq
//    GetARMTimer()->IRQ_Clear = 1;
//}
//

void isr_timer(void) {
    PROCESS p = interrupt_table[TIMER_IRQ];
    if (p != NULL && p->state == STATE_INTR_BLOCKED) {
        // Add event handler to read queue
        add_ready_queue(p);
    }
    
    dmb();         // Memory barrier before/after write to peripheral
    get_system_timer()->CS.M3 = 1;  // Clear system timer 3 interrupt
    /* 
     * set up the new value in the C3 output compare register.  This is
     * done by reading the current low-order bits of the counter and adding the
     * requested number of cycles.  Note that wraparounds will necessarily be
     * handled correctly because the output compare registers are by design only
     * 32 bits.  
     */
    get_system_timer()->C3 = get_system_timer()->CLO + 
            (CLOCK_FREQ / CLKTICKS_PER_SEC);
    dmb();
}


//void init_timer() {
//    int i;
//    for (i = 0; i < MAX_PROCS; i++) {
//        ticks_remaining[i] = 0;
//    }
//
//    /* Setup timer interrupt service routine(ISR) */
//    isr_table[TIMER_IRQ] = isr_timer;
//
//    // Enable receive timer interrupt IRQ
//    enable_irq(TIMER_IRQ);
//
//    // Get Timer register address, based on BCM2835 document section 14.2
//    // Setup Timer frequency around 1kHz
//    // Get timer load to 1024
//    GetARMTimer()->Load = 0x400;
//
//    // Enable Timer, send IRQ, no-prescale, use 32bit counter
//    GetARMTimer()->Control = INTR_TIMER_CTRL_23BIT |
//            INTR_TIMER_CTRL_ENABLE |
//            INTR_TIMER_CTRL_INT_ENABLE |
//            INTR_TIMER_CTRL_PRESCALE_1;
//    timer_port = create_process(timer_process, 6, 0, "Timer Process");
//
//}

/* System timer */
void init_timer() {
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        ticks_remaining[i] = 0;
    }

    // Setup timer interrupt service routine(ISR) 
    isr_table[TIMER_IRQ] = isr_timer;
    // Enable receive timer interrupt IRQ
    enable_irq(TIMER_IRQ);
}

