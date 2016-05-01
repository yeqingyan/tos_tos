#include <kernel.h>

/* Stack begin at 640 KB 
 * TODO Check sp base is ok with raspberry pi */
#define SP_BASE        0xA00000

/* Each process have 30 KB stack */
#define STACK_SIZE      0x7800

PCB pcb[MAX_PROCS];

/*
 * create_process()
 * ----------------
 *  Create Process
 *
 *  Parameters:
 *  ptr_to_new_proc:    function pointer point to a new process
 *  prio:               priority number
 *  param:              parameters for the new process
 *  name:               process name
 *
 *  Return:
 *  A port of the process
 */
PORT create_process(void (*ptr_to_new_proc)(PROCESS, PARAM), int prio, PARAM param, char *name) {
    volatile unsigned int cpsr_flag;
    int i, j;

    // DISABLE INTERRUPT
    SAVE_CPSR_DIS_IRQ(cpsr_flag);

    for (i = 0; i < MAX_PROCS; i++) {
        if (pcb[i].used == FALSE)
            break;
    }

    pcb[i].magic = MAGIC_PCB;
    pcb[i].used = TRUE;
    pcb[i].state = STATE_READY;
    pcb[i].priority = prio;
    pcb[i].first_port = NULL;
    pcb[i].name = name;
    pcb[i].next_blocked = NULL;

    MEM_ADDR sp = SP_BASE - STACK_SIZE * i;

    /* save actual PARAM (second param of func ptr_to_new_proc) */
    sp -= 4;
    poke_l(sp, param);
    
    /* save create process self (first param of func ptr_to_new_proc)*/
    sp -= 4;
    poke_l(sp, (LONG) & pcb[i]);

    sp -= 4;
    /* Push CPSR here, set mode to SYS mode, enable irq,
     * since enalbe irq is clear the 7th bit of cpsr will enable IRQ */
    poke_l(sp, 0x15f);

    /* new process address */
    sp -= 4;
    poke_l(sp, (LONG) ptr_to_new_proc);

    /* dummy return address here */
    sp -= 4;
    poke_l(sp, (LONG) 0);
    
    /* initialize r0 to r12 */
    for (j = 0; j <= 12; j++) {
        sp -= 4;
        poke_l(sp, 0);
    }    
    pcb[i].sp = sp;
    pcb[i].param_proc = &pcb[i];
    pcb[i].param_data = (void *) param;
    add_ready_queue(&pcb[i]);

    PORT new_port = create_new_port(&pcb[i]);

    RESUME_CPSR(cpsr_flag);

    return new_port;
}

void print_process(WINDOW *wnd, PROCESS p) {
    const char *states[6];
    states[0] = "READY";
    states[1] = "SEND_BLOCKED";
    states[2] = "REPLY_BLOCKED";
    states[3] = "RECEIVE_BLOCKED";
    states[4] = "MESSAGE_BLOCKED";
    states[5] = "INTR_BLOCKED";

    // print state
    assert((p->state < 6) && (p->state >= 0));
    wprintf(wnd, "%-18s", states[p->state]);
    // print active
    if (active_proc == p) {
        wprintf(wnd, "%8s", "*");
    } else {
        wprintf(wnd, "%8s", " ");
    }

    wprintf(wnd, "%2d ", p->priority);
    wprintf(wnd, "%s\n", p->name);
}

void print_all_processes(WINDOW *wnd) {
    int i;
    wprintf(wnd, "State             Active Prio Name\n");
    wprintf(wnd, "--------------------------------------------------\n");


    for (i = 0; i < MAX_PROCS; i++) {
        if (pcb[i].used == TRUE) {
            print_process(wnd, &pcb[i]);
        }
    }
}

/* 
 * init_process()
 * --------------
 *  Initialize process
 */
void init_process() {
    int i;
    for (i = 0; i < MAX_PROCS; i++) {
        pcb[i].used = FALSE;
    }

    // boot process init
    pcb[0].magic = MAGIC_PCB;
    pcb[0].used = TRUE;
    pcb[0].state = STATE_READY;
    pcb[0].priority = 1;
    pcb[0].first_port = NULL;
    pcb[0].name = "Boot process";
}
