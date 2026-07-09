/**
 * @file allocator_config.h
 * @brief Compile-time configuration for memory allocator
 */

#ifndef ALLOCATOR_CONFIG_H
#define ALLOCATOR_CONFIG_H

/* ======================== Selection of Allocation Strategy ======================== */

/**
 * Select allocation algorithm:
 * 0 - First-fit (default)
 * 1 - Best-fit
 * 2 - Next-fit
 * 3 - Worst-fit
 */
#ifndef ALLOCATOR_STRATEGY
#define ALLOCATOR_STRATEGY 0
#endif

/* ======================== Memory Pool Configuration ======================== */

#ifndef ALLOCATOR_DEFAULT_POOL_SIZE
#define ALLOCATOR_DEFAULT_POOL_SIZE (1024 * 1024)  /**< 1MB default pool */
#endif

#ifndef ALLOCATOR_MIN_BLOCK_SIZE
#define ALLOCATOR_MIN_BLOCK_SIZE 32  /**< Minimum block size including header */
#endif

#ifndef ALLOCATOR_MAX_BLOCK_SIZE
#define ALLOCATOR_MAX_BLOCK_SIZE (1024 * 1024 * 1024)  /**< 1GB maximum */
#endif

/* ======================== Debug and Validation ======================== */

#ifndef ALLOCATOR_DEBUG
#define ALLOCATOR_DEBUG 0  /**< Enable debug output (0=disabled, 1=basic, 2=verbose) */
#endif

#ifndef ALLOCATOR_VALIDATE_ON_FREE
#define ALLOCATOR_VALIDATE_ON_FREE 1  /**< Validate heap on every free */
#endif

#ifndef ALLOCATOR_POISON_ON_FREE
#define ALLOCATOR_POISON_ON_FREE 1  /**< Fill freed memory with poison pattern */
#endif

#ifndef ALLOCATOR_POISON_PATTERN
#define ALLOCATOR_POISON_PATTERN 0xDEADBEEF  /**< Pattern for poisoning freed memory */
#endif

/* ======================== Performance Tuning ======================== */

#ifndef ALLOCATOR_COALESCE_THRESHOLD
#define ALLOCATOR_COALESCE_THRESHOLD 16  /**< Minimum size to attempt coalescing */
#endif

#ifndef ALLOCATOR_SPLIT_THRESHOLD
#define ALLOCATOR_SPLIT_THRESHOLD 32  /**< Minimum remaining space to split block */
#endif

#ifndef ALLOCATOR_CACHE_FREE_LIST
#define ALLOCATOR_CACHE_FREE_LIST 1  /**< Cache free list head for faster access */
#endif

/* ======================== Platform Specific ======================== */

/* Detect platform */
#if defined(__linux__)
    #define ALLOCATOR_PLATFORM_LINUX 1
#elif defined(__ANDROID__)
    #define ALLOCATOR_PLATFORM_ANDROID 1
#elif defined(__arm__) || defined(__ARMEL__)
    #define ALLOCATOR_PLATFORM_EMBEDDED 1
#else
    #define ALLOCATOR_PLATFORM_GENERIC 1
#endif

/* Thread safety */
#if ALLOCATOR_THREAD_SAFE
    #ifdef __GNUC__
        #define ALLOCATOR_LOCK() __sync_synchronize()
        #define ALLOCATOR_UNLOCK() __sync_synchronize()
    #else
        #error "Thread safety requires compiler atomics support"
    #endif
#else
    #define ALLOCATOR_LOCK()
    #define ALLOCATOR_UNLOCK()
#endif

/* Inline hints */
#ifdef __GNUC__
    #define ALLOCATOR_ALWAYS_INLINE __attribute__((always_inline)) inline
    #define ALLOCATOR_HOT __attribute__((hot))
    #define ALLOCATOR_COLD __attribute__((cold))
#else
    #define ALLOCATOR_ALWAYS_INLINE inline
    #define ALLOCATOR_HOT
    #define ALLOCATOR_COLD
#endif

/* Deprecation warnings */
#ifdef __GNUC__
    #define ALLOCATOR_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
    #define ALLOCATOR_DEPRECATED(msg)
#endif

#endif /* ALLOCATOR_CONFIG_H */
