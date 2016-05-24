#include <kernel.h>

// Process keep system running, when boot process wait interrupt, dispatcher 
// can still find a process

void null_proc(PROCESS self, PARAM param) {
  while(1);
}

void init_null_process() {
  create_process(null_proc, 0, 0, "Null Process");
}