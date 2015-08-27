#include <kernel.h>

PCB pcb[MAX_PROCS];

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
