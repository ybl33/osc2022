/* ----------------------------------------------------------------------------------
| Address(BUS) | Field Name |              Description              | Bit Size | R/W |
| ------------ | ---------- |:-------------------------------------:| -------- |:---:|
| 0x7E200000   | GPFSEL0    |        GPIO Function Select 0         | 32       | R/W |
| 0x7E200004   | GPFSEL1    |        GPIO Function Select 1         | 32       | R/W |
| 0x7E200008   | GPFSEL2    |        GPIO Function Select 2         | 32       | R/W |
| 0x7E20000C   | GPFSEL3    |        GPIO Function Select 3         | 32       | R/W |
| 0x7E200010   | GPFSEL4    |        GPIO Function Select 4         | 32       | R/W |
| 0x7E200014   | GPFSEL5    |        GPIO Function Select 5         | 32       | R/W |
| 0x7E200018   | -          |               Reserved                | -        |  -  |
| 0x7E20001C   | GPSET0     |         GPIO Pin Output Set 0         | 32       |  W  |
| 0x7E200020   | GPSET1     |         GPIO Pin Output Set 1         | 32       |  W  |
| 0x7E200024   | -          |               Reserved                | -        |  -  |
| 0x7E200028   | GPCLR0     |        GPIO Pin Output Clear 0        | 32       |  W  |
| 0x7E20002C   | GPCLR1     |        GPIO Pin Output Clear 1        | 32       |  W  |
| 0x7E200030   | -          |               Reserved                | -        |  -  |
| 0x7E200034   | GPLEV0     |           GPIO Pin Level 0            | 32       |  R  |
| 0x7E200038   | GPLEV1     |           GPIO Pin Level 1            | 32       |  R  |
| 0x7E20003C   | -          |               Reserved                | -        |  -  |
| 0x7E200040   | GPEDS0     |    GPIO Pin Event Detect Status 0     | 32       | R/W |
| 0x7E200044   | GPEDS1     |    GPIO Pin Event Detect Status 1     | 32       | R/W |
| 0x7E200048   | -          |               Reserved                | -        |  -  |
| 0x7E20004C   | GPREN0     | GPIO Pin Rising Edge Detect Enable 0  | 32       | R/W |
| 0x7E200050   | GPREN1     | GPIO Pin Rising Edge Detect Enable 1  | 32       | R/W |
| 0x7E200054   | -          |               Reserved                | -        |  -  |
| 0x7E200058   | GPFEN0     | GPIO Pin Falling Edge Detect Enable 0 | 32       | R/W |
| 0x7E20005C   | GPFEN1     | GPIO Pin Falling Edge Detect Enable 1 | 32       | R/W |
| 0x7E200060   | -          |               Reserved                | -        |  -  |
| 0x7E200064   | GPHEN0     |     GPIO Pin High Detect Enable 0     | 32       | R/W |
| 0x7E200068   | GPHEN1     |     GPIO Pin High Detect Enable 1     | 32       | R/W |
| 0x7E20006C   | -          |               Reserved                | -        |  -  |
| 0x7E200070   | GPLEN0     |     GPIO Pin Low Detect Enable 0      | 32       | R/W |
| 0x7E200074   | GPLEN1     |     GPIO Pin Low Detect Enable 1      | 32       | R/W |
| 0x7E200078   | -          |               Reserved                | -        |  -  |
| 0x7E20007C   | GPAREN0    | GPIO Pin Async. Rising Edge Detect 0  | 32       | R/W |
| 0x7E200080   | GPAREN1    | GPIO Pin Async. Rising Edge Detect 1  | 32       | R/W |
| 0x7E200084   | -          |               Reserved                | -        |  -  |
| 0x7E200088   | GPAFEN0    | GPIO Pin Async. Falling Edge Detect 0 | 32       | R/W |
| 0x7E20008C   | GPAFEN1    | GPIO Pin Async. Falling Edge Detect 1 | 32       | R/W |
| 0x7E200090   | -          |               Reserved                | -        |  -  |
| 0x7E200094   | GPPUD      |     GPIO Pin Pull-up/down Enable      | 32       | R/W |
| 0x7E200098   | GPPUDCLK0  | GPIO Pin Pull-up/down Enable Clock 0  | 32       | R/W |
| 0x7E20009C   | GPPUDCLK1  | GPIO Pin Pull-up/down Enable Clock 1  | 32       | R/W |
| 0x7E2000A0   | -          |               Reserved                | -        |  -  |
| 0x7E2000B0   | -          |                 Test                  | 4        | R/W |
 ------------------------------------------------------------------------------------ */
// GPIO Registers 
#ifndef __GPIO__H__
#define __GPIO__H__
#include "mmio.h"
#define GPIO_BASE (MMIO_BASE + 0x00200000)
#define GPFSEL0   (GPIO_BASE + 0x00)
#define GPFSEL1   (GPIO_BASE + 0x04)
#define GPFSEL2   (GPIO_BASE + 0x08)
#define GPFSEL3   (GPIO_BASE + 0x0C)
#define GPFSEL4   (GPIO_BASE + 0x10)
#define GPFSEL5   (GPIO_BASE + 0x14)
#define GPSET0    (GPIO_BASE + 0x1C)
#define GPSET1    (GPIO_BASE + 0x20)
#define GPCLR0    (GPIO_BASE + 0x28)
#define GPCLR1    (GPIO_BASE + 0x2C)
#define GPLEV0    (GPIO_BASE + 0x34)
#define GPLEV1    (GPIO_BASE + 0x38)
#define GPEDS0    (GPIO_BASE + 0x40)
#define GPEDS1    (GPIO_BASE + 0x44)
#define GPREN0    (GPIO_BASE + 0x4C)
#define GPREN1    (GPIO_BASE + 0x50)
#define GPFEN0    (GPIO_BASE + 0x58)
#define GPFEN1    (GPIO_BASE + 0x5C)
#define GPHEN0    (GPIO_BASE + 0x64)
#define GPHEN1    (GPIO_BASE + 0x68)
#define GPLEN0    (GPIO_BASE + 0x70)
#define GPLEN1    (GPIO_BASE + 0x74)
#define GPAREN0   (GPIO_BASE + 0x7C)
#define GPAREN1   (GPIO_BASE + 0x80)
#define GPAFEN0   (GPIO_BASE + 0x88)
#define GPAFEN1   (GPIO_BASE + 0x8C)
#define GPPUD     (GPIO_BASE + 0x94)
#define GPPUDCLK0 (GPIO_BASE + 0x98)
#define GPPUDCLK1 (GPIO_BASE + 0x9C)

#define GPIO14_GPFSEL_OFFSET   12
#define GPIO15_GPFSEL_OFFSET   15
#define GPIO14_GPPUDCLK_OFFSET 14
#define GPIO15_GPPUDCLK_OFFSET 15
#endif