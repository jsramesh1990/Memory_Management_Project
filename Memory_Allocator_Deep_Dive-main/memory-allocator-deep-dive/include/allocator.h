/**
 * @file allocator.h
 * @brief Main memory allocator interface
 * @author Your Name
 * @version 1.0.0
 * 
 * This header provides the primary interface for memory allocation operations.
 * Supports multiple allocation strategies configurable at compile time.
 */

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Configuration Macros ======================== */

#ifndef ALLOCATOR_ALIGNMENT
#define ALLOCATOR_ALIGNMENT 8  /**< Default alignment in bytes (8 for 64-bit) */
#endif

#ifndef ALLOCATOR_MAGIC_NUMBER
#define ALLOCATOR_MAGIC_NUMBER 0xDEADBEEF  /**< Magic number for corruption detection */
#endif

#ifndef ALLOCATOR_THREAD_SAFE
#define ALLOCATOR_THREAD_SAFE 0  /**< Enable thread safety (0=disabled, 1=enabled) */
#endif

/* ======================== Statistics Structure ======================== */

/**
 * @struct allocator_stats_t
 * @brief Statistics about allocator usage
 */
typedef struct {
    size_t total_memory;        /**< Total memory managed by allocator */
    size_t allocated_memory;    /**< Currently allocated memory */
    size_t free_memory;         /**< Currently free memory */
    unsigned long allocation_count;   /**< Total number of allocations */
    unsigned long free_count;         /**< Total number of frees */
    unsigned long coalesce_count;     /**< Number of block coalescing operations */
    size_t smallest_allocation;       /**< Smallest allocation ever made */
    size_t largest_allocation;        /**< Largest allocation ever made */
    size_t fragmentation_count;       /**< Number of free blocks (fragmentation metric) */
} allocator_stats_t;

/* ======================== Error Callbacks ======================== */

/**
 * @brief Error handler function type
 * @param msg Error message
 * @param ptr Pointer involved (if applicable)
 */
typedef void (*allocator_error_handler_t)(const char* msg, void* ptr);

/**
 * @brief Set custom error handler
 * @param handler Error handler function (NULL for default)
 */
void allocator_set_error_handler(allocator_error_handler_t handler);

/* ======================== Core Functions ======================== */

/**
 * @brief Initialize the memory allocator
 * @param heap_start Starting address of memory pool
 * @param heap_size Size of memory pool in bytes
 * @return true on success, false on failure
 * 
 * @example
 * static uint8_t my_heap[1024 * 1024];  // 1MB heap
 * allocator_init(my_heap, sizeof(my_heap));
 */
bool allocator_init(void* heap_start, size_t heap_size);

/**
 * @brief Allocate memory
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void* allocator_malloc(size_t size);

/**
 * @brief Allocate and zero-initialize memory
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory or NULL on failure
 */
void* allocator_calloc(size_t count, size_t size);

/**
 * @brief Reallocate memory
 * @param ptr Previously allocated pointer (can be NULL)
 * @param new_size New size in bytes
 * @return Pointer to reallocated memory or NULL on failure
 */
void* allocator_realloc(void* ptr, size_t new_size);

/**
 * @brief Free allocated memory
 * @param ptr Pointer returned by allocator_malloc/calloc/realloc
 */
void allocator_free(void* ptr);

/**
 * @brief Aligned memory allocation
 * @param alignment Required alignment (must be power of 2)
 * @param size Size in bytes
 * @return Pointer to aligned memory or NULL on failure
 */
void* allocator_aligned_alloc(size_t alignment, size_t size);

/**
 * @brief Free aligned memory
 * @param ptr Pointer from allocator_aligned_alloc
 */
void allocator_aligned_free(void* ptr);

/* ======================== Utility Functions ======================== */

/**
 * @brief Get allocator statistics
 * @param stats Output structure for statistics
 */
void allocator_get_stats(allocator_stats_t* stats);

/**
 * @brief Reset allocator to initial state (frees all memory)
 * @warning Only call when all allocations have been freed!
 * @return true on success, false if memory leaks detected
 */
bool allocator_reset(void);

/**
 * @brief Check heap integrity
 * @return true if heap is valid, false if corruption detected
 */
bool allocator_validate(void);

/**
 * @brief Get total free memory
 * @return Amount of free memory in bytes
 */
size_t allocator_get_free_memory(void);

/**
 * @brief Get total allocated memory
 * @return Amount of allocated memory in bytes
 */
size_t allocator_get_allocated_memory(void);

/**
 * @brief Print heap state (for debugging)
 */
void allocator_dump_heap(void);

#ifdef __cplusplus
}
#endif

#endif /* ALLOCATOR_H */
