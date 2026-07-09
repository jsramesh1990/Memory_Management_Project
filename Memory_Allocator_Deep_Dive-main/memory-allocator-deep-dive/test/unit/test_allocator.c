/**
 * @file test_allocator.c
 * @brief Comprehensive unit tests for memory allocator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "allocator.h"

#define TEST_PASS() printf("\033[32m✓ %s passed\033[0m\n", __func__)
#define TEST_FAIL(fmt, ...) do { \
    printf("\033[31m✗ %s failed: " fmt "\033[0m\n", __func__, ##__VA_ARGS__); \
    return 1; \
} while(0)

#define TEST_HEAP_SIZE (1024 * 1024)  /* 1MB test heap */

static uint8_t test_heap[TEST_HEAP_SIZE];

/* Test 1: Basic allocation */
static int test_basic_allocation(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    void* p1 = allocator_malloc(64);
    void* p2 = allocator_malloc(128);
    void* p3 = allocator_malloc(256);
    
    if (!p1) TEST_FAIL("malloc 64 failed");
    if (!p2) TEST_FAIL("malloc 128 failed");
    if (!p3) TEST_FAIL("malloc 256 failed");
    
    /* Check no overlap */
    if ((char*)p1 + 64 > (char*)p2) TEST_FAIL("blocks overlap");
    if ((char*)p2 + 128 > (char*)p3) TEST_FAIL("blocks overlap");
    
    allocator_free(p2);
    void* p4 = allocator_malloc(128);
    if (p4 != p2) TEST_FAIL("memory not reused");
    
    allocator_free(p1);
    allocator_free(p3);
    allocator_free(p4);
    
    TEST_PASS();
    return 0;
}

/* Test 2: Zero-size and NULL frees */
static int test_edge_cases(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    /* Zero size should return NULL */
    if (allocator_malloc(0) != NULL) TEST_FAIL("malloc(0) should return NULL");
    
    /* Freeing NULL should not crash */
    allocator_free(NULL);
    
    /* Very large allocation should fail gracefully */
    void* huge = allocator_malloc(TEST_HEAP_SIZE * 2);
    if (huge != NULL) TEST_FAIL("oversized allocation should fail");
    
    TEST_PASS();
    return 0;
}

/* Test 3: Calloc zero-initialization */
static int test_calloc_zeroing(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    void* ptr = allocator_calloc(100, sizeof(int));
    if (!ptr) TEST_FAIL("calloc failed");
    
    int* arr = (int*)ptr;
    for (int i = 0; i < 100; i++) {
        if (arr[i] != 0) TEST_FAIL("calloc didn't zero memory at index %d", i);
    }
    
    allocator_free(ptr);
    TEST_PASS();
    return 0;
}

/* Test 4: Reallocation */
static int test_realloc(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    /* realloc(NULL, size) == malloc(size) */
    void* p1 = allocator_realloc(NULL, 100);
    if (!p1) TEST_FAIL("realloc(NULL) failed");
    
    /* Write data */
    memset(p1, 0xAA, 100);
    
    /* realloc to larger size */
    void* p2 = allocator_realloc(p1, 200);
    if (!p2) TEST_FAIL("realloc to larger failed");
    
    /* Data should be preserved */
    unsigned char* data = (unsigned char*)p2;
    for (int i = 0; i < 100; i++) {
        if (data[i] != 0xAA) TEST_FAIL("realloc corrupted data at %d", i);
    }
    
    /* realloc to smaller size */
    void* p3 = allocator_realloc(p2, 50);
    if (!p3) TEST_FAIL("realloc to smaller failed");
    
    allocator_free(p3);
    TEST_PASS();
    return 0;
}

/* Test 5: Fragmentation resistance */
static int test_fragmentation(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    #define NUM_ALLOCS 500
    void* ptrs[NUM_ALLOCS];
    
    /* Allocate alternating sizes */
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = (i % 2 == 0) ? 64 : 128;
        ptrs[i] = allocator_malloc(size);
        if (!ptrs[i]) TEST_FAIL("allocation %d failed", i);
        memset(ptrs[i], i, size);
    }
    
    /* Free every other block (creates holes) */
    for (int i = 0; i < NUM_ALLOCS; i += 2) {
        allocator_free(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    /* Should still allocate large block */
    void* large = allocator_malloc(4096);
    if (!large) TEST_FAIL("fragmentation prevented large allocation");
    
    /* Cleanup */
    allocator_free(large);
    for (int i = 1; i < NUM_ALLOCS; i += 2) {
        allocator_free(ptrs[i]);
    }
    
    TEST_PASS();
    return 0;
}

/* Test 6: Alignment */
static int test_alignment(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    for (int i = 1; i <= 1024; i++) {
        void* ptr = allocator_malloc(i);
        if (ptr) {
            /* Check alignment to 8 bytes (for 64-bit) */
            if (((uintptr_t)ptr & 7) != 0) {
                TEST_FAIL("pointer %p not 8-byte aligned for size %d", ptr, i);
            }
            allocator_free(ptr);
        }
    }
    
    TEST_PASS();
    return 0;
}

/* Test 7: Statistics accuracy */
static int test_statistics(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    allocator_stats_t stats_before, stats_after;
    
    allocator_get_stats(&stats_before);
    
    void* p1 = allocator_malloc(1000);
    void* p2 = allocator_malloc(2000);
    
    allocator_get_stats(&stats_after);
    
    if (stats_after.allocated_memory <= stats_before.allocated_memory) {
        TEST_FAIL("allocated memory not increased");
    }
    
    if (stats_after.allocation_count != stats_before.allocation_count + 2) {
        TEST_FAIL("allocation count wrong");
    }
    
    allocator_free(p1);
    allocator_free(p2);
    
    TEST_PASS();
    return 0;
}

/* Test 8: Heap validation */
static int test_heap_validation(void) {
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    if (!allocator_validate()) TEST_FAIL("validation failed on clean heap");
    
    void* ptr = allocator_malloc(100);
    if (!allocator_validate()) TEST_FAIL("validation failed after allocation");
    
    /* Corrupt magic number (simulate bug) */
    size_t* magic_ptr = (size_t*)((char*)ptr - 4);
    *magic_ptr = 0xFFFFFFFF;
    
    if (allocator_validate()) TEST_FAIL("validation should detect corruption");
    
    /* Reinitialize for next test */
    allocator_init(test_heap, TEST_HEAP_SIZE);
    
    TEST_PASS();
    return 0;
}

/* Main test runner */
int main(void) {
    printf("\n========================================\n");
    printf("  Memory Allocator Test Suite\n");
    printf("========================================\n\n");
    
    int failures = 0;
    
    failures += test_basic_allocation();
    failures += test_edge_cases();
    failures += test_calloc_zeroing();
    failures += test_realloc();
    failures += test_fragmentation();
    failures += test_alignment();
    failures += test_statistics();
    failures += test_heap_validation();
    
    printf("\n========================================\n");
    if (failures == 0) {
        printf("  ✓ ALL TESTS PASSED\n");
    } else {
        printf("  ✗ %d TESTS FAILED\n", failures);
    }
    printf("========================================\n\n");
    
    return failures ? 1 : 0;
}
