#ifndef __LIST__H__
#define __LIST__H__
#include "stddef.h"

// Circular doubly linked list
typedef struct list_t {
    struct list_t *prev;
    struct list_t *next;
} list_t;

void list_init(list_t *list);
void list_push(list_t *list, list_t *node);
void list_remove(list_t *node); 
bool list_empty(list_t *list);
list_t *list_pop(list_t *list);

#endif