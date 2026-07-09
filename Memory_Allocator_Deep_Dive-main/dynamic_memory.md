# Dynamic Memory

## Overview

Dynamic memory allocation allows programs to request memory at runtime from the heap. Unlike stack allocation, heap memory persists until explicitly freed, enabling flexible data structures like linked lists, trees, and dynamic arrays.

## Core APIs Visual Map

```
                    USER PROGRAM
                         │
        ┌────────────────┼────────────────┐
        │                │                │
    malloc()         calloc()         realloc()
        │                │                │
        └────────────────┼────────────────┘
                         │
                    allocator_internal()
                         │
              ┌──────────┴──────────┐
              │   Find free block    │
              │   Split if needed    │
              │   Update metadata    │
              └──────────┬──────────┘
                         │
                    void* pointer
                         │
                    USER USES MEMORY
                         │
                      free()
                         │
              ┌──────────┴──────────┐
              │   Validate pointer   │
              │   Mark as free       │
              │   Coalesce blocks    │
              └──────────┬──────────┘
                         │
              Return to free pool
```

## Detailed Memory Block Structure

Every allocated block has metadata stored BEFORE the user pointer:

```
MEMORY BLOCK STRUCTURE
═══════════════════════════════════════════════════════════════════

    ┌──────────────────────────────────────────────────────────┐
    │                    METADATA (Hidden)                      │
    ├────────────┬────────────┬────────────┬───────────────────┤
    │ size (8B)  │ next (8B)  │ prev (8B)  │ magic (4B) │ used │
    ├────────────┴────────────┴────────────┴────────────┴──────┤
    │                    PADDING (for alignment)                │
    ├──────────────────────────────────────────────────────────┤
    │                                                           │
    │                 USER DATA (returned to caller)            │
    │                                                           │
    │                      ⬆                                     │
    │                   user_ptr                                │
    │                                                           │
    └──────────────────────────────────────────────────────────┘

    Total metadata overhead: 28-32 bytes (64-bit system)
```

### Actual Memory Layout Example

```
Address        Content                    Description
─────────────────────────────────────────────────────────────────
0x1000     │ 64        │ ← Block size (including metadata)
0x1008     │ 0x2000    │ ← Next pointer (free list)
0x1010     │ 0x0       │ ← Previous pointer
0x1018     │ 0xDEADBEEF│ ← Magic number (integrity check)
0x101C     │ 1         │ ← Used flag
0x1020     │           │
    ⬇      │  USER     │ ← Returned to caller
0x1060     │   DATA    │
           │           │
```

## 1. malloc() - Memory Allocation

### Function Signature
```c
void* malloc(size_t size);
```

### Purpose
Allocates `size` bytes of uninitialized memory from the heap.

### Visual Flow

```
malloc(100) CALLED
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 1: Align request size                               │
│  Request: 100 bytes                                        │
│  + Metadata: 32 bytes                                      │
│  + Alignment: 4 bytes (to 8-byte boundary)                │
│  Total needed: 136 bytes                                   │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 2: Search free list for suitable block              │
│                                                            │
│  FREE LIST (before allocation):                           │
│  ┌──────┐    ┌──────┐    ┌──────┐                         │
│  │ 200B │───→│ 500B │───→│ 1000B│                         │
│  │ FREE │    │ FREE │    │ FREE │                         │
│  └──────┘    └──────┘    └──────┘                         │
│       ↑                                      ↑             │
│       └────────── 200B is sufficient ─────────┘             │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 3: Split block if too large                         │
│                                                            │
│  Before split:                                             │
│  ┌────────────────────────────────────┐                   │
│  │ 200 bytes total                     │                   │
│  └────────────────────────────────────┘                   │
│                                                            │
│  After split (need 136 bytes):                            │
│  ┌──────────────┬──────────────────────┐                  │
│  │ 136 bytes    │ 64 bytes remaining   │                  │
│  │ (allocate)   │ (free)               │                  │
│  └──────────────┴──────────────────────┘                  │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 4: Mark as used, update metadata                    │
│                                                            │
│  ┌──────────────────────────────────────────┐             │
│  │ size=136 │ next=NULL │ used=1 │ magic=OK │             │
│  ├──────────────────────────────────────────┤             │
│  │         USER DATA (100 bytes)             │             │
│  │         (contents = garbage)              │             │
│  └──────────────────────────────────────────┘             │
└───────────────────────────────────────────────────────────┘
        │
        ▼
    return pointer to user data area (metadata + 32)
```

### Key Characteristics

| Aspect | Description |
|--------|-------------|
| **Initialization** | Uninitialized (contains garbage values) |
| **Failure return** | NULL |
| **Size 0** | Implementation-defined (often NULL or unique pointer) |
| **Time complexity** | O(n) where n = number of free blocks |
| **Memory overhead** | 16-32 bytes per allocation |

### Common Pitfalls

```
❌ WRONG: Not checking return value
    void* ptr = malloc(1000);
    ptr[0] = 10;  // CRASH if malloc failed!

✅ CORRECT: Always check
    void* ptr = malloc(1000);
    if (ptr == NULL) {
        // Handle allocation failure
        return ERROR;
    }
    ptr[0] = 10;

❌ WRONG: Using uninitialized memory
    int* arr = malloc(10 * sizeof(int));
    printf("%d", arr[0]);  // Garbage value!

✅ CORRECT: Initialize immediately
    int* arr = malloc(10 * sizeof(int));
    for(int i = 0; i < 10; i++) arr[i] = 0;
    // Or use calloc()
```

## 2. calloc() - Contiguous Allocation

### Function Signature
```c
void* calloc(size_t num, size_t size);
```

### Purpose
Allocates memory for an array of `num` elements, each `size` bytes, and initializes all bytes to zero.

### Visual Flow

```
calloc(5, 8) CALLED  (5 elements of 8 bytes = 40 bytes)
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 1: Calculate total size                             │
│  total = num * size = 5 * 8 = 40 bytes                    │
│  Check for integer overflow!                              │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 2: Call malloc internally                           │
│  void* ptr = malloc(40);                                  │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 3: Zero-initialize memory                           │
│                                                            │
│  Before (malloc):        After (calloc):                  │
│  ┌────────────────┐      ┌────────────────┐              │
│  │ 0xAB │ 0xCD    │      │ 0x00 │ 0x00    │              │
│  │ 0x12 │ 0x34    │ ───→ │ 0x00 │ 0x00    │              │
│  │ 0xFE │ 0xDC    │      │ 0x00 │ 0x00    │              │
│  │ 0xBA │ 0x98    │      │ 0x00 │ 0x00    │              │
│  └────────────────┘      └────────────────┘              │
│   (garbage values)        (all zeros)                     │
└───────────────────────────────────────────────────────────┘
        │
        ▼
    return pointer to zero-initialized memory
```

### malloc() vs calloc() Comparison

```
malloc(40)                      calloc(5, 8)
─────────────────────────────────────────────────────────────
Memory:  ┌──────────┐           Memory:  ┌──────────┐
         │ 0xAB12   │                    │ 0x0000   │
         │ 0xCD34   │                    │ 0x0000   │
         │ 0xEF56   │                    │ 0x0000   │
         │ 0x789A   │                    │ 0x0000   │
         └──────────┘                    └──────────┘
         ⚠️ GARBAGE                      ✓ ZEROED

Speed:   Faster (no zeroing)     Speed:   Slower (zeroing overhead)
Use:     When you'll write       Use:     When you need zeros
         immediately                      or for arrays
```

### Overflow Protection

```c
// CRITICAL: calloc checks for overflow
size_t num = SIZE_MAX / 2;
size_t size = 3;

// malloc: may overflow silently
void* p1 = malloc(num * size);  // DANGEROUS! May wrap around

// calloc: detects overflow and returns NULL
void* p2 = calloc(num, size);   // SAFE! Returns NULL on overflow
```

## 3. realloc() - Reallocation

### Function Signature
```c
void* realloc(void* ptr, size_t new_size);
```

### Purpose
Resizes previously allocated memory block, preserving existing data.

### Visual Flow - Four Scenarios

```
SCENARIO 1: Expanding into free space
═══════════════════════════════════════════════════════════════

Before realloc(ptr, 200):
┌──────────────┬──────────────┬──────────────┐
│  100 bytes   │  150 bytes   │  200 bytes   │
│  (allocated) │   (free)     │   (free)     │
└──────────────┴──────────────┴──────────────┘
      ↑
     ptr

After realloc (adjacent free space):
┌────────────────────────┬──────────────┐
│      250 bytes         │  200 bytes   │
│   (reallocated)        │   (free)     │
└────────────────────────┴──────────────┘
      ↑
   returns SAME pointer (ptr)


SCENARIO 2: Expanding - Need to move
═══════════════════════════════════════════════════════════════

Before realloc(ptr, 300):
┌──────────────┬──────────────┬──────────────┐
│  100 bytes   │   50 bytes   │  400 bytes   │
│  (allocated) │   (free)     │   (free)     │
└──────────────┴──────────────┴──────────────┘
      ↑
     ptr

After realloc (must move):
┌──────────────┬──────────────┬────────────────┐
│   50 bytes   │  400 bytes   │   300 bytes    │
│   (free)     │   (free)     │ (reallocated)  │
└──────────────┴──────────────┴────────────────┘
                                   ↑
                           returns NEW pointer
            (data copied from old location)


SCENARIO 3: Shrinking
═══════════════════════════════════════════════════════════════

Before realloc(ptr, 50):
┌────────────────────────────┬──────────────┐
│        200 bytes           │  100 bytes   │
│      (allocated)           │   (free)     │
└────────────────────────────┴──────────────┘
      ↑
     ptr

After realloc (split block):
┌──────────────┬──────────────┬──────────────┐
│   50 bytes   │  150 bytes   │  100 bytes   │
│ (reallocated)│   (free)     │   (free)     │
└──────────────┴──────────────┴──────────────┘
      ↑
   returns SAME pointer


SCENARIO 4: realloc(NULL, size) = malloc(size)
═══════════════════════════════════════════════════════════════
realloc(NULL, 100)  →  malloc(100)  ✓

realloc(ptr, 0)     →  free(ptr) and return NULL
```

### Complete realloc Flow Diagram

```
realloc(ptr, new_size) CALLED
            │
            ▼
    ┌───────────────┐
    │ ptr == NULL?  │
    └───────────────┘
         │        │
        YES       NO
         │        │
         ▼        ▼
    malloc()   Get current block size
    new_size       │
         │        ▼
         │   ┌─────────────┐
         │   │ new_size    │
         │   │ == 0?       │
         │   └─────────────┘
         │        │        │
         │       YES       NO
         │        │        │
         │        ▼        ▼
         │    free(ptr)   Can expand
         │    return NULL  in place?
         │        │        │     │
         │        │       YES    NO
         │        │        │     │
         │        │        ▼     ▼
         │        │    Expand    Allocate
         │        │    in place  new block
         │        │        │     │
         │        │        │     ▼
         │        │        │   Copy data
         │        │        │     │
         │        │        │     ▼
         │        │        │   free(ptr)
         │        │        │     │
         └────────┴────────┴─────┘
                        │
                        ▼
                  return pointer
```

### realloc Example with Visualization

```c
// Example: Growing dynamic array
int* arr = malloc(3 * sizeof(int));
arr[0] = 10; arr[1] = 20; arr[2] = 30;

// Memory layout:
// [10][20][30] _ _ _ _ _ _ _ _ _ _ (free space)

// Grow to 10 elements
arr = realloc(arr, 10 * sizeof(int));
// Original data preserved: [10][20][30][?][?][?][?][?][?][?]

// Add more elements
arr[3] = 40; arr[4] = 50;
// [10][20][30][40][50][_][_][_][_][_]
```

### Important realloc Behaviors

| Condition | Behavior |
|-----------|----------|
| `realloc(NULL, size)` | Same as `malloc(size)` |
| `realloc(ptr, 0)` | Same as `free(ptr)` and returns NULL |
| `realloc(ptr, smaller_size)` | May shrink, may return same pointer |
| `realloc(ptr, larger_size)` | May move if can't expand in place |
| Allocation fails | Returns NULL, original ptr VALID (not freed!) |

### CRITICAL Pitfall

```c
❌ DANGEROUS - Memory leak on failure
ptr = realloc(ptr, new_size);  // If realloc fails, ptr becomes NULL
                               // Original memory LOST!

✅ SAFE - Always use temporary pointer
void* new_ptr = realloc(ptr, new_size);
if (new_ptr == NULL) {
    // Handle error, ptr still valid
    return ERROR;
}
ptr = new_ptr;
```

## 4. free() - Memory Deallocation

### Function Signature
```c
void free(void* ptr);
```

### Purpose
Returns previously allocated memory back to the heap's free pool.

### Visual Flow

```
free(ptr) CALLED
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 1: Validate pointer                                 │
│                                                            │
│  if (ptr == NULL) return;  // Free(NULL) is safe          │
│                                                            │
│  Locate metadata: block = ptr - sizeof(metadata)          │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 2: Integrity checks                                 │
│                                                            │
│  Check magic number:                                      │
│  if (block->magic != MAGIC_ALLOCATED) {                   │
│      // Double free or corruption                         │
│      abort();                                             │
│  }                                                        │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 3: Poison memory (debug mode)                       │
│                                                            │
│  Fill user data with 0xDEADBEEF pattern:                  │
│  ┌──────────────────────┐                                 │
│  │ Original: "Hello"    │                                 │
│  │ After poison: 0xDE   │  ← Catches use-after-free       │
│  │             0xAD      │                                 │
│  └──────────────────────┘                                 │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 4: Mark as free                                     │
│                                                            │
│  block->used = 0;                                         │
│  block->magic = MAGIC_FREE;                               │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 5: Coalesce with adjacent free blocks               │
│                                                            │
│  Before coalescing:                                       │
│  ┌──────┬──────┬──────┬──────┬──────┐                    │
│  │ FREE │ USED │ FREE │ FREE │ USED │                    │
│  │ 100B │ 50B  │ 75B  │ 60B  │ 200B │                    │
│  └──────┴──────┴──────┴──────┴──────┘                    │
│            ↑                                              │
│         freeing this                                      │
│                                                            │
│  After coalescing (merge with neighbors):                 │
│  ┌──────┬─────────────┬──────┐                           │
│  │ FREE │    FREE     │ USED │                           │
│  │ 100B │ 185B total  │ 200B │                           │
│  └──────┴─────────────┴──────┘                           │
└───────────────────────────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────────────────────────┐
│  STEP 6: Insert into free list                            │
│                                                            │
│  Maintain sorted order for better coalescing:             │
│                                                            │
│  FREE LIST HEAD → [100B] → [185B] → [500B] → NULL        │
│                         ↑                                 │
│                   new block inserted                      │
└───────────────────────────────────────────────────────────┘
```

### Free List Coalescing Visualization

```
SCENARIO: Freeing middle block
═══════════════════════════════════════════════════════════════

Initial state:
┌────────────────────────────────────────────────────────────┐
│ Block A     │ Block B     │ Block C     │ Block D         │
│ 100 bytes   │ 50 bytes    │ 75 bytes    │ 200 bytes       │
│ FREE        │ ALLOCATED   │ FREE        │ ALLOCATED       │
└────────────────────────────────────────────────────────────┘
                    ↑
                 freeing B

Step 1: Mark B as free
┌────────────────────────────────────────────────────────────┐
│ Block A     │ Block B     │ Block C     │ Block D         │
│ FREE        │ FREE        │ FREE        │ ALLOCATED       │
└────────────────────────────────────────────────────────────┘

Step 2: Coalesce A+B+C into single block
┌──────────────────────────────────┬────────────────────────┐
│        Combined Block            │        Block D         │
│    225 bytes TOTAL               │    200 bytes           │
│         FREE                     │     ALLOCATED          │
└──────────────────────────────────┴────────────────────────┘

Result: 3 small blocks → 1 large block (reduced fragmentation)
```

### Memory Poisoning Visualization

```
BEFORE FREE (valid data):
┌────────────────────────────────────────────────────────────┐
│ Metadata │ 'H' │ 'e' │ 'l' │ 'l' │ 'o' │ '\0' │ padding   │
└────────────────────────────────────────────────────────────┘

AFTER FREE (poisoned):
┌────────────────────────────────────────────────────────────┐
│ Metadata │ 0xDE │ 0xAD │ 0xBE │ 0xEF │ 0xDE │ 0xAD │ 0xBE │
└────────────────────────────────────────────────────────────┘
           ╘══════════════════════════════════════════════╛
                       Use-after-free detection pattern
```

## Complete Allocation/Deallocation Cycle

```
PROGRAM EXECUTION FLOW
═══════════════════════════════════════════════════════════════

1. Initial heap (all free)
   ┌────────────────────────────────────────────────────────┐
   │                                                        │
   │                   FREE (1MB total)                      │
   │                                                        │
   └────────────────────────────────────────────────────────┘

2. After malloc(100)
   ┌──────────────┬─────────────────────────────────────────┐
   │  Allocated   │              FREE                        │
   │   100 bytes  │                                         │
   └──────────────┴─────────────────────────────────────────┘

3. After malloc(200)
   ┌──────┬────────────┬────────────────────────────────────┐
   │ 100B │   200B     │              FREE                   │
   │ used │   used     │                                    │
   └──────┴────────────┴────────────────────────────────────┘

4. After free(first 100B)
   ┌──────┬────────────┬────────────────────────────────────┐
   │ FREE │   200B     │              FREE                   │
   │ 100B │   used     │                                    │
   └──────┴────────────┴────────────────────────────────────┘
     ↑
  Fragmentation! (hole between used blocks)

5. After malloc(150) - allocator finds suitable block
   ┌──────────────────┬────────────┬────────────────────────┐
   │     150B         │   200B     │         FREE           │
   │     used         │   used     │                        │
   └──────────────────┴────────────┴────────────────────────┘
   (Used the 100B free block? No - too small)
   (Used the large free block at end? Yes - 150 fits)

Result: 100B hole remains (internal fragmentation)
```

## Common Errors and Detection

### Error Types Visualization

```
1. MEMORY LEAK
   ┌──────┐     ┌──────┐     ┌──────┐
   │ malloc │──→│ use  │     └──────┘
   └──────┘     └──────┘     (never freed)
   ✓ allocated  ✓ used      ✗ LEAKED

2. DOUBLE FREE
   free(ptr) → free(ptr)
   ┌──────────────────────────────────────┐
   │ First free:  ✓ works                 │
   │ Second free: ✗ CRASH (corruption)    │
   └──────────────────────────────────────┘

3. USE AFTER FREE
   ptr = malloc(100);
   free(ptr);
   ptr[0] = 10;  ← WRONG! Undefined behavior
   ┌──────────────────────────────────────┐
   │ May crash, corrupt data, or work     │
   │ unpredictably                        │
   └──────────────────────────────────────┘

4. BUFFER OVERFLOW
   malloc(100) → write 150 bytes
   ┌────────────┬─────────────────────────┐
   │ User area  │ Metadata (corrupted!)   │
   │ (100B)     │ next ptr, size, magic   │
   └────────────┴─────────────────────────┘
                     ↑
              Next malloc/free crashes
```

## Best Practices Summary

### DO's ✅
- Always check return value of malloc/calloc/realloc
- Use temporary pointer for realloc
- Free memory in reverse order of allocation
- Set freed pointers to NULL
- Use calloc for arrays and sensitive data
- Check for integer overflow in size calculations

### DON'Ts ❌
- Don't access memory after freeing
- Don't free the same pointer twice
- Don't free stack variables
- Don't free middle of allocated block
- Don't ignore allocation failures
- Don't assume malloc returns zeroed memory

## Debugging Tools

| Tool | Purpose | Example |
|------|---------|---------|
| **Valgrind** | Detect leaks, invalid access | `valgrind ./program` |
| **Address Sanitizer** | Heap buffer overflow | `gcc -fsanitize=address` |
| **MALLOC_CHECK_** | Glibc heap checking | `MALLOC_CHECK_=1 ./program` |
| **mtrace** | Trace malloc/free calls | `mtrace ./program` |

This comprehensive guide covers everything about dynamic memory APIs in C with visual explanations perfect for interview preparation!
