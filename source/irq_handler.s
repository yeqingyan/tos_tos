.global irq_handler

irq_handler:

    /* 
        According to ARM1176JZFS Manual 2.12.2 Exception entry and exit summary :
        When CPU into IRQ mode, the Link Register(R14) will have value PC+4, where PC is the address 
        of the instruction that was NOT executed because the FIQ took priority. In order to return to 
        the right address(PC), we need to minus R14 by 4 
    */ 
    sub lr, lr, #4
 	
    /*
        SRS instruction store R14 and SPSR of the IRQ mode into SYS_R13(Stack Pointer in SYS Mode), 
        0x1f mean SYS mode.
        db means addressing mode is Decrement Before, cpu will decrement the stack pointer before store
        value into stack. ! means after store R14 and SPSR of the IRQ mode into SYS mode stack, update the 
        stack pointer SYS_R13.
        
        Note: since IRQ interrupt mask is in SPSR, so we do not need to disalbe/enable irq here, since cpsid 
        instruction below will disable IRQ, and when we restore the SPSR to CPSR in rfeia, the interrupt will 
        be enabled.
    */
    srsdb #0x1f!
    
    /*
        CPS instruction change proces state to SYS mode(0x1f), id means interrupt disable.
        i means disable IRQ.
    */    
    cpsid i, #0x1f
    
    /* 
        According to ARM Procedure Call Standard, before call interrupt service routine(ISR) we need 
        to store caller-save general purpose register, this include r0-r3, r12.  
        
        Note, we do not need to push lr here, since lr have been pushed in srsdb instruction 
              We do not push r12(ip, Intra procedure call scratch register), since we should not use this in TOS */
    push {r0-r3}
    
    /* Through according to Procedure Call Standard for the ARM Architecture[AAPCS], eight byte stack alignment is a 
        requirement, but we do not do sp alignment, because when context swtich happened in isr_dispatcher function, when we back from isr_dispatcher, we do not know how to restore stack aligment */
    
    /* TODO Check memory barrier mentioned in BCM2835 */
    
    /* Call isr_dispatcher to handle interrupt */
    bl isr_dispatcher 
        
    /*
        Restore stored registers from SYS mode stack
    */
    pop {r0-r3}
    
    /*
        REF instruction load R14, SPSR on SYS stack into PC and CPSR register, ia means increment stack pointer
        after read from stack. ! means update value in sp after instruction.          
    */    
    rfeia sp!