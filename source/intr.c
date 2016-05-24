#include <kernel.h>

/*
 * Ref. BCM2835 manual section 7.5 
 * TODO add interrupt details for irq
 */

// Store enabled basic IRQs
static unsigned int enabled_basic_irqs;
static unsigned int temp_enabled_basic_irqs;
static unsigned int current_irq;
static unsigned int enable_usb = 0;

/**
 * Get IRQ Controller
 * 
 * @return  IRQ controller base address
 */
irq_controller_t *GetIRQController(void) {
    return (irq_controller_t *) INTR_BASE;
}


/**
 * Enable IRQ
 * TODO We use ARM_TIMER_0 is basic IRQ. Other IRQ went to IRQ_Pending_1 
 * 
 * @param intr_no   interrupt number
 */
void enable_irq(int intr_no) {
//    if (intr_no == 0) {
//        enabled_basic_irqs |= (1 << intr_no);
//        GetIRQController()->Enable_Basic_IRQs = (1 << intr_no);
//    } else if (intr_no == USB_IRQ) {
//        enable_usb = 1;
    if (intr_no == USB_IRQ) {
        enable_usb = 1;
    }
    GetIRQController()->Enable_IRQs_1 = (1 << intr_no);
}

/**
 * clear IRQ
 * 
 * @param intr_no   interrupt number
 */
void clear_irq(int intr_no) {
//    if (intr_no == 0) {
//        enabled_basic_irqs &= ~(1 << intr_no);
//        GetIRQController()->Disable_Basic_IRQs |= (1 << intr_no);
//    } else if (intr_no == USB_IRQ) {
    if (intr_no == USB_IRQ) {
        enable_usb = 0;
    }
    GetIRQController()->Disable_IRQs_1 |= (1 << intr_no);
}

/* 
    ISR dispatcher 
    Dispatch enabled interrupt routine
    
    TODO: Change Timer IRQ from basic irq to IRQs_1. change for loop to link list.
      */
/**
 * ISR dispatcher, dispatch enabled interrupt routine.
 * Note, we only handle TIMER_IRQ and USB_IRQ now.
 * 
 * TODO: Change Timer IRQ from basic irq to IRQs_1. change for loop to link list.
 */
void irq_handler(void) {
    // Interrupt alreay disabled in irq_handler
    
    temp_enabled_basic_irqs = enabled_basic_irqs;
    current_irq = 0;
    //kprintf("In irq_handler\n");
//    if (((GetIRQController()->IRQ_Basic_Pending) & (1 << TIMER_IRQ)) > 0) {
//        assert(isr_table[TIMER_IRQ] != NULL);
//        //kprintf("run timer\n");
//        isr_table[TIMER_IRQ]();
//    }
    if (((GetIRQController()->Enable_IRQs_1) & (1 << TIMER_IRQ)) > 0) {
        assert(isr_table[TIMER_IRQ] != NULL);
        isr_table[TIMER_IRQ]();
    }
    
//    while (temp_enabled_basic_irqs) {
//        // Find enable irq
//        if ((temp_enabled_basic_irqs & 1) == 1) {
//            // Check IRQ is pending or not
//            if ((((GetIRQController()->IRQ_Basic_Pending) & (1 << current_irq)) > 0) &&
//                    (isr_table[current_irq] != NULL)) {
//                isr_table[current_irq]();
//            }
//        }
//        temp_enabled_basic_irqs = temp_enabled_basic_irqs >> 1;
//        current_irq += 1;
//    }

//    if (enable_usb && ((GetIRQController()->IRQ_Pending_1) & (1 << USB_IRQ)) > 0) {
//        assert(isr_table[USB_IRQ] != NULL);
//        isr_table[USB_IRQ]();
//    }
}

/**
 * Data memory barrier. Used in master_isr() and resign().
 * 
 * According to BCM2835 1.3 Peripheral access precautions for correct memory 
 * ordering.
 * 
 * Accesses to the same peripheral will always arrive and return in-order. 
 * It is only when switching from one peripheral to another that data can arrive
 * out-of-order. The simplest way to make sure that data is processed in-order 
 * is to place a memory barrier instruction at critical positions in the code. 
 * You should place:
 * 
 * A memory write barrier before the first write to a peripheral.
 * A memory read barrier after the last read of a peripheral.
 * 
 * It is not required to put a memory barrier instruction after each read or 
 * write access. Only at those places in the code where it is possible that a 
 * peripheral read or write may be followed by a read or write of a different 
 * peripheral. This is normally at the entry and exit points of the peripheral 
 * service code.
 * As interrupts can appear anywhere in the code so you should safeguard those. 
 * If an interrupt routine reads from a peripheral the routine should start with
 * a memory read barrier. If an interrupt routine writes to a peripheral the 
 * routine should end with a memory write barrier.
 * 
 * Ref. BCM2835 1.3
 * Ref. Xinu project https://github.com/xinu-os/xinu/blob/4384915cc63a9c057418015c996f44372eec0a2e/docs/arm/rpi/BCM2835-Memory-Barriers.rst
 * Ref. Xinu memory barreir function https://github.com/xinu-os/xinu/blob/4384915cc63a9c057418015c996f44372eec0a2e/system/arch/arm/memory_barrier.S
 */
void dmb(void) {
    asm("mov r12, #0");
    // Move to Coprocessor from ARM core register(MCR).
    // Syntax MCR <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>, <opcode_2>
    // P15: (Coprocessor 15. System control coprocessor.)
    // 0,5: Opcode 1/2 
    // c7,c10: Coprocessor register.
    // c7, c10, 0(opcode1), 5(opcode2)
    // r12: Rd. SBZ(Should be zero)
    // Ref. ARM Manual A8.8.98 
    // Ref. ARM Arm1176jzfs Manual Page 3-84
    asm("mcr p15,  0, r12, c7, c10, 5");
    asm("mov pc, lr");
}

/**
 * Called when irq happend.
 */
void master_isr(void) {
    // When CPU into IRQ mode, the Link Register(R14) will have value PC+4, 
    // where PC is the address of the instruction that was NOT executed because 
    // the IRQ took priority. In order to return to the right address(PC), 
    // we need to minus R14 by 4 
    asm("sub lr, lr, #4");
    // Save CPSR register and link register 
    // SRS (Store Return State) stores the LR and SPSR of the current mode to 
    // the stack in SYS mode. (0x1f) indicate the SYS mode. "db" (Decrement 
    // Before) suffix means CPU will decrement the stack pointer before storing 
    // values onto stack. "!" means after store R14 and SPSR of the IRQ mode 
    // into SYS mode stack, update the stack pointer in SYS mode. (Ref. ARM 
    // Manual B9.3.16)
    asm("srsdb #0x1f!"); 
    // Change Processor State(CPS) Change CPSR to SYS mode, "i" means disable 
    // IRQ. "0x1f" means SYSTEM mode. 
    asm("cpsid i, #0x1f");
    // Save registers
    asm("push {r0-r12, r14}");
    // Save stack pointer
    asm("mov %[old_sp], %%sp" : [old_sp] "=r"(active_proc->sp) :);
    // Handle IRQs 
    asm("bl irq_handler");
    // Memory barrier
    asm("bl dmb");
    // Call dispatcher to find next process
    asm("bl dispatcher_impl");
    asm("bl dmb");
    // Restore stack pointer
    asm("mov %%sp, %[new_sp]" : : [new_sp] "r"(active_proc->sp));
    // Restore registers
    asm("pop {r0-r12, r14}");
    // RFE (Return From Exception) loads LR, SPSR on SYS stack into PC and CPSR 
    // registers, "ia" (increase after) means increment stack pointer after 
    // read from stack. "!" means update value in sp after instruction. (Ref. 
    // ARM Manual B9.3.13)
    asm("rfeia sp!");
}

/**
 * Resign until wake up by irq.
 * 
 * @param intr_no
 */
void wait_for_interrupt(int intr_no) {
    volatile unsigned int flag;

    SAVE_CPSR_DIS_IRQ(flag);
    /*
    if (!(intr_no == TIMER_IRQ || intr_no == COM1_IRQ || intr_no == KEYB_IRQ)) {
          panic("Interrupt must be TIMER_IRQ or COM1_IRQ or KEYB_IRQ");
    }*/
    // Process became STATE_INTR_BLOCKED
    active_proc->state = STATE_INTR_BLOCKED;

    remove_ready_queue(active_proc);
    if (interrupt_table[intr_no] != NULL) {
        panic("ISR already used!");
    }
    interrupt_table[intr_no] = active_proc;
    // Remove process from ready queue
    resign();
    interrupt_table[intr_no] = NULL;
    RESUME_CPSR(flag);
}


/**
 * Initialize Interrupts 
 */
void init_interrupts(void) {
    int i;

    for (i = 0; i < MAX_INTERRUPTS; i++) {
        isr_table[i] = NULL;
        interrupt_table[i] = NULL;
    }

    init_timer();
//    if (intr_usb) {
//        init_usb_interrupts();
//    }
    // Enable IRQ
    // Move to Register from Special register(MRS) move cpsr to r0
    asm("mrs r0, cpsr");
    // We can not change CPSR directly, need to change it in r0 first then store
    // it back.
    // Bitwise Bit Clear (BIC) Set bit 7 to 0 to enable IRQ
    asm("bic r0, r0, #0x80");
    asm("msr cpsr_c, r0");
}
