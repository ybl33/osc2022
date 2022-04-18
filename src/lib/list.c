#include "list.h"
#include "uart.h"

void list_init (list_t *list) {

    list->prev = list;
    list->next = list;

    return;
}

void list_push (list_t *list, list_t *node) {

    list_t *prev = list->prev;

    node->prev = prev;
    node->next = list;
    prev->next = node;
    list->prev = node;

    return;
}

void list_remove (list_t *node) {

    list_t *prev = node->prev;
    list_t *next = node->next;

    prev->next = next;
    next->prev = prev;

    return;
}

list_t *list_pop (list_t *list) {

    list_t *node = list->next;

    if (list_empty(list))
    {
        /* Empty list */
        return NULL;
    }

    list_remove(node);

    return node;
}

bool list_empty(list_t *list) {

    if (list->next == list)
    {
        /* Empty list */
        return 1;
    }

    return 0;
}