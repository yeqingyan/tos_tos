#include <kernel.h>

// Stack begin at 640 KB
// TODO Check sp base is ok with raspberry pi
#define SP_BASE 0xA00000

// Each process have 30 KB stack
#define STACK_SIZE 0x7800

PCB pcb[MAX_PROCS];

/**
 * Create process
 *
 * @param ptr_to_new_proc       function pointer point to a new process
 * @param prio                  priority number
 * @param param                 parameters for the new process
 * @param name                  process name
 * @return                      port of the new process
 */
PORT create_process(void (*ptr_to_new_proc)(PROCESS, PARAM), int prio,
                    PARAM param, char *name) {
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

  // save actual PARAM (second param of func ptr_to_new_proc)
  sp -= 4;
  poke_l(sp, param);

  // save create process self (first param of func ptr_to_new_proc)
  sp -= 4;
  poke_l(sp, (LONG)&pcb[i]);

  // Push CPSR here, set mode to SYS mode, enable irq,
  // since enalbe irq is clear the 7th bit of cpsr will enable IRQ
  sp -= 4;
  poke_l(sp, 0x15f);

  // new process address
  sp -= 4;
  poke_l(sp, (LONG)ptr_to_new_proc);

  // dummy return address here
  // TODO change this to a funciton that will do while loop
  sp -= 4;
  poke_l(sp, (LONG)0);

  // initialize r0 to r12
  for (j = 0; j <= 12; j++) {
    sp -= 4;
    poke_l(sp, 0);
  }

  pcb[i].sp = sp;
  pcb[i].param_proc = &pcb[i];
  pcb[i].param_data = (void *)param;
  add_ready_queue(&pcb[i]);

  PORT new_port = create_new_port(&pcb[i]);

  RESUME_CPSR(cpsr_flag);

  return new_port;
}

/**
 * Print process status
 *
 * @param wnd
 * @param p
 */
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

/**
 * Print all processes
 *
 * @param wnd
 */
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

// For debug only, do not disable irq
/**
 * since print will disable/enable the interrupt, when using gdb debuging in
 * irq, sometimes call print_all_processes will cause QEMU hang, this one do not
 * disable/enable irq, so we can use it in gdb without worries it will hang QEMU
 *
 * @param wnd
 */
void debug_print_all_process(WINDOW *wnd) {
  int i;
  const char *states[6];
  PROCESS p;
  debugprintf(wnd, "State             Active Prio Name\n");
  debugprintf(wnd, "--------------------------------------------------\n");

  for (i = 0; i < MAX_PROCS; i++) {
    if (pcb[i].used == TRUE) {
      p = &pcb[i];
      const char *states[6];
      states[0] = "READY";
      states[1] = "SEND_BLOCKED";
      states[2] = "REPLY_BLOCKED";
      states[3] = "RECEIVE_BLOCKED";
      states[4] = "MESSAGE_BLOCKED";
      states[5] = "INTR_BLOCKED";

      // print state
      assert((p->state < 6) && (p->state >= 0));
      debugprintf(wnd, "%-18s", states[p->state]);
      // print active
      if (active_proc == p) {
        debugprintf(wnd, "%8s", "*");
      } else {
        debugprintf(wnd, "%8s", " ");
      }

      debugprintf(wnd, "%2d ", p->priority);
      debugprintf(wnd, "%s\n", p->name);
    }
  }
}

/**
 * Initialize processes
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
