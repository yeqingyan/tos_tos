#include <kernel.h>

#define HIGHEST_PRIORITY 7
PROCESS active_proc;
/* Ready queues for all eight priorites */
PCB *ready_queue[MAX_READY_QUEUES];
/*
 * add_ready_queue
 * ---------------
 *  The process pointed to by p is put the ready queue.
 *  The appropiate ready queue is determined by p->priority.
 *
 *  Parameters:
 *  proc: process need to be added
 */

void add_ready_queue (PROCESS proc)
{
        /* TODO variable used for interrupt */
        //volatile int saved_if;

        /* TODO disable interrupt here */

        // add proc to ready queue
        // proc add to the tail of the double linked list depend on its priority
        if (ready_queue[proc->priority] == NULL) {
                // add first process
                ready_queue[proc->priority] = proc;
                proc->next = proc;
                proc->prev = proc;
        } else {
                // add process to the tail of the linked list
                proc->next = ready_queue[proc->priority];
                proc->prev = ready_queue[proc->priority]->prev;
                ready_queue[proc->priority]->prev->next = proc;
                ready_queue[proc->priority]->prev = proc;
        }
        /* TODO enable interrupt here*/
}

/*
 * remove_ready_queue()
 * --------------------
 *  The process pointed to by p is dequeued from the ready queue.
 *
 *  Parameters:
 *  proc: process need to remove from ready queue
 */
void remove_ready_queue (PROCESS proc)
{
        // TODO save interrupt
        if (proc->next == proc) {
                /* I am the only process in the queue */
                ready_queue[proc->priority] = NULL;
                proc->prev = NULL;
                proc->next = NULL;
        } else {
                if (proc == ready_queue[proc->priority]) {
                        /* I am the head of the list */
                        ready_queue[proc->priority] = proc->next;
                }
                proc->prev->next = proc->next;
                proc->next->prev = proc->prev;
                proc->prev = NULL;
                proc->next = NULL;
        }
        
        // TODO enable interrupt
}
/*
 * dispatcher()
 * ----------
 *  Determines a new process to dispatched. The process
 *  with the highest priority is taken. Within one priority
 *  level round robin is used
 */
PROCESS dispatcher()
{
        int i;
        int cur_pri = active_proc->priority;

        for(i=HIGHEST_PRIORITY; i>=0; i--) {
                if(ready_queue[i] != NULL) {
                        //same priority use round robin
                        if((i == cur_pri) && (ready_queue[i] == active_proc)) {
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
 */
void resign()
{
        /* 1. save the content of the current process
         * Note: Push a list of register in stack, the lowest-numbered register to the lowest memory address
         * through to the highest-numbered register to the highest memory address.
         * The sp(r13) and pc(r15) register cannot be in the list. (From ARMv6 manual.)
         */
        //register int sp asm("sp");
        asm("push {r0-r12}");

        // 2. set active process
        //active_proc->sp = sp;
        asm("mov %[old_sp], %%sp": [old_sp] "=r" (active_proc->sp):);
        active_proc = dispatcher();
        //sp = active_proc->sp;
        asm("mov %%sp, %[new_sp]" : :[new_sp] "r" (active_proc->sp));

        asm("pop {r0-r12}");
        // TODO change to iret
        //asm("ret");
}

/*
 * init_dispatcher
 * ---------------
 *  Initialize the necessary data structures.
 */

void init_dispatcher()
{
        int i;
        active_proc = &pcb[0];

        for (i=0; i<MAX_READY_QUEUES; i++)
                ready_queue[i] = NULL;

        add_ready_queue(&pcb[0]);
}
