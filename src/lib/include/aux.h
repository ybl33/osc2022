/* ---------------------------------------------------------------------------
| Address(BUS) | Field Name         |         Description          | Bit Size |
| ------------ | ------------------ |:----------------------------:|:--------:|
| 0x7E215000   | AUX_IRQ            |  Auxiliary Interrupt status  |    3     |
| 0x7E215004   | AUX_ENABLES        |      Auxiliary enables       |    3     |
| 0x7E215040   | AUX_MU_IO_REG      |      Mini Uart I/O Data      |    8     |
| 0x7E215044   | AUX_MU_IER_REG     |  Mini Uart Interrupt Enable  |    8     |
| 0x7E215048   | AUX_MU_IIR_REG     | Mini Uart Interrupt Identify |    8     |
| 0x7E21504C   | AUX_MU_LCR_REG     |    Mini Uart Line Control    |    8     |
| 0x7E215050   | AUX_MU_MCR_REG     |   Mini Uart Modem Control    |    8     |
| 0x7E215054   | AUX_MU_LSR_REG     |    Mini Uart Line Status     |    8     |
| 0x7E215058   | AUX_MU_MSR_REG     |    Mini Uart Modem Status    |    8     |
| 0x7E21505C   | AUX_MU_SCRATCH     |      Mini Uart Scratch       |    8     |
| 0x7E215060   | AUX_MU_CNTL_REG    |   Mini Uart Extra Control    |    8     |
| 0x7E215064   | AUX_MU_STAT_REG    |    Mini Uart Extra Status    |    32    |
| 0x7E215068   | AUX_MU_BAUD_REG    |      Mini Uart Baudrate      |    16    |
| 0x7E215080   | AUX_SPI0_CNTL0_REG |   SPI 1 Control register 0   |    32    |
| 0x7E215084   | AUX_SPI0_CNTL1_REG |   SPI 1 Control register 1   |    8     |
| 0x7E215088   | AUX_SPI0_STAT_REG  |         SPI 1 Status         |    32    |
| 0x7E215090   | AUX_SPI0_IO_REG    |          SPI 1 Data          |    32    |
| 0x7E215094   | AUX_SPI0_PEEK_REG  |          SPI 1 Peek          |    16    |
| 0x7E2150C0   | AUX_SPI1_CNTL0_REG |   SPI 2 Control register 0   |    32    |
| 0x7E2150C4   | AUX_SPI1_CNTL1_REG |   SPI 2 Control register 1   |    8     |
| 0x7E2150C8   | AUX_SPI1_STAT_REG  |         SPI 2 Status         |    32    |
| 0x7E2150D0   | AUX_SPI1_IO_REG    |          SPI 2 Data          |    32    |
| 0x7E2150D4   | AUX_SPI1_PEEK_REG  |          SPI 2 Peek          |    16    |
 ---------------------------------------------------------------------------- */
#ifndef __AUX__H__
#define __AUX__H__
#include "mmio.h"
#define AUX_BASE           (MMIO_BASE + 0x00215000)
#define AUX_IRQ            (AUX_BASE  + 0x00)
#define AUX_ENABLES        (AUX_BASE  + 0x04)
#define AUX_MU_IO_REG      (AUX_BASE  + 0x40)
#define AUX_MU_IER_REG     (AUX_BASE  + 0x44)
#define AUX_MU_IIR_REG     (AUX_BASE  + 0x48)
#define AUX_MU_LCR_REG     (AUX_BASE  + 0x4C)
#define AUX_MU_MCR_REG     (AUX_BASE  + 0x50)
#define AUX_MU_LSR_REG     (AUX_BASE  + 0x54)
#define AUX_MU_MSR_REG     (AUX_BASE  + 0x58)
#define AUX_MU_SCRATCH     (AUX_BASE  + 0x5C)
#define AUX_MU_CNTL_REG    (AUX_BASE  + 0x60)
#define AUX_MU_STAT_REG    (AUX_BASE  + 0x64)
#define AUX_MU_BAUD_REG    (AUX_BASE  + 0x68)
#define AUX_SPI0_CNTL0_REG (AUX_BASE  + 0x80)
#define AUX_SPI0_CNTL1_REG (AUX_BASE  + 0x84)
#define AUX_SPI0_STAT_REG  (AUX_BASE  + 0x88)
#define AUX_SPI0_IO_REG    (AUX_BASE  + 0x90)
#define AUX_SPI0_PEEK_REG  (AUX_BASE  + 0x94)
#define AUX_SPI1_CNTL0_REG (AUX_BASE  + 0xC0)
#define AUX_SPI1_CNTL1_REG (AUX_BASE  + 0xC4)
#define AUX_SPI1_STAT_REG  (AUX_BASE  + 0xC8)
#define AUX_SPI1_IO_REG    (AUX_BASE  + 0xD0)
#define AUX_SPI1_PEEK_REG  (AUX_BASE  + 0xD4)
#endif