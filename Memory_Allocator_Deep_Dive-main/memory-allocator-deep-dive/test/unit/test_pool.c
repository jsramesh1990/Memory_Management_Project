/**
 * @file test_pool.c
 * @brief Unit tests for pool allocator
 */

#include <stdio.h>
#include <string.h>
#include "pool.h"

#define TEST_PASS() printf("\033[32m✓ %s passed\033[0m\n", __func__)
#define TEST_FAIL(fmt, ...) do { \
    printf("\033[31m✗ %s failed: " fmt "\033[0m\n", __func__, ##__VA_ARGS__); \
    return 1; \
} while(0)

static int test_pool_basic(void) {
    pool_alloc_t* pool = pool_create(64, 100, NULL);
    if (!pool) TEST_FAIL("pool creation failed");
    
    void* p1 = pool_alloc(pool);
    void* p2 = pool_alloc(pool);
    
    if (!p1 || !p2) TEST_FAIL("allocation failed");
    if (p1 == p2) TEST_FAIL("got same pointer twice");
    
    pool_free(pool, p1);
    pool_free(pool, p2);
    
    pool_destroy(pool);
    TEST_PASS();
    return 0;
}

static int test_pool_exhaustion(void) {
    pool_alloc_t* pool = pool_create(32, 10, NULL);
    if (!pool) TEST_FAIL("pool creation failed");
    
    void* ptrs[20] = {0};
    int allocated = 0;
    
    /* Allocate until failure */
    for (int i = 0; i < 20; i++) {
        ptrs[i] = pool_alloc(pool);
        if (ptrs[i]) allocated++;
        else break;
    }
    
    if (allocated != 10) TEST_FAIL("should allocate exactly 10 blocks, got %d", allocated);
    
    /* Free and allocate again */
    for (int i = 0; i < 10; i++) {
        pool_free(pool, ptrs[i]);
    }
    
    void* p = pool_alloc(pool);
    if (!p) TEST_FAIL("failed to allocate after free");
    
    pool_free(pool, p);
    pool_destroy(pool);
    
    TEST_PASS();
    return 0;
}

static int test_pool_pointer_validation(void) {
    pool_alloc_t* pool = pool_create(64, 10, NULL);
    if (!pool) TEST_FAIL("pool creation failed");
    
    void* ptr = pool_alloc(pool);
    if (!ptr) TEST_FAIL("allocation failed");
    
    /* Wrong pointer should be rejected */
    int fake = 42;
    if (pool_free(pool, &fake)) TEST_FAIL("should reject foreign pointer");
    
    /* Double free should be rejected */
    if (!pool_free(pool, ptr)) TEST_FAIL("first free failed");
    if (pool_free(pool, ptr)) TEST_FAIL("double free should be rejected");
    
    pool_destroy(pool);
    TEST_PASS();
    return 0;
}

static int test_pool_stats(void) {
    pool_alloc_t* pool = pool_create(128, 50, NULL);
    if (!pool) TEST_FAIL("pool creation failed");
    
    pool_stats_t stats;
    pool_stats(pool, &stats);
    
    if (stats.total_blocks != 50) TEST_FAIL("wrong total blocks");
    if (stats.free_blocks != 50) TEST_FAIL("wrong free blocks");
    
    void* ptrs[30];
    for (int i = 0; i < 30; i++) {
        ptrs[i] = pool_alloc(pool);
    }
    
    pool_stats(pool, &stats);
    if (stats.allocated_blocks != 30) TEST_FAIL("wrong allocated count");
    if (stats.free_blocks != 20) TEST_FAIL("wrong free count");
    
    for (int i = 0; i < 30; i++) {
        pool_free(pool, ptrs[i]);
    }
    
    pool_stats(pool, &stats);
    if (stats.allocated_blocks != 0) TEST_FAIL("blocks not freed");
    
    pool_destroy(pool);
    TEST_PASS();
    return 0;
}

int main(void) {
    printf("\n=== Pool Allocator Tests ===\n\n");
    
    int failures = 0;
    failures += test_pool_basic();
    failures += test_pool_exhaustion();
    failures += test_pool_pointer_validation();
    failures += test_pool_stats();
    
    printf("\n===========================\n");
    if (failures == 0) {
        printf("  ✓ ALL POOL TESTS PASSED\n");
    } else {
        printf("  ✗ %d TESTS FAILED\n", failures);
    }
    printf("===========================\n\n");
    
    return failures ? 1 : 0;
}
