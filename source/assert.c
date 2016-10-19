#include <kernel.h>

static WINDOW error_window = {0, 41, 80, 1, 0, 0, ' '};

/**
 * Panic. Write error message to error window
 * 
 * @param msg   error message
 * @param file  file name when error happened
 * @param line  line number when error happened
 * @param func  function name when error happened
 */
void panic_mode(const char *msg, const char *file, int line, const char *func) {
  volatile unsigned int cpsr_flag;
  SAVE_CPSR_DIS_IRQ(cpsr_flag);
  clear_window(&error_window);
  wprintf(&error_window, "PANIC: '%s' at line %d of %s (function %s)",
          msg, line, file, func);
  while (1);
}

/**
 * Reset exception happened.
 * TODO: I tried to print error message here but failed. QEMU seems will hang 
 * before print the message, might because we didn't setup up the stack for 
 * these processor modes, since wprintf will push/pop variables. Same for the 
 * following exceptions.
 */
void reset_handler(void)
{
  while (1);
}

/**
 * Undefined exception happened
 */
void undef_handler(void)
{
  while (1);
}

/**
 * Software interrupt(SWI) exception happened.
 */
void swi_handler(void)
{
  while (1);
}

/**
 * Prefetch abort exception happened.
 */
void prefetch_handler(void)
{
  while (1);
}

/**
 * Data abort exception happened.
 */
void abort_handler(void)
{
  while (1);
}

/**
 * Fast interrupt exception happened.
 */
void fiq_handler(void)
{
  while (1);
}