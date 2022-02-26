#include "utils.h"

void get_board_revision (unsigned int* board_revision) {
    mail_t mbox;
    mbox.header.packet_size = MAIL_PACKET_SIZE;
    mbox.header.code        = REQUEST_CODE;

    mbox.body.identifier    = GET_BOARD_REVISION;
    mbox.body.buffer_size   = MAIL_BUF_SIZE;
    mbox.body.code          = TAG_REQUEST_CODE;
    mbox.body.end           = END_TAG;

    /* Clear data buffer */
    for (int i = 0; i < MAIL_BODY_BUF_LEN; i++) mbox.body.buffer[i] = 0;

    mbox_call(&mbox, MBOX_CH_PROP);

    // Should be 0xA020D3 for rpi3 b+
    *board_revision = mbox.body.buffer[0];
    return;
}

void get_board_serial(unsigned int* msb, unsigned int* lsb) {
    mail_t mbox;
    mbox.header.packet_size = MAIL_PACKET_SIZE;
    mbox.header.code        = REQUEST_CODE;

    mbox.body.identifier    = GET_BOARD_SERIAL;
    mbox.body.buffer_size   = MAIL_BUF_SIZE;
    mbox.body.code          = TAG_REQUEST_CODE;
    mbox.body.end           = END_TAG;

    /* Clear data buffer */
    for (int i = 0; i < MAIL_BODY_BUF_LEN; i++) mbox.body.buffer[i] = 0;
    
    if (mbox_call(&mbox, MBOX_CH_PROP)) 
    {
        *msb = mbox.body.buffer[1];
        *lsb = mbox.body.buffer[0];
    }
    else 
    {
        *msb = 0xFFFFFFFF;
        *lsb = 0xFFFFFFFF;
    }

    return;
}

void get_memory_info (unsigned int* mem_base, unsigned int* mem_size ) {
    mail_t mbox;
    mbox.header.packet_size = MAIL_PACKET_SIZE;
    mbox.header.code        = REQUEST_CODE;

    mbox.body.identifier    = GET_MEMORY_BASE;
    mbox.body.buffer_size   = MAIL_BUF_SIZE;
    mbox.body.code          = TAG_REQUEST_CODE;
    mbox.body.end           = END_TAG;

    /* Clear data buffer */
    for (int i = 0; i < MAIL_BODY_BUF_LEN; i++) mbox.body.buffer[i] = 0;

    if (mbox_call(&mbox, MBOX_CH_PROP)) 
    {
        *mem_size = mbox.body.buffer[1];
        *mem_base = mbox.body.buffer[0];
    } 
    else
    {
        *mem_size = 0xFFFFFFFF;
        *mem_base = 0xFFFFFFFF;
    } 

    return;
}

void reset(int tick) {                      // reboot after watchdog timer expire
    mmio_put(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    mmio_put(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
    return;
}

void cancel_reset() {
    mmio_put(PM_RSTC, PM_PASSWORD | 0);  // full reset
    mmio_put(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
    return;
}