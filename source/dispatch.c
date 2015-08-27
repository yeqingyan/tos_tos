#include <kernel.h>

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
