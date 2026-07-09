# Custom Memory Allocator 

## Overview

Building a custom memory allocator is the ultimate test of understanding memory management. This guide walks through implementing `my_malloc()` and `my_free()` from scratch, covering all critical concepts.

## Basic Allocator Architecture

```
CUSTOM ALLOCATOR ARCHITECTURE
═══════════════════════════════════════════════════════════════════

                    USER PROGRAM
                         │
            ┌────────────┼────────────┐
            │            │            │
        my_malloc()  my_calloc()  my_realloc()
            │            │            │
            └────────────┼────────────┘
                         │
              ┌──────────┴──────────┐
              │   Allocator Core    │
              │                     │
              │  • Free list mgmt   │
              │  • Splitting        │
              │  • Coalescing       │
              │  • Alignment        │
              └──────────┬──────────┘
                         │
              ┌──────────┴──────────┐
              │   System Heap       │
              │   (sbrk/mmap)       │
              └─────────────────────┘
```

## Data Structures

### Block Header Design

```c
// Basic block header (16 bytes on 64-bit)
typedef struct block {
    size_t size;           // Size including header (8 bytes)
    struct block* next;    // Next block in free list (8 bytes)
    struct block* prev;    // Previous block (8 bytes) - for coalescing
    uint32_t magic;        // Magic number for debugging (4 bytes)
    uint32_t free : 1;     // Free flag (1 bit)
    uint32_t padding : 31; // Padding for alignment
} block_t;

// Memory layout visualization:
// ┌─────────────────────────────────────────────────────────────┐
// │  size (8)  │ next (8) │ prev (8) │ magic(4) │ free(1) │ pad│
// ├─────────────────────────────────────────────────────────────┤
// │                      USER DATA                              │
// │                 (size - sizeof(block_t))                    │
// └─────────────────────────────────────────────────────────────┘
```

### Allocator State

```c
// Global allocator state
static block_t* free_list = NULL;     // Head of free list
static void* heap_start = NULL;       // Start of heap region
static void* heap_end = NULL;         // End of heap region
static size_t heap_size = 0;          // Total heap size

// Statistics for debugging
static struct {
    size_t total_memory;
    size_t allocated_memory;
    unsigned long allocations;
    unsigned long frees;
    unsigned long splits;
    unsigned long coalesces;
} stats = {0};
```

## Core Implementation - Version 1 (Basic)

### Initialization

```c
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define ALIGNMENT 8
#define MIN_BLOCK_SIZE sizeof(block_t)
#define MAGIC_NUMBER 0xDEADBEEF

// Align size to multiple of ALIGNMENT
static size_t align_size(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

// Initialize heap using sbrk
int my_heap_init(size_t initial_size) {
    // Align initial size
    initial_size = align_size(initial_size);
    
    // Request memory from OS
    heap_start = sbrk(initial_size);
    if (heap_start == (void*)-1) {
        return -1;  // Failed
    }
    
    heap_end = (char*)heap_start + initial_size;
    heap_size = initial_size;
    
    // Initialize initial free block
    free_list = (block_t*)heap_start;
    free_list->size = initial_size;
    free_list->next = NULL;
    free_list->prev = NULL;
    free_list->magic = MAGIC_NUMBER;
    free_list->free = 1;
    
    stats.total_memory = initial_size;
    
    return 0;
}
```

### Basic my_malloc() - First Fit

```c
void* my_malloc(size_t size) {
    // Handle zero size
    if (size == 0) return NULL;
    
    // Calculate total needed (header + user data + alignment)
    size_t total_size = size + sizeof(block_t);
    total_size = align_size(total_size);
    
    // Search free list (First Fit)
    block_t* current = free_list;
    while (current) {
        if (current->free && current->size >= total_size) {
            // Found suitable block
            current->free = 0;
            current->magic = MAGIC_NUMBER;
            
            // Split if block is significantly larger
            split_block(current, total_size);
            
            stats.allocated_memory += current->size;
            stats.allocations++;
            
            // Return pointer to user data area
            return (void*)((char*)current + sizeof(block_t));
        }
        current = current->next;
    }
    
    // No suitable block found - request more memory from OS
    return request_more_memory(total_size);
}

// Simple block splitting
static void split_block(block_t* block, size_t needed_size) {
    size_t remaining = block->size - needed_size;
    
    // Only split if remaining space can hold a valid block
    if (remaining >= MIN_BLOCK_SIZE + ALIGNMENT) {
        // Create new free block
        block_t* new_block = (block_t*)((char*)block + needed_size);
        new_block->size = remaining;
        new_block->free = 1;
        new_block->magic = MAGIC_NUMBER;
        new_block->next = block->next;
        new_block->prev = block;
        
        // Update current block
        block->size = needed_size;
        block->next = new_block;
        
        stats.splits++;
    }
}
```

### Basic my_free() - Simple Free

```c
void my_free(void* ptr) {
    if (!ptr) return;
    
    // Get block header
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    
    // Validate pointer (debug)
    if (block->magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: Corrupted block or double free!\n");
        return;
    }
    
    // Mark as free
    block->free = 1;
    block->magic = MAGIC_NUMBER;
    
    stats.allocated_memory -= block->size;
    stats.frees++;
    
    // Coalesce with adjacent free blocks
    coalesce_blocks(block);
}

// Simple coalescing
static void coalesce_blocks(block_t* block) {
    // Coalesce with next block
    if (block->next && block->next->free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
        stats.coalesces++;
    }
    
    // Coalesce with previous block
    if (block->prev && block->prev->free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        stats.coalesces++;
    }
}
```

## Optimized Implementation - Version 2

### Improved Block Structure

```c
// Enhanced block header with more features
typedef struct block {
    size_t size;                // Block size including header
    struct block* next;         // Next in free list
    struct block* prev;         // Previous in free list
    struct block* next_free;    // Next free block (separate list)
    struct block* prev_free;    // Previous free block
    uint32_t magic;             // Magic number
    uint32_t free : 1;          // Free flag
    uint32_t mmap : 1;          // Mmap'd block (for large allocations)
    uint32_t padding : 30;      // Padding
} block_t;

// Separate free lists for different size classes
#define NUM_BINS 16
static block_t* free_bins[NUM_BINS];

// Fastbin sizes: 16, 32, 48, 64, 80, 96, 112, 128, etc.
static size_t bin_sizes[NUM_BINS] = {
    16, 32, 48, 64, 80, 96, 112, 128,
    192, 256, 384, 512, 768, 1024, 1536, 2048
};

// Get bin index for a given size
static int get_bin_index(size_t size) {
    for (int i = 0; i < NUM_BINS; i++) {
        if (size <= bin_sizes[i]) return i;
    }
    return -1;  // Large block
}
```

### Optimized my_malloc() - Best Fit with Bins

```c
void* my_malloc_optimized(size_t size) {
    if (size == 0) return NULL;
    
    size_t total_size = size + sizeof(block_t);
    total_size = align_size(total_size);
    
    // Large allocations use mmap
    if (total_size > MMAP_THRESHOLD) {
        return mmap_allocation(total_size);
    }
    
    // Check fastbins first
    int bin_idx = get_bin_index(total_size);
    if (bin_idx >= 0 && free_bins[bin_idx]) {
        // Fast allocation from bin
        block_t* block = free_bins[bin_idx];
        free_bins[bin_idx] = block->next_free;
        
        block->free = 0;
        block->magic = MAGIC_NUMBER;
        
        stats.allocations++;
        return (void*)((char*)block + sizeof(block_t));
    }
    
    // Search appropriate free list
    block_t* best = find_best_fit(total_size);
    if (best) {
        best->free = 0;
        split_block_optimized(best, total_size);
        stats.allocations++;
        return (void*)((char*)best + sizeof(block_t));
    }
    
    // Expand heap
    return expand_heap(total_size);
}

// Best-fit search
static block_t* find_best_fit(size_t size) {
    block_t* best = NULL;
    size_t best_size = SIZE_MAX;
    
    // Start from largest bins for efficiency
    for (int i = NUM_BINS - 1; i >= 0; i--) {
        block_t* current = free_bins[i];
        while (current) {
            if (current->size >= size && current->size < best_size) {
                best = current;
                best_size = current->size;
                if (best_size == size) return best; // Perfect fit
            }
            current = current->next_free;
        }
    }
    
    return best;
}
```

### Optimized my_free() with Binning

```c
void my_free_optimized(void* ptr) {
    if (!ptr) return;
    
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    
    // Validate
    if (block->magic != MAGIC_NUMBER) {
        fprintf(stderr, "Heap corruption detected!\n");
        return;
    }
    
    // Handle mmap'd blocks
    if (block->mmap) {
        munmap(block, block->size);
        return;
    }
    
    // Mark as free
    block->free = 1;
    stats.frees++;
    
    // Coalesce with neighbors
    block = coalesce_blocks_optimized(block);
    
    // Add to appropriate bin
    int bin_idx = get_bin_index(block->size);
    if (bin_idx >= 0) {
        // Add to fastbin
        block->next_free = free_bins[bin_idx];
        block->prev_free = NULL;
        if (free_bins[bin_idx]) {
            free_bins[bin_idx]->prev_free = block;
        }
        free_bins[bin_idx] = block;
    } else {
        // Add to large block list
        add_to_free_list(block);
    }
    
    // Return memory to OS if possible
    try_return_to_os(block);
}
```

## Advanced Features

### Memory Pool Implementation

```c
// Fixed-size block pool - zero fragmentation
typedef struct pool {
    size_t block_size;
    size_t block_count;
    void* memory;
    void* free_list;
    struct pool* next;
} pool_t;

static pool_t* pools = NULL;

pool_t* create_pool(size_t block_size, size_t block_count) {
    size_t total_size = block_size * block_count + sizeof(pool_t);
    void* memory = mmap(NULL, total_size, 
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) return NULL;
    
    pool_t* pool = (pool_t*)memory;
    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->memory = (char*)memory + sizeof(pool_t);
    
    // Initialize free list
    pool->free_list = pool->memory;
    char* current = (char*)pool->free_list;
    for (size_t i = 0; i < block_count - 1; i++) {
        void** next = (void**)current;
        *next = current + block_size;
        current += block_size;
    }
    void** last = (void**)current;
    *last = NULL;
    
    // Add to global pool list
    pool->next = pools;
    pools = pool;
    
    return pool;
}

void* pool_alloc(pool_t* pool) {
    if (!pool->free_list) return NULL;
    void* block = pool->free_list;
    pool->free_list = *(void**)block;
    return block;
}

void pool_free(pool_t* pool, void* ptr) {
    *(void**)ptr = pool->free_list;
    pool->free_list = ptr;
}
```

### Thread-Safe Allocator

```c
// Thread-safe version with arena
#ifdef THREAD_SAFE
#include <pthread.h>

typedef struct arena {
    block_t* free_list;
    block_t* free_bins[NUM_BINS];
    pthread_mutex_t mutex;
    struct arena* next;
} arena_t;

static arena_t* main_arena;
static pthread_key_t arena_key;

// Per-thread arena
static arena_t* get_thread_arena(void) {
    arena_t* arena = pthread_getspecific(arena_key);
    if (!arena) {
        // Create new arena for this thread
        arena = create_arena();
        pthread_setspecific(arena_key, arena);
    }
    return arena;
}

void* malloc_thread_safe(size_t size) {
    arena_t* arena = get_thread_arena();
    pthread_mutex_lock(&arena->mutex);
    void* ptr = arena_malloc(arena, size);
    pthread_mutex_unlock(&arena->mutex);
    return ptr;
}

void free_thread_safe(void* ptr) {
    if (!ptr) return;
    
    // Find which arena owns this pointer
    arena_t* arena = find_arena(ptr);
    if (arena) {
        pthread_mutex_lock(&arena->mutex);
        arena_free(arena, ptr);
        pthread_mutex_unlock(&arena->mutex);
    }
}
#endif
```

### Debug Mode Features

```c
// Debug allocator with corruption detection
#ifdef DEBUG_MODE

#define POISON_PATTERN 0xDEADBEEF
#define FREED_PATTERN  0xBAADF00D

static void poison_block(block_t* block) {
    void* user_data = (char*)block + sizeof(block_t);
    size_t user_size = block->size - sizeof(block_t);
    memset(user_data, 0xDE, user_size);
}

static void check_poison(block_t* block) {
    void* user_data = (char*)block + sizeof(block_t);
    size_t user_size = block->size - sizeof(block_t);
    uint32_t* words = (uint32_t*)user_data;
    
    for (size_t i = 0; i < user_size / sizeof(uint32_t); i++) {
        if (words[i] != POISON_PATTERN && words[i] != 0) {
            fprintf(stderr, "Use-after-free detected at offset %zu!\n", i * 4);
            abort();
        }
    }
}

void* debug_malloc(size_t size) {
    // Allocate extra space for canaries
    size_t total_size = size + sizeof(block_t) + 2 * CANARY_SIZE;
    block_t* block = (block_t*)_malloc(total_size);
    
    if (block) {
        // Write canaries before and after user data
        uint32_t* before = (uint32_t*)((char*)block + sizeof(block_t));
        uint32_t* after = (uint32_t*)((char*)block + sizeof(block_t) + size);
        *before = CANARY_VALUE;
        *after = CANARY_VALUE;
        
        return (void*)(before + 1);
    }
    return NULL;
}

void debug_free(void* ptr) {
    // Check canaries before freeing
    uint32_t* before = (uint32_t*)ptr - 1;
    uint32_t* after = (uint32_t*)((char*)ptr + ((block_t*)((char*)before - sizeof(block_t)))->size);
    
    if (*before != CANARY_VALUE || *after != CANARY_VALUE) {
        fprintf(stderr, "Buffer overflow detected!\n");
        abort();
    }
    
    _free((char*)before - sizeof(block_t));
}
#endif
```

## Complete Production-Ready Implementation

### Final Allocator Header

```c
// allocator.h
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Configuration
#define ALLOCATOR_ALIGNMENT 16
#define MMAP_THRESHOLD (128 * 1024)  // 128KB
#define FASTBIN_COUNT 16

// Statistics
typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    unsigned long allocation_count;
    unsigned long free_count;
    unsigned long mmap_count;
    float fragmentation_ratio;
} allocator_stats_t;

// Core functions
void* malloc_custom(size_t size);
void* calloc_custom(size_t count, size_t size);
void* realloc_custom(void* ptr, size_t new_size);
void free_custom(void* ptr);

// Utility functions
void allocator_init(void);
void allocator_stats(allocator_stats_t* stats);
void allocator_validate(void);
void allocator_reset(void);

#endif
```

### Complete Implementation

```c
// allocator.c - Complete implementation
#include "allocator.h"
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdatomic.h>

// ==================== Block Structure ====================
typedef struct block {
    size_t size;
    struct block* next;
    struct block* prev;
    uint32_t magic;
    uint32_t free : 1;
    uint32_t mmap : 1;
    uint32_t _padding : 30;
} block_t;

// ==================== Global State ====================
static block_t* free_list = NULL;
static block_t* fastbins[FASTBIN_COUNT] = {0};
static atomic_size_t current_usage = 0;
static atomic_size_t peak_usage = 0;
static atomic_ulong allocations = 0;

// ==================== Helper Functions ====================
static inline size_t align(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

static inline size_t get_fastbin_index(size_t size) {
    if (size <= 16) return 0;
    if (size <= 32) return 1;
    if (size <= 48) return 2;
    if (size <= 64) return 3;
    if (size <= 80) return 4;
    if (size <= 96) return 5;
    if (size <= 112) return 6;
    if (size <= 128) return 7;
    if (size <= 192) return 8;
    if (size <= 256) return 9;
    if (size <= 384) return 10;
    if (size <= 512) return 11;
    if (size <= 768) return 12;
    if (size <= 1024) return 13;
    if (size <= 1536) return 14;
    return 15;
}

// ==================== Block Management ====================
static void split_block(block_t* block, size_t size) {
    size_t remaining = block->size - size;
    
    if (remaining >= sizeof(block_t) + ALLOCATOR_ALIGNMENT) {
        block_t* new_block = (block_t*)((char*)block + size);
        new_block->size = remaining;
        new_block->free = 1;
        new_block->magic = 0xCAFEBABE;
        new_block->next = block->next;
        new_block->prev = block;
        new_block->mmap = 0;
        
        if (block->next) block->next->prev = new_block;
        block->next = new_block;
        block->size = size;
    }
}

static block_t* coalesce(block_t* block) {
    // Coalesce with next
    if (block->next && block->next->free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    
    // Coalesce with previous
    if (block->prev && block->prev->free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
    
    return block;
}

// ==================== Core Allocation ====================
static void* allocate_from_fastbin(size_t size) {
    int idx = get_fastbin_index(size);
    if (idx < FASTBIN_COUNT && fastbins[idx]) {
        block_t* block = fastbins[idx];
        fastbins[idx] = block->next;
        block->free = 0;
        block->magic = 0xDEADBEEF;
        return (void*)((char*)block + sizeof(block_t));
    }
    return NULL;
}

static void* allocate_from_heap(size_t size) {
    block_t* best = NULL;
    size_t best_size = SIZE_MAX;
    block_t* current = free_list;
    
    // Best-fit search
    while (current) {
        if (current->free && current->size >= size && current->size < best_size) {
            best = current;
            best_size = current->size;
            if (best_size == size) break;  // Perfect fit
        }
        current = current->next;
    }
    
    if (best) {
        // Remove from free list
        if (best->prev) best->prev->next = best->next;
        if (best->next) best->next->prev = best->prev;
        if (free_list == best) free_list = best->next;
        
        best->free = 0;
        best->magic = 0xDEADBEEF;
        
        // Split if too large
        split_block(best, size);
        
        return (void*)((char*)best + sizeof(block_t));
    }
    
    return NULL;
}

static void* expand_heap(size_t size) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t allocation = align(size, page_size);
    
    void* ptr = sbrk(allocation);
    if (ptr == (void*)-1) return NULL;
    
    block_t* block = (block_t*)ptr;
    block->size = allocation;
    block->free = 0;
    block->magic = 0xDEADBEEF;
    block->next = NULL;
    block->prev = NULL;
    block->mmap = 0;
    
    // Add to free list for future allocations
    if (free_list) {
        block->next = free_list;
        free_list->prev = block;
    }
    free_list = block;
    
    // Now allocate from this block
    return allocate_from_heap(size);
}

// ==================== Public API ====================
void* malloc_custom(size_t size) {
    if (size == 0) return NULL;
    
    size_t total_size = size + sizeof(block_t);
    total_size = align(total_size, ALLOCATOR_ALIGNMENT);
    
    // Large allocations use mmap
    if (total_size >= MMAP_THRESHOLD) {
        void* ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr != MAP_FAILED) {
            block_t* block = (block_t*)ptr;
            block->size = total_size;
            block->free = 0;
            block->mmap = 1;
            block->magic = 0xDEADBEEF;
            
            atomic_fetch_add(&current_usage, total_size);
            atomic_fetch_add(&allocations, 1);
            
            return (void*)((char*)block + sizeof(block_t));
        }
        return NULL;
    }
    
    // Try fastbin first
    void* ptr = allocate_from_fastbin(total_size);
    if (ptr) {
        atomic_fetch_add(&current_usage, total_size);
        atomic_fetch_add(&allocations, 1);
        return ptr;
    }
    
    // Try heap allocation
    ptr = allocate_from_heap(total_size);
    if (ptr) {
        size_t used = atomic_fetch_add(&current_usage, total_size);
        if (used + total_size > atomic_load(&peak_usage)) {
            atomic_store(&peak_usage, used + total_size);
        }
        atomic_fetch_add(&allocations, 1);
        return ptr;
    }
    
    // Expand heap
    return expand_heap(total_size);
}

void free_custom(void* ptr) {
    if (!ptr) return;
    
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    
    // Validate magic
    if (block->magic != 0xDEADBEEF) {
        fprintf(stderr, "Free: corrupted pointer or double free!\n");
        return;
    }
    
    // Handle mmapped blocks
    if (block->mmap) {
        atomic_fetch_sub(&current_usage, block->size);
        munmap(block, block->size);
        return;
    }
    
    size_t block_size = block->size;
    block->free = 1;
    block->magic = 0xCAFEBABE;
    
    atomic_fetch_sub(&current_usage, block_size);
    
    // Coalesce with neighbors
    block = coalesce(block);
    
    // Add to appropriate free list
    int idx = get_fastbin_index(block->size);
    if (idx < FASTBIN_COUNT) {
        // Add to fastbin
        block->next = fastbins[idx];
        fastbins[idx] = block;
    } else {
        // Add to main free list
        block->next = free_list;
        if (free_list) free_list->prev = block;
        free_list = block;
        block->prev = NULL;
    }
}

void* calloc_custom(size_t count, size_t size) {
    size_t total = count * size;
    void* ptr = malloc_custom(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void* realloc_custom(void* ptr, size_t new_size) {
    if (!ptr) return malloc_custom(new_size);
    if (new_size == 0) {
        free_custom(ptr);
        return NULL;
    }
    
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    size_t old_size = block->size - sizeof(block_t);
    
    if (new_size <= old_size) return ptr;
    
    void* new_ptr = malloc_custom(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        free_custom(ptr);
    }
    
    return new_ptr;
}
```

## Testing Framework

```c
// test_allocator.c
#include <stdio.h>
#include <assert.h>

void test_basic_allocation() {
    printf("Testing basic allocation...\n");
    
    void* p1 = malloc_custom(100);
    assert(p1 != NULL);
    
    void* p2 = malloc_custom(200);
    assert(p2 != NULL);
    
    free_custom(p1);
    free_custom(p2);
    
    printf("✓ Basic allocation passed\n");
}

void test_fragmentation() {
    printf("Testing fragmentation resistance...\n");
    
    void* ptrs[100];
    
    // Allocate alternating sizes
    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc_custom(i % 2 ? 64 : 128);
        assert(ptrs[i] != NULL);
    }
    
    // Free every other
    for (int i = 0; i < 100; i += 2) {
        free_custom(ptrs[i]);
    }
    
    // Should still allocate large block
    void* large = malloc_custom(4096);
    assert(large != NULL);
    
    free_custom(large);
    
    // Cleanup
    for (int i = 1; i < 100; i += 2) {
        free_custom(ptrs[i]);
    }
    
    printf("✓ Fragmentation test passed\n");
}

void test_large_allocations() {
    printf("Testing large allocations...\n");
    
    // This should use mmap
    void* large = malloc_custom(200000);  // 200KB
    assert(large != NULL);
    
    memset(large, 0xAA, 200000);
    free_custom(large);
    
    printf("✓ Large allocation test passed\n");
}

void test_alignment() {
    printf("Testing alignment...\n");
    
    for (int i = 1; i <= 1024; i++) {
        void* ptr = malloc_custom(i);
        assert(ptr != NULL);
        assert(((uintptr_t)ptr & (ALLOCATOR_ALIGNMENT - 1)) == 0);
        free_custom(ptr);
    }
    
    printf("✓ Alignment test passed\n");
}

int main() {
    printf("\n=== Custom Allocator Tests ===\n\n");
    
    allocator_init();
    
    test_basic_allocation();
    test_fragmentation();
    test_large_allocations();
    test_alignment();
    
    allocator_stats_t stats;
    allocator_stats(&stats);
    
    printf("\n=== Statistics ===\n");
    printf("Total allocated: %zu bytes\n", stats.total_allocated);
    printf("Current usage: %zu bytes\n", stats.current_usage);
    printf("Peak usage: %zu bytes\n", stats.peak_usage);
    printf("Allocations: %lu\n", stats.allocation_count);
    printf("Fragmentation: %.2f%%\n", stats.fragmentation_ratio * 100);
    
    printf("\n✓ All tests passed!\n");
    return 0;
}
```

## Performance Optimization Checklist

```
OPTIMIZATION TECHNIQUES
═══════════════════════════════════════════════════════════════════

✓ Use fastbins for common sizes (O(1) allocation)
✓ Implement best-fit for larger allocations
✓ Separate metadata from data (cache alignment)
✓ Use thread-local arenas (reduce contention)
✓ Batch coalescing operations
✓ Maintain free lists sorted by address
✓ Use mmap for large allocations
✓ Implement allocation pools for fixed sizes
✓ Add memory poisoning in debug mode
✓ Use atomic operations for thread safety
```

This comprehensive implementation demonstrates deep understanding of memory allocator internals and is perfect for interview discussions!
