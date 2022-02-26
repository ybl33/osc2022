#include "mbox.h"

int mbox_call (mail_t *mbox, unsigned char ch) {

    unsigned int addr_channel;

    /* Wait for mbox not full */
    while (mmio_get(MBOX_STATUS) & MBOX_FULL) asm volatile ("nop");
    /* Write the address of message to the mbox with channel identifier */
    addr_channel = (((unsigned int)(unsigned long)mbox) & 0xFFFFFFF0) | (ch & 0xF);
    mmio_put(MBOX_WRITE, addr_channel);
    /* Wait for our mbox response */
    do {

        while (mmio_get(MBOX_STATUS) & MBOX_EMPTY) asm volatile("nop");

    } while (mmio_get(MBOX_READ) != addr_channel);

    /* check response vaild */
    return mbox->header.code == MBOX_RESPONSE;
}