#include "mbox.h"

int mbox_call (mail_t *mbox, unsigned char ch) {
    
    unsigned int message;
    unsigned int *pmbox = (unsigned int *)user_va_to_pa(mbox);
    
    /* Wait for mbox not full */
    while (mmio_get(MBOX_STATUS) & MBOX_FULL) asm volatile ("nop");
    /* Write the address of message to the mbox with channel identifier */
    message = (((unsigned int)(unsigned long)pmbox) & ~0xF) | (ch & 0xF);
    mmio_put(MBOX_WRITE, message);
    /* Wait for our mbox response */
    do {

        while (mmio_get(MBOX_STATUS) & MBOX_EMPTY) asm volatile("nop");

    } while (mmio_get(MBOX_READ) != message);

    /* check response vaild */
    return mbox->header.code == MBOX_RESPONSE;
}