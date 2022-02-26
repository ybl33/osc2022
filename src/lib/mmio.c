#include "mmio.h"

void mmio_put (long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

unsigned int mmio_get (long addr) {
    volatile unsigned int* point = (unsigned int*)addr;
    return *point;
}