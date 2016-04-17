#include <kernel.h>

#define HIGHEST_PRIORITY 7
PROCESS active_proc;

/* Ready queues for all eight priorites */
PCB *ready_queue[MAX_READY_QUEUES];

/*
 * add_ready_queue()
 * -----------------
 *  The process pointed to by p is put the ready queue.
 *  The appropiate ready queue is determined by p->priority.
 *
 *  Parameters:
 *  proc: process need to be added to the queue
 */
void add_ready_queue(PROCESS proc) {
    DISABLE_INTR();

    if (ready_queue[proc->priority] == NULL) {
        ready_queue[proc->priority] = proc;
        proc->next = proc;
        proc->prev = proc;
    } else {
        proc->next = ready_queue[proc->priority];
        proc->prev = ready_queue[proc->priority]->prev;
        ready_queue[proc->priority]->prev->next = proc;
        ready_queue[proc->priority]->prev = proc;
    }

    ENABLE_INTR();
}

/*
 * remove_ready_queue()
 * --------------------
 *  The process pointed to by p is dequeued from the ready queue.
 *
 *  Parameters:
 *  proc: process need to remove from ready queue
 */
void remove_ready_queue(PROCESS proc) {
    DISABLE_INTR();

    if (proc->next == proc) {
        ready_queue[proc->priority] = NULL;
        proc->prev = NULL;
        proc->next = NULL;
    } else {
        if (proc == ready_queue[proc->priority]) {
            ready_queue[proc->priority] = proc->next;
        }
        proc->prev->next = proc->next;
        proc->next->prev = proc->prev;
        proc->prev = NULL;
        proc->next = NULL;
    }

    ENABLE_INTR();
}

/*
 * dispatcher()
 * ----------
 *  Determines a new process to dispatched. The process
 *  with the highest priority is taken. Within one priority
 *  level round robin is used
 */
PROCESS dispatcher() {
    int i;
    int cur_pri = active_proc->priority;

    for (i = HIGHEST_PRIORITY; i >= 0; i--) {
        if (ready_queue[i] != NULL) {
            if ((i == cur_pri) && (ready_queue[i] == active_proc)) {
                ready_queue[i] = ready_queue[i]->next;
            }
            return ready_queue[i];
        }
    }

    // should not run into this line
    assert(0);
    return NULL;
}

/*
 * resign()
 * --------
 *  The current process gives up the CPU voluntarily. The
 *  next running process is determined iva dispatcher().
 *  The stack of the calling process is setup such that is
 *  looks like on interrupt.
 *  
 * Use resign_impl helper function, so all code in resign are assembly 
 * and gcc compile won't add push/pop around the function.    
 */
void resign_impl() {
    active_proc = dispatcher();
}

void resign() {
    /* Save the content of the current process
     *
     * Note: Push a list of register in stack, the lowest-numbered register to the lowest memory address
     * through to the highest-numbered register to the highest memory address.
     * The sp(r13) and pc(r15) register cannot be in the list. (From ARMv6 manual.)
     */
    asm("push {r0-r12, r14}");

    /* Save cpsr register */
    asm("mrs r12, cpsr");
    asm("push {r12}");

    /* Set active process */
    asm("mov %[old_sp], %%sp": [old_sp] "=r"(active_proc->sp):);
    asm("bl resign_impl");
    asm("mov %%sp, %[new_sp]" : :[new_sp] "r"(active_proc->sp));

    /*
     * Restore CPSR. Using cpsr_c to only change the CPSR control field.
     * if the starting process is a new process, CPSR is set to SYS mode and enable interrupt at create_process.
     * if the starting process is not a new process, we just use the cpsr we stored before.
     *
     * TODO: For new process, we do not have a return/exit address, in the future, when we have the return/exit
     * function, we setup the return address in create_process() and pop it to lr before pop pc register. For
     * other process, we push a garbage value before call dispatcher, since lr will not be used in here. It is
     * safe we pop the garbage value into lr here.
    */
    asm("pop {r12}");
    asm("msr cpsr_c, r12");

    asm("pop {r0-r12, pc}");
}

/*
 * init_dispatcher()
 * -----------------
 *  Initialize the necessary data structures.
 */

void init_dispatcher() {
    int i;
    active_proc = &pcb[0];

    for (i = 0; i < MAX_READY_QUEUES; i++)
        ready_queue[i] = NULL;

    add_ready_queue(&pcb[0]);
}
