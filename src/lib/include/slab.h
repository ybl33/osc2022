#ifndef __SLAB__H__
#define __SLAB__H__
#include "stddef.h"
#include "heap.h"
#include "buddy.h"
#include "list.h"
#include "log.h"

#define SLAB_ARCH_ALIGN (8)
#define SLAB_LOG_ON     (1)

struct kmem_cache_t;
struct kmem_slab_t;

typedef struct kmem_cache {

    size_t size;
    void (*ctor)(void *);
    list_t free_list;
    struct kmem_slab *slabs;
    
} kmem_cache_t;

typedef struct kmem_slab {

    void *page_addr;
    unsigned long inuse;
    struct kmem_cache *cache;
    struct kmem_slab  *next;

} kmem_slab_t;

kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align, unsigned long flags, void (*ctor)(void *));
void kmem_cache_grow(kmem_cache_t *cache);
void *kmem_cache_alloc(kmem_cache_t *cache, int flags);
void kmem_cache_free(kmem_cache_t *cache, void *object);
void kmalloc_init();
void *kmalloc(unsigned int size);
void kfree(void *object);

#endif