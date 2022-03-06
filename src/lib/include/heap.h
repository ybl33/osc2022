#ifndef __HEAP__H__
#define __HEAP__H__
#include "stddef.h"
#include "string.h"
void *malloc(size_t n);
void free(void *m);
void *calloc(size_t n, size_t sz);
#endif