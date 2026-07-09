/**
 * @file basic_allocation.c
 * @brief Basic usage example for memory allocator
 */

#include <stdio.h>
#include <string.h>
#include "allocator.h"
#include "pool.h"

/* Static memory pool for allocator */
static uint8_t heap[2 * 1024 * 1024];  /* 2MB heap */

int main(void) {
    printf("=== Memory Allocator Demo ===\n\n");
    
    /* Initialize allocator */
    if (!allocator_init(heap, sizeof(heap))) {
        printf("Failed to initialize allocator\n");
        return 1;
    }
    
    printf("1. Basic allocation\n");
    int* numbers = (int*)allocator_malloc(10 * sizeof(int));
    if (numbers) {
        for (int i = 0; i < 10; i++) {
            numbers[i] = i * i;
        }
        printf("   Allocated array: ");
        for (int i = 0; i < 10; i++) {
            printf("%d ", numbers[i]);
        }
        printf("\n");
        allocator_free(numbers);
    }
    
    printf("\n2. Pool allocator demo\n");
    pool_alloc_t* pool = pool_create(64, 100, NULL);
    if (pool) {
        void* messages[10];
        for (int i = 0; i < 10; i++) {
            messages[i] = pool_alloc(pool);
            if (messages[i]) {
                sprintf((char*)messages[i], "Message %d", i);
            }
        }
        
        for (int i = 0; i < 10; i++) {
            printf("   %s\n", (char*)messages[i]);
            pool_free(pool, messages[i]);
        }
        
        pool_destroy(pool);
    }
    
    printf("\n3. Statistics\n");
    allocator_stats_t stats;
    allocator_get_stats(&stats);
    printf("   Total memory: %zu bytes\n", stats.total_memory);
    printf("   Allocated: %zu bytes\n", stats.allocated_memory);
    printf("   Free: %zu bytes\n", stats.free_memory);
    printf("   Allocations: %lu\n", stats.allocation_count);
    
    printf("\n=== Demo Complete ===\n");
    return 0;
}
