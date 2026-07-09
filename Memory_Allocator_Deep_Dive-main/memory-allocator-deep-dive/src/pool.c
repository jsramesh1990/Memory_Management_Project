/**
 * @file pool.c
 * @brief Fixed-block pool allocator implementation
 */

#include "pool.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#if defined(POOL_THREAD_SAFE) && !defined(__embedded__)
#include <pthread.h>
#define POOL_LOCK(pool)   pthread_mutex_lock(&(pool)->mutex)
#define POOL_UNLOCK(pool) pthread_mutex_unlock(&(pool)->mutex)
#else
#define POOL_LOCK(pool)
#define POOL_UNLOCK(pool)
#endif

/* Block metadata */
typedef struct block_meta {
    struct block_meta* next;
    uint32_t magic;
} block_meta_t;

struct pool_allocator {
    uint8_t* data;              /* Raw block data */
    block_meta_t* free_list;    /* Linked list of free blocks */
    block_meta_t* free_list_cache; /* Cached free list head */
    size_t block_size;          /* Size of each block (with alignment) */
    size_t block_count;         /* Total blocks */
    size_t free_count;          /* Currently free blocks */
    
    /* Statistics */
    unsigned long allocations;
    unsigned long frees;
    unsigned long failures;
    unsigned long cache_hits;
    unsigned long cache_misses;
    
#if defined(POOL_THREAD_SAFE) && !defined(__embedded__)
    pthread_mutex_t mutex;
#endif
    
    uint32_t magic_alloc;
};

#define POOL_MAGIC_ALLOC 0xA11C3A11
#define POOL_MAGIC_FREE  0xF4E5F4E5
#define POOL_MAGIC_VALID 0xDEADBEEF

static inline size_t align_size(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

pool_alloc_t* pool_create(size_t block_size, size_t block_count, void* memory) {
    if (block_size == 0 || block_count == 0 || block_count > POOL_MAX_BLOCKS) {
        return NULL;
    }
    
    /* Align block size */
    size_t aligned_block_size = align_size(block_size, POOL_ALIGNMENT);
    
    /* Calculate memory needs */
    size_t total_data_size = aligned_block_size * block_count;
    size_t total_meta_size = sizeof(block_meta_t) * block_count;
    size_t total_size = total_data_size + total_meta_size + sizeof(pool_alloc_t);
    
    pool_alloc_t* pool;
    uint8_t* memory_base;
    
    if (memory) {
        /* Use user-provided memory (static allocation) */
        memory_base = (uint8_t*)memory;
        pool = (pool_alloc_t*)memory_base;
        memory_base += sizeof(pool_alloc_t);
    } else {
        /* Dynamic allocation */
        pool = (pool_alloc_t*)malloc(total_size);
        if (!pool) return NULL;
        memory_base = (uint8_t*)pool;
    }
    
    /* Initialize pool structure */
    memset(pool, 0, sizeof(pool_alloc_t));
    pool->block_size = aligned_block_size;
    pool->block_count = block_count;
    pool->free_count = block_count;
    pool->magic_alloc = POOL_MAGIC_VALID;
    
    /* Split memory into metadata and data sections */
    block_meta_t* meta = (block_meta_t*)memory_base;
    pool->data = memory_base + total_meta_size;
    
    /* Initialize free list (linked in LIFO order for cache performance) */
    pool->free_list = NULL;
    for (size_t i = 0; i < block_count; i++) {
        meta[i].next = pool->free_list;
        meta[i].magic = POOL_MAGIC_FREE;
        pool->free_list = &meta[i];
    }
    pool->free_list_cache = pool->free_list;
    
#if defined(POOL_THREAD_SAFE) && !defined(__embedded__)
    pthread_mutex_init(&pool->mutex, NULL);
#endif
    
    return pool;
}

void pool_destroy(pool_alloc_t* pool) {
    if (!pool || pool->magic_alloc != POOL_MAGIC_VALID) return;
    
#if defined(POOL_THREAD_SAFE) && !defined(__embedded__)
    pthread_mutex_destroy(&pool->mutex);
#endif
    
    /* Only free if we allocated dynamically */
    /* Note: Can't easily know if memory was provided or allocated */
    /* For simplicity, we assume caller manages static memory */
    
    pool->magic_alloc = 0;
}

void* pool_alloc(pool_alloc_t* pool) {
    if (!pool || pool->magic_alloc != POOL_MAGIC_VALID) return NULL;
    
    POOL_LOCK(pool);
    
    /* Check cache first for faster access */
    if (pool->free_list_cache && pool->free_list_cache->magic == POOL_MAGIC_FREE) {
        block_meta_t* block = pool->free_list_cache;
        pool->free_list_cache = block->next;
        
        /* Update actual free list */
        if (pool->free_list == block) {
            pool->free_list = block->next;
        }
        
        block->magic = POOL_MAGIC_ALLOC;
        pool->free_count--;
        pool->allocations++;
        pool->cache_hits++;
        
        size_t block_index = block - (block_meta_t*)(pool->data - sizeof(block_meta_t) * pool->block_count);
        void* user_ptr = pool->data + (block_index * pool->block_size);
        
        POOL_UNLOCK(pool);
        return user_ptr;
    }
    
    /* No cached block, pop from free list */
    if (!pool->free_list) {
        pool->failures++;
        pool->cache_misses++;
        POOL_UNLOCK(pool);
        return NULL;
    }
    
    block_meta_t* block = pool->free_list;
    pool->free_list = block->next;
    pool->free_count--;
    pool->allocations++;
    pool->cache_misses++;
    
    /* Update cache */
    pool->free_list_cache = pool->free_list;
    
    /* Mark as allocated */
    block->magic = POOL_MAGIC_ALLOC;
    
    /* Calculate block index and return data pointer */
    size_t block_index = block - (block_meta_t*)(pool->data - sizeof(block_meta_t) * pool->block_count);
    void* user_ptr = pool->data + (block_index * pool->block_size);
    
    POOL_UNLOCK(pool);
    return user_ptr;
}

size_t pool_alloc_multiple(pool_alloc_t* pool, size_t count, void** out_ptrs) {
    if (!pool || !out_ptrs || count == 0) return 0;
    
    size_t allocated = 0;
    for (size_t i = 0; i < count; i++) {
        out_ptrs[i] = pool_alloc(pool);
        if (!out_ptrs[i]) break;
        allocated++;
    }
    
    return allocated;
}

bool pool_free(pool_alloc_t* pool, void* ptr) {
    if (!pool || !ptr || pool->magic_alloc != POOL_MAGIC_VALID) return false;
    
    POOL_LOCK(pool);
    
    /* Validate pointer belongs to this pool */
    uintptr_t ptr_addr = (uintptr_t)ptr;
    uintptr_t data_start = (uintptr_t)pool->data;
    uintptr_t data_end = data_start + (pool->block_size * pool->block_count);
    
    if (ptr_addr < data_start || ptr_addr >= data_end) {
        POOL_UNLOCK(pool);
        return false;
    }
    
    /* Calculate block index and metadata pointer */
    size_t block_index = (ptr_addr - data_start) / pool->block_size;
    block_meta_t* block = (block_meta_t*)(pool->data - sizeof(block_meta_t) * pool->block_count) + block_index;
    
    /* Validate magic number (detect double free) */
    if (block->magic != POOL_MAGIC_ALLOC) {
        POOL_UNLOCK(pool);
        return false;
    }
    
    /* Return to free list (LIFO for better cache performance) */
    block->magic = POOL_MAGIC_FREE;
    block->next = pool->free_list;
    pool->free_list = block;
    pool->free_count++;
    pool->frees++;
    
    /* Update cache if it's empty */
    if (!pool->free_list_cache) {
        pool->free_list_cache = block;
    }
    
    POOL_UNLOCK(pool);
    return true;
}

size_t pool_free_multiple(pool_alloc_t* pool, size_t count, void** ptrs) {
    if (!pool || !ptrs || count == 0) return 0;
    
    size_t freed = 0;
    for (size_t i = 0; i < count; i++) {
        if (pool_free(pool, ptrs[i])) {
            freed++;
        }
    }
    return freed;
}

void pool_stats(pool_alloc_t* pool, pool_stats_t* out_stats) {
    if (!pool || !out_stats || pool->magic_alloc != POOL_MAGIC_VALID) return;
    
    POOL_LOCK(pool);
    
    out_stats->total_blocks = pool->block_count;
    out_stats->block_size = pool->block_size;
    out_stats->free_blocks = pool->free_count;
    out_stats->allocated_blocks = pool->block_count - pool->free_count;
    out_stats->total_allocations = pool->allocations;
    out_stats->total_frees = pool->frees;
    out_stats->allocation_failures = pool->failures;
    out_stats->cache_hits = pool->cache_hits;
    out_stats->cache_misses = pool->cache_misses;
    
    POOL_UNLOCK(pool);
}

bool pool_reset(pool_alloc_t* pool) {
    if (!pool || pool->magic_alloc != POOL_MAGIC_VALID) return false;
    
    POOL_LOCK(pool);
    
    /* Check for memory leaks */
    if (pool->free_count != pool->block_count) {
        POOL_UNLOCK(pool);
        return false;
    }
    
    /* Reinitialize free list */
    block_meta_t* meta = (block_meta_t*)(pool->data - sizeof(block_meta_t) * pool->block_count);
    pool->free_list = NULL;
    for (size_t i = 0; i < pool->block_count; i++) {
        meta[i].next = pool->free_list;
        meta[i].magic = POOL_MAGIC_FREE;
        pool->free_list = &meta[i];
    }
    pool->free_list_cache = pool->free_list;
    pool->free_count = pool->block_count;
    
    /* Reset statistics (keep cumulative?) */
    pool->allocations = 0;
    pool->frees = 0;
    pool->failures = 0;
    pool->cache_hits = 0;
    pool->cache_misses = 0;
    
    POOL_UNLOCK(pool);
    return true;
}

bool pool_owns_pointer(pool_alloc_t* pool, void* ptr) {
    if (!pool || !ptr) return false;
    
    uintptr_t ptr_addr = (uintptr_t)ptr;
    uintptr_t data_start = (uintptr_t)pool->data;
    uintptr_t data_end = data_start + (pool->block_size * pool->block_count);
    
    return (ptr_addr >= data_start && ptr_addr < data_end);
}

size_t pool_block_size(pool_alloc_t* pool) {
    return pool ? pool->block_size : 0;
}

size_t pool_free_count(pool_alloc_t* pool) {
    return pool ? pool->free_count : 0;
}
