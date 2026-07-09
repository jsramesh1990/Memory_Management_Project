/**
 * @file pool.h
 * @brief Fixed-size block pool allocator - Zero fragmentation, O(1) operations
 */

#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Configuration ======================== */

#ifndef POOL_ALIGNMENT
#define POOL_ALIGNMENT 8
#endif

#ifndef POOL_MAX_BLOCKS
#define POOL_MAX_BLOCKS 65536  /**< Maximum number of blocks per pool */
#endif

/* ======================== Statistics ======================== */

typedef struct {
    size_t total_blocks;        /**< Total blocks in pool */
    size_t block_size;          /**< Size of each block (bytes) */
    size_t free_blocks;         /**< Currently free blocks */
    size_t allocated_blocks;    /**< Currently allocated blocks */
    unsigned long total_allocations;
    unsigned long total_frees;
    unsigned long allocation_failures;
    unsigned long cache_hits;    /**< Free list cache hits */
    unsigned long cache_misses;  /**< Free list cache misses */
} pool_stats_t;

/* ======================== Pool Handle ======================== */

typedef struct pool_allocator pool_alloc_t;

/* ======================== Core Functions ======================== */

/**
 * @brief Create a new pool allocator
 * @param block_size Size of each fixed block (automatically aligned)
 * @param block_count Number of blocks in the pool
 * @param memory Optional pre-allocated memory buffer (NULL = dynamic allocation)
 * @return Pool handle or NULL on failure
 */
pool_alloc_t* pool_create(size_t block_size, size_t block_count, void* memory);

/**
 * @brief Destroy pool and free associated memory
 * @param pool Pool to destroy
 */
void pool_destroy(pool_alloc_t* pool);

/**
 * @brief Allocate a single block from pool
 * @param pool Pool to allocate from
 * @return Pointer to block or NULL if pool is empty
 */
void* pool_alloc(pool_alloc_t* pool);

/**
 * @brief Allocate multiple blocks from pool
 * @param pool Pool to allocate from
 * @param count Number of blocks to allocate
 * @param out_ptrs Array to store pointers (must have space for 'count')
 * @return Number of blocks successfully allocated
 */
size_t pool_alloc_multiple(pool_alloc_t* pool, size_t count, void** out_ptrs);

/**
 * @brief Free a block back to pool
 * @param pool Pool that owns the block
 * @param ptr Block to free (must come from this pool)
 * @return true if successful, false if pointer invalid
 */
bool pool_free(pool_alloc_t* pool, void* ptr);

/**
 * @brief Free multiple blocks
 * @param pool Pool that owns the blocks
 * @param count Number of blocks to free
 * @param ptrs Array of pointers to free
 * @return Number of blocks successfully freed
 */
size_t pool_free_multiple(pool_alloc_t* pool, size_t count, void** ptrs);

/* ======================== Utility Functions ======================== */

/**
 * @brief Get pool statistics
 * @param pool Pool to query
 * @param stats Output structure for statistics
 */
void pool_stats(pool_alloc_t* pool, pool_stats_t* stats);

/**
 * @brief Reset pool to empty state
 * @warning Only call when ALL blocks have been freed!
 * @param pool Pool to reset
 * @return true if successful, false if blocks still allocated
 */
bool pool_reset(pool_alloc_t* pool);

/**
 * @brief Check if pointer belongs to this pool
 * @param pool Pool to check
 * @param ptr Pointer to validate
 * @return true if pointer is from this pool
 */
bool pool_owns_pointer(pool_alloc_t* pool, void* ptr);

/**
 * @brief Get block size for this pool
 * @param pool Pool to query
 * @return Block size in bytes
 */
size_t pool_block_size(pool_alloc_t* pool);

/**
 * @brief Get number of free blocks
 * @param pool Pool to query
 * @return Number of free blocks
 */
size_t pool_free_count(pool_alloc_t* pool);

#ifdef __cplusplus
}
#endif

#endif /* POOL_ALLOCATOR_H */
