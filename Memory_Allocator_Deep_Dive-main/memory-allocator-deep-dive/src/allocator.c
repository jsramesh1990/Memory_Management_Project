/**
 * @file allocator.c
 * @brief Main allocator implementation with first-fit algorithm
 */

#include "allocator.h"
#include "allocator_config.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#if ALLOCATOR_DEBUG >= 1
#include <stdio.h>
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/* ======================== Block Header Structure ======================== */

typedef struct block {
    size_t size;                /* Size including this header */
    struct block* next;         /* Next in free list */
    struct block* prev;         /* Previous in free list */
    uint32_t magic;             /* Magic number for corruption detection */
    uint8_t used;               /* 1 if allocated, 0 if free */
    uint8_t padding[3];         /* Align to 8 bytes */
} block_t;

/* Static assertions to verify structure size */
_Static_assert(sizeof(block_t) == 32, "block_t size must be 32 bytes");
_Static_assert(offsetof(block_t, size) == 0, "size must be at offset 0");
_Static_assert(offsetof(block_t, magic) == 24, "magic must be at offset 24");
_Static_assert(offsetof(block_t, used) == 28, "used must be at offset 28");

/* ======================== Global State ======================== */

static block_t* free_list = NULL;
static void* heap_start = NULL;
static void* heap_end = NULL;
static allocator_error_handler_t error_handler = NULL;

/* Statistics */
static allocator_stats_t stats = {0};

/* Magic numbers */
#define MAGIC_ALLOCATED 0xDEADBEEF
#define MAGIC_FREE      0xCAFEBABE
#define MAGIC_POISON    0xDEADBEEF

/* ======================== Helper Functions ======================== */

static ALLOCATOR_ALWAYS_INLINE size_t align_size(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

static ALLOCATOR_ALWAYS_INLINE size_t align_to_ptr(size_t size) {
    return align_size(size, sizeof(void*));
}

static void report_error(const char* msg, void* ptr) {
    if (error_handler) {
        error_handler(msg, ptr);
    } else {
#if ALLOCATOR_DEBUG >= 1
        fprintf(stderr, "allocator error: %s (ptr=%p)\n", msg, ptr);
#else
        (void)msg;
        (void)ptr;
#endif
    }
}

/* ======================== Block Management ======================== */

static block_t* split_block(block_t* block, size_t size) {
    size_t remaining = block->size - size;
    
    if (remaining < ALLOCATOR_SPLIT_THRESHOLD) {
        return NULL;
    }
    
    /* Create new block after current */
    block_t* new_block = (block_t*)((char*)block + size);
    new_block->size = remaining;
    new_block->used = 0;
    new_block->magic = MAGIC_FREE;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
    
    stats.coalesce_count++;
    return new_block;
}

static block_t* coalesce_blocks(block_t* block) {
    if (!block->used) {
        /* Coalesce with next if free */
        if (block->next && !block->next->used) {
            block->size += block->next->size;
            block->next = block->next->next;
            if (block->next) {
                block->next->prev = block;
            }
            stats.coalesce_count++;
        }
        
        /* Coalesce with previous if free */
        if (block->prev && !block->prev->used) {
            block->prev->size += block->size;
            block->prev->next = block->next;
            if (block->next) {
                block->next->prev = block->prev;
            }
            block = block->prev;
            stats.coalesce_count++;
        }
    }
    return block;
}

/* ======================== Allocation Strategies ======================== */

#if ALLOCATOR_STRATEGY == 0  /* First-fit */

static block_t* find_free_block(size_t size) {
    block_t* current = free_list;
    
    while (current) {
        if (!current->used && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#elif ALLOCATOR_STRATEGY == 1  /* Best-fit */

static block_t* find_free_block(size_t size) {
    block_t* current = free_list;
    block_t* best = NULL;
    size_t best_size = ~0;
    
    while (current) {
        if (!current->used && current->size >= size) {
            if (current->size < best_size) {
                best = current;
                best_size = current->size;
                if (best_size == size) break;  /* Perfect fit */
            }
        }
        current = current->next;
    }
    return best;
}

#elif ALLOCATOR_STRATEGY == 2  /* Next-fit */

static block_t* last_allocated = NULL;

static block_t* find_free_block(size_t size) {
    block_t* current = last_allocated ? last_allocated : free_list;
    block_t* start = current;
    
    do {
        if (!current->used && current->size >= size) {
            last_allocated = current;
            return current;
        }
        current = current->next;
        if (!current) current = free_list;
    } while (current != start);
    
    return NULL;
}

#else  /* Worst-fit */

static block_t* find_free_block(size_t size) {
    block_t* current = free_list;
    block_t* worst = NULL;
    size_t worst_size = 0;
    
    while (current) {
        if (!current->used && current->size >= size) {
            if (current->size > worst_size) {
                worst = current;
                worst_size = current->size;
            }
        }
        current = current->next;
    }
    return worst;
}

#endif

/* ======================== Core API Implementation ======================== */

bool allocator_init(void* heap_start_addr, size_t heap_size) {
    if (!heap_start_addr || heap_size < sizeof(block_t)) {
        return false;
    }
    
    /* Align heap start to pointer boundary */
    uintptr_t start_aligned = (uintptr_t)heap_start_addr;
    start_aligned = align_size(start_aligned, sizeof(void*));
    
    heap_start = (void*)start_aligned;
    heap_end = (void*)((char*)heap_start + heap_size - (start_aligned - (uintptr_t)heap_start_addr));
    
    /* Create initial free block */
    free_list = (block_t*)heap_start;
    free_list->size = (size_t)((char*)heap_end - (char*)heap_start);
    free_list->used = 0;
    free_list->magic = MAGIC_FREE;
    free_list->next = NULL;
    free_list->prev = NULL;
    
    /* Initialize statistics */
    memset(&stats, 0, sizeof(stats));
    stats.total_memory = free_list->size;
    stats.free_memory = free_list->size;
    stats.fragmentation_count = 1;
    stats.smallest_allocation = (size_t)-1;
    
    DEBUG_PRINT("allocator: initialized with %zu bytes at %p\n", free_list->size, heap_start);
    
    return true;
}

void* allocator_malloc(size_t size) {
    if (size == 0) return NULL;
    
    ALLOCATOR_LOCK();
    
    /* Align size to store header properly */
    size_t total_size = size + sizeof(block_t);
    total_size = align_to_ptr(total_size);
    
    if (total_size > stats.total_memory) {
        ALLOCATOR_UNLOCK();
        report_error("allocation size exceeds heap", NULL);
        return NULL;
    }
    
    /* Find free block */
    block_t* block = find_free_block(total_size);
    if (!block) {
        DEBUG_PRINT("allocator: out of memory for %zu bytes\n", size);
        stats.allocation_count++;
        ALLOCATOR_UNLOCK();
        return NULL;
    }
    
    /* Split if possible */
    split_block(block, total_size);
    
    /* Mark as used */
    block->used = 1;
    block->magic = MAGIC_ALLOCATED;
    
    /* Update statistics */
    stats.allocated_memory += block->size;
    stats.free_memory -= block->size;
    stats.allocation_count++;
    stats.fragmentation_count = stats.free_memory / ALLOCATOR_MIN_BLOCK_SIZE;
    
    if (size < stats.smallest_allocation) stats.smallest_allocation = size;
    if (size > stats.largest_allocation) stats.largest_allocation = size;
    
    void* user_ptr = (void*)((char*)block + sizeof(block_t));
    
    DEBUG_PRINT("allocator: allocated %zu bytes at %p (block %p, size %zu)\n", 
                size, user_ptr, block, block->size);
    
    ALLOCATOR_UNLOCK();
    return user_ptr;
}

void* allocator_calloc(size_t count, size_t size) {
    size_t total = count * size;
    void* ptr = allocator_malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* allocator_realloc(void* ptr, size_t new_size) {
    if (!ptr) return allocator_malloc(new_size);
    if (new_size == 0) {
        allocator_free(ptr);
        return NULL;
    }
    
    /* Get current block size */
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    if (block->magic != MAGIC_ALLOCATED) {
        report_error("realloc: invalid pointer", ptr);
        return NULL;
    }
    
    size_t old_size = block->size - sizeof(block_t);
    
    /* If new size fits, return same pointer */
    if (new_size <= old_size) {
        return ptr;
    }
    
    /* Otherwise allocate new block and copy */
    void* new_ptr = allocator_malloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        allocator_free(ptr);
    }
    
    return new_ptr;
}

void allocator_free(void* ptr) {
    if (!ptr) return;
    
    ALLOCATOR_LOCK();
    
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    
    /* Validate magic number */
    if (block->magic != MAGIC_ALLOCATED) {
        ALLOCATOR_UNLOCK();
        report_error("double free or corruption", ptr);
        return;
    }
    
    DEBUG_PRINT("allocator: freeing %p (block %p, size %zu)\n", ptr, block, block->size);
    
    /* Poison memory to catch use-after-free */
#if ALLOCATOR_POISON_ON_FREE
    memset(ptr, 0xDE, block->size - sizeof(block_t));
#endif
    
    /* Mark as free */
    block->used = 0;
    block->magic = MAGIC_FREE;
    
    /* Update statistics */
    stats.allocated_memory -= block->size;
    stats.free_memory += block->size;
    stats.free_count++;
    
    /* Coalesce with adjacent free blocks */
    block = coalesce_blocks(block);
    
    /* Insert into free list (maintain address order for better coalescing) */
    if (block != free_list && block < free_list) {
        block->next = free_list;
        free_list->prev = block;
        free_list = block;
    }
    
    stats.fragmentation_count = stats.free_memory / ALLOCATOR_MIN_BLOCK_SIZE;
    
    ALLOCATOR_UNLOCK();
}

void* allocator_aligned_alloc(size_t alignment, size_t size) {
    if (alignment < ALLOCATOR_ALIGNMENT) alignment = ALLOCATOR_ALIGNMENT;
    
    /* Allocate extra space for alignment and header */
    size_t total_size = size + alignment + sizeof(block_t);
    void* raw_ptr = allocator_malloc(total_size);
    if (!raw_ptr) return NULL;
    
    /* Align the pointer */
    uintptr_t raw_addr = (uintptr_t)raw_ptr;
    uintptr_t aligned_addr = (raw_addr + alignment - 1) & ~(alignment - 1);
    
    /* Store original pointer before aligned region */
    void** original_ptr = (void**)(aligned_addr - sizeof(void*));
    *original_ptr = raw_ptr;
    
    return (void*)aligned_addr;
}

void allocator_aligned_free(void* ptr) {
    if (!ptr) return;
    
    /* Retrieve original pointer */
    void** original_ptr = (void**)((uintptr_t)ptr - sizeof(void*));
    allocator_free(*original_ptr);
}

void allocator_get_stats(allocator_stats_t* out_stats) {
    if (out_stats) {
        memcpy(out_stats, &stats, sizeof(allocator_stats_t));
    }
}

bool allocator_reset(void) {
    if (stats.allocated_memory != 0) {
        report_error("reset failed: memory still allocated", NULL);
        return false;
    }
    
    return allocator_init(heap_start, (size_t)((char*)heap_end - (char*)heap_start));
}

bool allocator_validate(void) {
    block_t* current = free_list;
    unsigned long block_count = 0;
    
    while (current) {
        block_count++;
        
        /* Check magic numbers */
        if (current->magic != MAGIC_ALLOCATED && current->magic != MAGIC_FREE) {
            report_error("corrupted magic number", current);
            return false;
        }
        
        /* Check block within heap bounds */
        if ((void*)current < heap_start || (void*)current >= heap_end) {
            report_error("block outside heap bounds", current);
            return false;
        }
        
        /* Check size validity */
        if (current->size < sizeof(block_t) || current->size > stats.total_memory) {
            report_error("invalid block size", current);
            return false;
        }
        
        current = current->next;
    }
    
    return true;
}

size_t allocator_get_free_memory(void) {
    return stats.free_memory;
}

size_t allocator_get_allocated_memory(void) {
    return stats.allocated_memory;
}

void allocator_dump_heap(void) {
#if ALLOCATOR_DEBUG >= 1
    block_t* current = free_list;
    int block_num = 0;
    
    printf("\n=== Heap Dump ===\n");
    printf("Total memory: %zu bytes\n", stats.total_memory);
    printf("Allocated: %zu bytes\n", stats.allocated_memory);
    printf("Free: %zu bytes\n", stats.free_memory);
    printf("Fragmentation: %zu blocks\n\n", stats.fragmentation_count);
    
    while (current) {
        printf("Block %d: addr=%p, size=%zu, %s, magic=0x%08X\n",
               block_num++, current, current->size,
               current->used ? "ALLOCATED" : "FREE",
               current->magic);
        current = current->next;
    }
    printf("================\n");
#endif
}

void allocator_set_error_handler(allocator_error_handler_t handler) {
    error_handler = handler;
}
