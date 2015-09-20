#include <kernel.h>

/* Stack begin at 640 KB 
 * TODO Check sp base is ok with raspberry pi */
#define SP_BASE        0xA0000

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
 *  param:              paramters for the new process
 *  name:               process name
 *
 *  Return:
 *  A port of the process
 */
PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM), int prio, PARAM param, char *name)
{
        int i,j;

        //TODO save interrupt

        for (i=0; i<MAX_PROCS; i++) {
                if (pcb[i].used == FALSE)
                        break;
        }

        pcb[i].magic            = MAGIC_PCB;
        pcb[i].used             = TRUE;
        pcb[i].state            = STATE_READY;
        pcb[i].priority         = prio;
        pcb[i].first_port       = NULL;
        pcb[i].name             = name;
        pcb[i].next_blocked     = NULL;

        MEM_ADDR sp = SP_BASE - STACK_SIZE * i;

        /* save actual PARAM (second param of func ptr_to_new_proc) */
        sp -= 4;
        poke_l(sp, param);
        /* save create process self (first param of func ptr_to_new_proc)*/
        sp -= 4;
        poke_l(sp, (LONG)&pcb[i]);

        //sp -= 4;
        // TODO push EFLAGS here
        //poke_l(sp, 0);

        /* push CS */
        // TODO: Need check for rapsberry pi CS
        //sp -= 4;
        //poke_l(sp, (LONG)8);

        /* new process address, return address. */
        sp -= 4;
        poke_l(sp, (LONG)ptr_to_new_proc);

        /* initialize r0 to r12 */
        for (j=0; j<=12; j++){ 
                sp -= 4;
                poke_l(sp, 0);
        }

        pcb[i].sp = sp;
        pcb[i].param_proc = &pcb[i];
        pcb[i].param_data = (void *)param;
        add_ready_queue(&pcb[i]);
        WriteString("Create Process Done");
        /* TODO port related code */

        /* TODO enable interrupt here */
        
        /* TODO return port here */
        return NULL;
}
/* 
 * init_process()
 * --------------
 *  Initialize process
 */
void init_process()
{
        int i;
        for (i=0; i<MAX_PROCS; i++) {
                pcb[i].used = FALSE;
        }

        // boot process init
        pcb[0].magic            = MAGIC_PCB;
        pcb[0].used             = TRUE;
        pcb[0].state            = STATE_READY;
        pcb[0].priority         = 1;
        pcb[0].first_port       = NULL;
        pcb[0].name             = "Boot process";
}
