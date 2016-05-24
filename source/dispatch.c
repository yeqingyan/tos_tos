#include <kernel.h>

#define HIGHEST_PRIORITY 7
PROCESS active_proc;

// Ready queues for all eight priorities
PCB *ready_queue[MAX_READY_QUEUES];

/**
 * The process pointed to by p is put the ready queue.
 * The appropriate ready queue is determined by p->priority.
 * 
 * @param proc  process need to be added to the queue
 */
void add_ready_queue(PROCESS proc) {
    int prio;
    volatile unsigned int cpsr_flag;

    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    assert(proc->magic == MAGIC_PCB);
//    kprintf("[%s] added back from ready queue!\n", proc->name);
    prio = proc->priority;
    if (ready_queue[prio] == NULL) {
        // Only one process on this priority level 
        ready_queue[prio] = proc;
        proc->next = proc;
        proc->prev = proc;
    } else {
        // Some other processes on this priority level 
        proc->next = ready_queue[prio];
        proc->prev = ready_queue[prio]->prev;
        ready_queue[prio]->prev->next = proc;
        ready_queue[prio]->prev = proc;
    }
    proc->state = STATE_READY;
    RESUME_CPSR(cpsr_flag);
}

/**
 * The process pointed to by p is dequeued from the ready queue.
 * 
 * @param proc  process need to remove from ready queue
 */
void remove_ready_queue(PROCESS proc) {
    int prio;
    volatile unsigned int cpsr_flag;

    SAVE_CPSR_DIS_IRQ(cpsr_flag);
    assert(proc->magic == MAGIC_PCB);
    prio = proc->priority;
    //kprintf("[%s] remove [%s] from ready queue, %d\n", active_proc->name, proc->name, proc->state);
    if (proc->next == proc) {
        //kprintf("[%s] In priority %d, No process is available\n", proc->name, proc->priority);
        ready_queue[prio] = NULL;
    } else {
        ready_queue[prio] = proc->next;
        proc->next->prev = proc->prev;
        proc->prev->next = proc->next;
    }

    RESUME_CPSR(cpsr_flag);
}

/**
 * Determines a new process to dispatched. The process with the highest 
 * priority is taken. Within one priority level round robin is used.
 * 
 * @return ptr to a new process's pcb.
 */
PROCESS dispatcher() {
    int i;
    int cur_pri = active_proc->priority;

    for (i = HIGHEST_PRIORITY; i >= 0; i--) {
        if (ready_queue[i] != NULL) {
            if ((i == cur_pri) && (ready_queue[i] == active_proc)) {
                ready_queue[i] = ready_queue[i]->next;
            }
            assert(ready_queue[i]->magic == MAGIC_PCB);
            //kprintf("Got process %s\n", ready_queue[i]->name);
//            ps();
            return ready_queue[i];
        }
    }
    // should not run into this line
    assert(0);
    return NULL;
}

/**
 * Call by master_isr() and resign(). this is a help function, so we do not 
 * need to worried about return value in assembly. Since so all code in resign()
 * and master_isr() are assembly, GCC compile won't add push/pop around the 
 * function.    
 */
void dispatcher_impl() {
    active_proc = dispatcher();
}

/**
 * The current process gives up the CPU voluntarily. The next running process 
 * is determined via dispatcher(). The stack of the calling process is setup 
 * such that is looks like on interrupt.
 */
void resign() {
    // Save CPSR register and link register 
    // SRS (Store Return State) stores the LR and SPSR of the current mode to 
    // the stack in SYS mode. (0x1f) indicate the SYS mode. "db" (Decrement 
    // Before) suffix means CPU will decrement the stack pointer before storing 
    // values onto stack. "!" means after store R14 and SPSR of the IRQ mode 
    // into SYS mode stack, update the stack pointer in SYS mode. (Ref. ARM 
    // Manual B9.3.16)
    // This instruction is equal to:
   // TODO they are different !!!!
    asm("mrs r12, cpsr");
    asm("push {r12}");
    asm("push {lr}");
    //asm("srsdb #0x1f!"); // store lr and SPSR to stack
    
    // Save the content of the current process
    //    
    // Note: Push a list of register in stack, the lowest-numbered register to 
    // the lowest memory address through to the highest-numbered register to 
    // the highest memory address. The SP(r13) and PC(r15) register cannot be 
    // in the list. (Ref. ARM Manual A8.8.133)
    asm("push {r0-r12, r14}");

    // Set active process
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);
    
    // data memory barrier. see dmb implmentation in intr.c
    asm("bl dmb");      
    asm("bl dispatcher_impl");
    asm("bl dmb");      
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));

    // Restore the content of the process
    asm("pop {r0-r12, r14}");
    
    // RFE (Return From Exception) loads LR, SPSR on SYS stack into PC and CPSR 
    // registers, "ia" (increase after) means increment stack pointer after 
    // read from stack. "!" means update value in sp after instruction. (Ref. 
    // ARM Manual B9.3.13)
    asm("rfeia sp!");
}

/**
 * Initialize the necessary data structures.
 */
void init_dispatcher() {
    int i;
    active_proc = &pcb[0];

    for (i = 0; i < MAX_READY_QUEUES; i++)
        ready_queue[i] = NULL;

    add_ready_queue(&pcb[0]);
}
