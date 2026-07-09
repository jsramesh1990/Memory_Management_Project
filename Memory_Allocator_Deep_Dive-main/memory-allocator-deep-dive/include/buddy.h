/**
 * @file buddy.h
 * @brief Buddy system allocator - Power-of-two block allocation
 */

#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum order (2^MAX_ORDER bytes) */
#ifndef BUDDY_MAX_ORDER
#define BUDDY_MAX_ORDER 20  /* 1MB max block */
#endif

#ifndef BUDDY_MIN_ORDER
#define BUDDY_MIN_ORDER 4   /* 16 bytes min block */
#endif

/* Statistics structure */
typedef struct {
    size_t total_memory;
    size_t allocated_memory;
    unsigned long allocations[32];  /* Allocations per order */
    unsigned long frees[32];
    unsigned long splits;
    unsigned long coalesces;
    unsigned long allocation_failures;
} buddy_stats_t;

/* Opaque buddy allocator handle */
typedef struct buddy_allocator buddy_t;

/**
 * @brief Create buddy allocator
 * @param memory Memory region to manage (must be power of 2 size)
 * @param size Size of memory region (must be power of 2)
 * @return Buddy allocator handle or NULL
 */
buddy_t* buddy_create(void* memory, size_t size);

/**
 * @brief Destroy buddy allocator
 * @param buddy Buddy handle
 */
void buddy_destroy(buddy_t* buddy);

/**
 * @brief Allocate memory of at least given size
 * @param buddy Buddy handle
 * @param size Requested size (rounded up to next power of two)
 * @return Pointer to allocated memory or NULL
 */
void* buddy_alloc(buddy_t* buddy, size_t size);

/**
 * @brief Free allocated memory
 * @param buddy Buddy handle
 * @param ptr Pointer to free
 */
void buddy_free(buddy_t* buddy, void* ptr);

/**
 * @brief Get statistics
 * @param buddy Buddy handle
 * @param stats Output statistics
 */
void buddy_stats(buddy_t* buddy, buddy_stats_t* stats);

/**
 * @brief Get allocation size for a pointer
 * @param buddy Buddy handle
 * @param ptr Allocated pointer
 * @return Size of allocation in bytes
 */
size_t buddy_get_allocation_size(buddy_t* buddy, void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* BUDDY_ALLOCATOR_H */
