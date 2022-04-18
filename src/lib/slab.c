#include "slab.h"

#define KMALLOC_SIZE_NUM (7)
unsigned int   kmalloc_sizes[] = {16, 32, 48, 96, 128, 256, 512};
kmem_cache_t **kmalloc_caches;

void *_get_page (void *ptr) {
    
    void *page;

    page = (void *)((unsigned long)ptr / BUDDY_PAGE_SIZE * BUDDY_PAGE_SIZE);

    return page;
}

kmem_slab_t *_get_slab (void *ptr) {

    kmem_slab_t *slab;

    slab = (kmem_slab_t *)(_get_page(ptr) + BUDDY_PAGE_SIZE - sizeof(kmem_slab_t));

    return slab;
}

kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align, unsigned long flags, void (*ctor)(void *)) {

    kmem_cache_t *new_cache;

    if (size > BUDDY_PAGE_SIZE)
    {
        /* Not supported */
        return NULL;
    }

    new_cache = malloc(sizeof(kmem_cache_t));

    if (new_cache != NULL)
    {
        if (align == 0)
        {
            align = SLAB_ARCH_ALIGN;
        }

        new_cache->size  = align * ((size - 1) / align + 1);
        new_cache->ctor  = ctor;
        new_cache->slabs = NULL;
        list_init(&new_cache->free_list);
    }

    return new_cache;
}

void kmem_cache_grow (kmem_cache_t *cache) {
    
    unsigned int object_count;
    kmem_slab_t *new_slab;
    void *new_page;
    void *ptr;

    if (cache != NULL)
    {
        // Allocated one page
        new_page = alloc_pages(1);

        // The slab structure is at the end of the allocated pages
        new_slab = _get_slab(new_page);
        new_slab->page_addr = new_page;
        new_slab->inuse     = 0;
        new_slab->cache     = cache;
        new_slab->next      = NULL;

        object_count = (BUDDY_PAGE_SIZE - sizeof(kmem_slab_t)) / cache->size;
        ptr = new_page;

        for (int i = 0; i < object_count; i++)
        {
            list_push(&cache->free_list, (list_t *)ptr);
            ptr += cache->size;
        }

        if (cache->slabs == NULL)
        {
            cache->slabs = new_slab;
        }
        else
        {
            new_slab->next = cache->slabs;
            cache->slabs   = new_slab;
        }

        log_puts("[kmem cache grow] Allocated a page from buddy system, address: 0x", SLAB_LOG_ON);
        log_puth((unsigned long)new_page, SLAB_LOG_ON);
        log_puts(", object count: ", SLAB_LOG_ON);
        log_putu(object_count, SLAB_LOG_ON);
        log_puts(".\n", SLAB_LOG_ON);

    }

    return;
}

void *kmem_cache_alloc (kmem_cache_t *cache, int flags) {

    void *object;
    kmem_slab_t *slab;
    
    if (list_empty(&cache->free_list))
    {
        // No Free blocks
        kmem_cache_grow(cache);
    }

    object = list_pop(&cache->free_list);
    
    if (object != NULL)
    {
        // Find the slab structure, and increment the inuse count
        slab = _get_slab(object);
        slab->inuse += 1;

        log_puts("[kmem cache alloc] slab->inuse: ", SLAB_LOG_ON);
        log_putu(slab->inuse, SLAB_LOG_ON);
        log_puts(".\n", SLAB_LOG_ON);
    }

    return object;
}

void kmem_cache_free(kmem_cache_t *cache, void *object) {

    kmem_slab_t *slab;
    kmem_slab_t *curr_slab, *prev_slab;
    void *page;

    if (cache != NULL)
    {
        slab = _get_slab(object);
        slab->inuse -= 1;

        log_puts("[kmem cache free] slab->inuse: ", SLAB_LOG_ON);
        log_putu(slab->inuse, SLAB_LOG_ON);
        log_puts(".\n", SLAB_LOG_ON);

        if (slab->inuse == 0)
        {
            // Entire page is free, return it back
            page = _get_page(object);
            free_page(page);

            prev_slab = NULL;
            curr_slab = cache->slabs;

            // Find the position of slab 
            while (curr_slab != NULL)
            {
                if (curr_slab->page_addr == page)
                {
                    break;
                }

                prev_slab = curr_slab;
                curr_slab = curr_slab->next;
            }

            if (prev_slab == NULL)
            {
                // At head of list
                cache->slabs = curr_slab->next;
            }
            else
            {
                prev_slab->next = curr_slab->next;
            }
        }

    }

    return;
}

void kmalloc_init () {

    if (kmalloc_caches == NULL)
    {
        kmalloc_caches = malloc(sizeof(kmem_cache_t *) * KMALLOC_SIZE_NUM);
        
        for (int i = 0; i < KMALLOC_SIZE_NUM; i++)
        {
            kmalloc_caches[i] = kmem_cache_create(NULL, kmalloc_sizes[i], SLAB_ARCH_ALIGN, 0, NULL);
        }
    }

    return;
}

unsigned int _get_kmalloc_index (unsigned int size) {

    unsigned int index = 0;

    for (index = 0; index < KMALLOC_SIZE_NUM; index++)
    {
        if (kmalloc_sizes[index] >= size)
        {
            break;
        }
    }

    return index;
}

void *kmalloc (unsigned int size) {

    void *ptr = NULL;
    unsigned int index;

    if (kmalloc_caches == NULL)
    {
        kmalloc_init();
    }

    if (size != 0)
    {

        index = _get_kmalloc_index(size);

        if (index < KMALLOC_SIZE_NUM)
        {
            log_puts("[kmalloc] Found cache index: ", SLAB_LOG_ON);
            log_putu(index, SLAB_LOG_ON);
            log_puts(", cache size: ", SLAB_LOG_ON);
            log_putu(kmalloc_caches[index]->size, SLAB_LOG_ON);
            log_puts(".\n", SLAB_LOG_ON);

            ptr = kmem_cache_alloc(kmalloc_caches[index], 0);
        }

    }

    return ptr;
}

void kfree (void *object) {

    kmem_slab_t  *slab;
    kmem_cache_t *cache;

    slab = _get_slab(object);
    cache = slab->cache;
    
    kmem_cache_free(cache, object);

    return;
}