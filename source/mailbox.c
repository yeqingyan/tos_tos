#include <kernel.h>
/*
 * GetMailboxBase
 * --------------
 *  return mailbox base address
 */
int GetMailboxBase(){
        return 0x2000B880;
}

/*
 * MailboxWrite
 * ------------
 * Write to mailbox
 *
 * Parameters
 * content: message content, only use top 28 bits, lowest 4 bits should be 0
 * channel: mailbox using 4 bits.
 */
void MailboxWrite(int content, int channel) {
        if (content & 0b1111) {   /* Make sure lowest 4 bits in r0 are all 0 */
                goto ERROR;
        
        }
        if (channel > 15)       /* Make sure mailbox is in 4 bits */
                goto ERROR;

        int* mailboxBase = (int*) GetMailboxBase();
        int status;

        /* Write wait */
        do {
                status = *(mailboxBase + 6);    /* Status address is 0x2000B898 */
        } while(status & 0x80000000);           /* Check top bit is 0 */

        content = content | channel;

        *(mailboxBase + 8) = content;           /* Send address is 0x2000B8A0 */
ERROR:
        return;
}

/*
 * MailboxRead
 * -----------
 * Function to read from mailbox
 *
 * Parameters:
 * channel: mailbox, only using 4 bits.
 *
 * Return:
 * read message
 */
int MailboxRead(int channel) {
        if (channel > 15)       /* Make sure mailbox is in 4 bits */
                goto ERROR;

        int* mailboxBase = (int*) GetMailboxBase();
        int status, content, readChannel;

        while(1) {
                /* Write wait */
                do {
                        status = *(mailboxBase + 6);    /* Status address is 0x2000B898 */
                } while(status & 0x80000000);           /* Check top bit is 0 */

                content = *(mailboxBase);
                readChannel = content & 0b1111;         /* Get channel information in msg */

                if (readChannel == channel) {    /* Make sure channel is match */
                        //debug();
                        break;
                }
        }
        return content & 0xFFFFFFF0;            /* Remove mail channel information in msg */
ERROR:
        return 0;

}
