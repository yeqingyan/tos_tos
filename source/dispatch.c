#include <kernel.h>

#define HIGHEST_PRIORITY 7
PROCESS active_proc;

/* Ready queues for all eight priorities */
PCB *ready_queue[MAX_READY_QUEUES];
/*
 * The bits in ready_procs tell which ready queue is empty.
 * The MSB of ready_procs corresponds to ready_queue[7].
 */
//unsigned ready_procs;

/*
 * add_ready_queue()
 * -----------------
 *  The process pointed to by p is put the ready queue.
 *  The appropriate ready queue is determined by p->priority.
 *
 *  Parameters:
 *  proc: process need to be added to the queue
 */
void add_ready_queue(PROCESS proc) {
    int prio;
    volatile unsigned int cpsr_flag;
    
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    assert (proc->magic == MAGIC_PCB);
    prio = proc->priority;
    //kprintf("Add [%s] to ready queue\n", proc->name);
    if (ready_queue[prio] == NULL) {
        /* The only process on this priority level */
        ready_queue[prio] = proc;
        proc->next = proc;
        proc->prev = proc;
        //ready_procs |= 1 << prio;
    } else {
        /* Some other processes on this priority level */
        proc->next = ready_queue[prio];
        proc->prev = ready_queue[prio]->prev;
        ready_queue[prio]->prev->next = proc;
        ready_queue[prio]->prev = proc;
    }
    proc->state = STATE_READY;
    RESUME_CPSR(cpsr_flag);
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
    int prio;
    volatile unsigned int cpsr_flag;
    
    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    assert (proc->magic == MAGIC_PCB);
    prio = proc->priority;
    //kprintf("Remove [%s] from ready queue, %d\n", proc->name, proc->state);
    if (proc->next == proc) {
        //kprintf("[%s] In priority %d, No process is available\n", proc->name, proc->priority);
        //print_all_processes(kernel_window);

        ready_queue[prio] = NULL;
        //ready_procs &= ~(1 << prio);
    } else {
        ready_queue[prio] = proc->next;
        proc->next->prev = proc->prev;
        proc->prev->next = proc->next;
    }

    RESUME_CPSR(cpsr_flag);
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
 *  next running process is determined via dispatcher().
 *  The stack of the calling process is setup such that is
 *  looks like on interrupt.
 *  
 * Use resign_impl() helper function, so all code in resign are assembly 
 * and GCC compile won't add push/pop around the function.    
 */
void resign_impl() {
    active_proc = dispatcher();
}

void resign() {
    /* Save the content of the current process
     *
     * Note: Push a list of register in stack, the lowest-numbered register to 
     * the lowest memory address through to the highest-numbered register to 
     * the highest memory address. The SP(r13) and PC(r15) register cannot be 
     * in the list. (From ARMv6 manual.)
     */
    asm("push {r0-r12, r14}");

    /* Save CPSR register */
    asm("mrs r12, cpsr");
    asm("push {r12}");

    /* Set active process */
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);
    asm("bl resign_impl");
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));

    /*
     * Restore CPSR. Using CPSR_C to only change the CPSR control field.
     * if the starting process is a new process, CPSR is set to SYS mode and 
     * enable interrupt at create_process. If the starting process is not a new
     * process, we just use the CPSR we stored before.
     *
     * TODO: For new process, we do not have a return/exit address, in the 
     * future, when we have the return/exit function, we setup the return 
     * address in create_process() and pop it to LR before pop PC register. For
     * other process, we push a garbage value before call dispatcher, since LR 
     * will not be used in here. It is safe we pop the garbage value into 
     * LR here.
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
