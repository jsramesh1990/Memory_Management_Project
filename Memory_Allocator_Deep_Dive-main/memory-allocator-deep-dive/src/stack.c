/**
 * @file stack.c
 * @brief Stack (LIFO) allocator - Very fast, linear allocation
 */

#include "stack.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct stack_allocator {
    uint8_t* memory_start;
    uint8_t* memory_end;
    uint8_t* current;
    
    /* Statistics */
    size_t total_memory;
    size_t used_memory;
    unsigned long allocations;
    unsigned long frees;
    
    /* For marker support */
    struct marker {
        uint8_t* position;
        struct marker* next;
    }* markers;
};

stack_alloc_t* stack_create(void* memory, size_t size) {
    if (!memory || size < 16) return NULL;
    
    stack_alloc_t* stack = (stack_alloc_t*)memory;
    uint8_t* memory_start = (uint8_t*)memory + sizeof(stack_alloc_t);
    size_t usable_size = size - sizeof(stack_alloc_t);
    
    stack->memory_start = memory_start;
    stack->memory_end = memory_start + usable_size;
    stack->current = memory_start;
    stack->total_memory = usable_size;
    stack->used_memory = 0;
    stack->allocations = 0;
    stack->frees = 0;
    stack->markers = NULL;
    
    return stack;
}

void* stack_alloc(stack_alloc_t* stack, size_t size) {
    if (!stack) return NULL;
    
    /* Align size */
    size = (size + 7) & ~7;
    
    /* Check if enough space */
    if (stack->current + size > stack->memory_end) {
        return NULL;
    }
    
    void* ptr = stack->current;
    stack->current += size;
    stack->used_memory += size;
    stack->allocations++;
    
    return ptr;
}

void stack_free(stack_alloc_t* stack) {
    if (!stack) return;
    
    /* Just reset current pointer (LIFO only) */
    stack->current = stack->memory_start;
    stack->used_memory = 0;
    stack->frees++;
    
    /* Clear markers */
    stack->markers = NULL;
}

void* stack_get_marker(stack_alloc_t* stack) {
    if (!stack) return NULL;
    return stack->current;
}

void stack_free_to_marker(stack_alloc_t* stack, void* marker) {
    if (!stack || !marker) return;
    
    uint8_t* marker_pos = (uint8_t*)marker;
    if (marker_pos >= stack->memory_start && marker_pos <= stack->memory_end) {
        size_t freed = (size_t)(stack->current - marker_pos);
        stack->used_memory -= freed;
        stack->current = marker_pos;
        stack->frees++;
    }
}

void stack_stats(stack_alloc_t* stack, stack_stats_t* stats) {
    if (!stack || !stats) return;
    
    stats->total_memory = stack->total_memory;
    stats->used_memory = stack->used_memory;
    stats->available_memory = stack->total_memory - stack->used_memory;
    stats->allocations = stack->allocations;
    stats->frees = stack->frees;
}

void stack_reset(stack_alloc_t* stack) {
    if (!stack) return;
    
    stack->current = stack->memory_start;
    stack->used_memory = 0;
    stack->markers = NULL;
}
