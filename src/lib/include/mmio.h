#ifndef __MMIO__H__
#define __MMIO__H__
#define PHY_BASE 0x3F000000
#define BUS_BASE 0x7E000000

#if USE_BUS
#define MMIO_BASE BUS_BASE
#else
#define MMIO_BASE PHY_BASE
#endif

void mmio_put(long addr, unsigned int value);
unsigned int mmio_get(long addr);
#endif