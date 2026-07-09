# Fragmentation - Complete Guide

## Overview

Fragmentation is the enemy of memory allocators. It occurs when memory becomes divided into small, unusable pieces, wasting space and causing allocation failures even when total free memory exceeds the request size.

## The Two Types of Fragmentation

```
FRAGMENTATION TYPES AT A GLANCE
═══════════════════════════════════════════════════════════════════

                    INTERNAL FRAGMENTATION
                    ┌─────────────────────────┐
                    │  Wasted space INSIDE    │
                    │  an allocated block     │
                    └─────────────────────────┘

                    EXTERNAL FRAGMENTATION
                    ┌─────────────────────────┐
                    │  Wasted space BETWEEN   │
                    │  allocated blocks       │
                    └─────────────────────────┘
```

## 1. Internal Fragmentation - Deep Dive

### Definition
Memory wasted **inside** an allocated block because the allocator gave you more than you requested.

### Visualization

```
INTERNAL FRAGMENTATION VISUALIZED
═══════════════════════════════════════════════════════════════════

You ask for: 10 bytes
Allocator gives: 16 bytes (due to alignment)

┌─────────────────────────────────────────────────────────────────┐
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                     16 bytes total                       │  │
│  │  ┌──────────────┬────────────────────────────────────┐   │  │
│  │  │  Metadata    │  Your Data (10 bytes)  │  WASTED   │   │  │
│  │  │   (6 bytes)  │                        │  (0 bytes?)│   │  │
│  │  └──────────────┴────────────────────────┴───────────┘   │  │
│  │       ↑                           ↑                      │  │
│  │   Minimum overhead            Unused space               │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘

Wait! Internal fragmentation = The unused space in allocated block
But metadata is necessary overhead, not fragmentation!

CORRECT VIEW:
┌─────────────────────────────────────────────────────────────────┐
│  Request: 10 bytes                                             │
│  Allocation: 32 bytes (8-byte aligned + 16 byte metadata)      │
│  Actually usable: 24 bytes (after metadata)                    │
│  Internal fragmentation = 14 bytes (24 - 10)                   │
│                                                                 │
│  ┌────────────────────────────────────────────────────────┐    │
│  │  Meta  │     Your usable space (24 bytes)             │    │
│  │ 8 bytes│  ┌──────────────────┬──────────────────────┐ │    │
│  │        │  │  Your data 10B   │  Internal Waste 14B  │ │    │
│  │        │  └──────────────────┴──────────────────────┘ │    │
│  └────────────────────────────────────────────────────────┘    │
│           ↑                                                    │
│    Memory you can actually use                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Real-World Example

```c
// Example showing internal fragmentation
struct align_example {
    char c;        // 1 byte
    int i;         // 4 bytes  
    double d;      // 8 bytes
};  // Total logical size: 13 bytes
    // Actual size with padding: 16 bytes (3 bytes internal fragmentation)

// Memory layout:
┌───────┬─────────┬─────────┬─────────┬─────────┬─────────┐
│  c    │ padding │    i    │ padding │    d    │ padding │
│ 1 byte│ 3 bytes │ 4 bytes │ 0 bytes │ 8 bytes │ 0 bytes │
└───────┴─────────┴─────────┴─────────┴─────────┴─────────┘
         ↑
    Internal fragmentation
```

### Causes of Internal Fragmentation

```
CAUSES VISUALIZATION
═══════════════════════════════════════════════════════════════════

1. ALIGNMENT REQUIREMENTS
┌─────────────────────────────────────────────────────────────────┐
│  CPU requires 8-byte alignment for certain types               │
│  Allocator rounds up: 5 → 8, 12 → 16, 20 → 24                  │
│  Waste: 3, 4, 4 bytes respectively                             │
└─────────────────────────────────────────────────────────────────┘

2. MINIMUM ALLOCATION SIZE
┌─────────────────────────────────────────────────────────────────┐
│  Allocator has minimum block size (typically 16-32 bytes)      │
│  Request 1 byte → get 32 bytes → 31 bytes wasted!              │
└─────────────────────────────────────────────────────────────────┘

3. METADATA OVERHEAD
┌─────────────────────────────────────────────────────────────────┐
│  Each allocation needs metadata (size, flags, etc.)           │
│  Minimum block = metadata + min user data                      │
│  Small allocations have high waste percentage                  │
└─────────────────────────────────────────────────────────────────┘

4. POWER-OF-TWO BUDDY SYSTEM
┌─────────────────────────────────────────────────────────────────┐
│  Request 100 bytes → allocate 128 bytes → 28 bytes waste       │
│  Request 500 bytes → allocate 512 bytes → 12 bytes waste       │
│  Request 1000 bytes → allocate 1024 bytes → 24 bytes waste     │
└─────────────────────────────────────────────────────────────────┘
```

### Internal Fragmentation Calculation

```c
// Calculate waste percentage
size_t request = 10;
size_t allocated = 32;  // After alignment + metadata
size_t overhead = 16;   // Metadata size
size_t usable = allocated - overhead;  // 16 bytes
size_t waste = usable - request;        // 6 bytes
float waste_percent = (float)waste / allocated * 100;  // 18.75%

printf("Waste: %zu bytes (%.1f%%)\n", waste, waste_percent);

// Real glibc example:
// malloc(1) → actually allocates 32 bytes on 64-bit system
// Waste: 31 bytes (96.9% waste!)
```

## 2. External Fragmentation - Deep Dive

### Definition
Free memory exists but is split into small, non-contiguous pieces that cannot satisfy a large allocation request.

### The Classic Example

```
EXTERNAL FRAGMENTATION - CLASSIC SCENARIO
═══════════════════════════════════════════════════════════════════

Total free memory: 12 bytes (4+4+4 = 12)
Request: 10 bytes
Result: FAIL! (no contiguous 10-byte block)

Memory layout:
┌─────────────────────────────────────────────────────────────────┐
│ Address 0    4    8    12   16   20   24   28   32             │
│ ┌────┬────┬────┬────┬────┬────┬────┬────┬────┐                │
│ │ A  │FREE│ B  │FREE│ C  │FREE│ D  │ E  │    │                │
│ │4B  │4B  │4B  │4B  │4B  │4B  │4B  │4B  │    │                │
│ └────┴────┴────┴────┴────┴────┴────┴────┴────┘                │
│      ↑         ↑         ↑                                     │
│   4-byte    4-byte    4-byte                                   │
│    hole      hole      hole                                    │
│                                                                 │
│ CANNOT ALLOCATE 10 BYTES!                                      │
│ Even though 12 bytes free, none is contiguous!                 │
└─────────────────────────────────────────────────────────────────┘
```

### How External Fragmentation Happens

```
FRAGMENTATION CREATION PROCESS
═══════════════════════════════════════════════════════════════════

Timeline of allocations and frees:

STEP 1: Initial state
┌─────────────────────────────────────────────────────────────────┐
│                         FREE (1000 bytes)                       │
└─────────────────────────────────────────────────────────────────┘

STEP 2: Allocate A (100 bytes)
┌──────────────────────────────┬──────────────────────────────────┐
│         A (100B)             │          FREE (900B)             │
└──────────────────────────────┴──────────────────────────────────┘

STEP 3: Allocate B (200 bytes)  
┌──────────┬───────────────────┬──────────────────────────────────┐
│ A (100B) │    B (200B)       │          FREE (700B)             │
└──────────┴───────────────────┴──────────────────────────────────┘

STEP 4: Allocate C (300 bytes)
┌──────────┬──────────┬─────────────────────┬────────────────────┐
│ A (100B) │ B (200B) │      C (300B)       │     FREE (400B)    │
└──────────┴──────────┴─────────────────────┴────────────────────┘

STEP 5: Allocate D (150 bytes)
┌────┬─────┬───────┬──────────┬──────────────────┬───────────────┐
│ A  │ B   │   C   │ D (150B) │      FREE        │   FREE        │
│100B│200B │ 300B  │          │     (250B)       │   (400B)??    │
└────┴─────┴───────┴──────────┴──────────────────┴───────────────┘
Wait, layout messed up - let me fix:

Correct timeline:
┌──────────┬──────────┬─────────────────────┬────────────────────┐
│ A (100B) │ B (200B) │      C (300B)       │     FREE (400B)    │
└──────────┴──────────┴─────────────────────┴────────────────────┘
                           ↓
              Allocate D (150 bytes) - takes from FREE
┌──────────┬──────────┬──────────┬──────────┬───────────────────┐
│ A (100B) │ B (200B) │ D (150B) │ C (150B) │    FREE (250B)??  │
└──────────┴──────────┴──────────┴──────────┴───────────────────┘
No - C was 300B, but we split it?

Better to show properly:


CORRECT SEQUENCE:
═══════════════════════════════════════════════════════════════════

1. After A, B, C:
   ┌────────┬─────────┬────────────────────────┬─────────────────┐
   │ A(100) │  B(200) │       C(300)           │   FREE(400)     │
   └────────┴─────────┴────────────────────────┴─────────────────┘

2. Free B:
   ┌────────┬─────────┬────────────────────────┬─────────────────┐
   │ A(100) │ FREE(200)│       C(300)           │   FREE(400)     │
   └────────┴─────────┴────────────────────────┴─────────────────┘

3. Allocate D(250) - uses part of the 200B free? Too small.
                - Uses part of 400B free at end:
   ┌────────┬─────────┬────────────────────────┬──────────┬──────┐
   │ A(100) │ FREE(200)│       C(300)           │ D(250)   │FREE │
   └────────┴─────────┴────────────────────────┴──────────┴──────┘
                                                    (150B left)

4. Free A:
   ┌────────┬─────────┬────────────────────────┬──────────┬──────┐
   │FREE(100)│ FREE(200)│       C(300)          │ D(250)   │FREE │
   └────────┴─────────┴────────────────────────┴──────────┴──────┘
                                                       (150B)

5. Try to allocate E(350):
   Total free = 100+200+150 = 450 bytes
   But NO contiguous 350-byte block!
   ┌────┬─────┬────────────────────────┬──────────┬──────┐
   │100 │ 200 │       C(300)           │ D(250)   │ 150 │  ← Two fragments
   │FREE│ FREE│       USED              │ USED     │ FREE│
   └────┴─────┴────────────────────────┴──────────┴──────┘
     ↑              ↑                         ↑
   Fragment 1    Large used block        Fragment 2
   (300B total)  blocks coalescing      (150B)
   BUT: 100+200 not adjacent to 150!

Result: FRAGMENTATION - 450B free, 350B allocation fails!
```

### Visual Fragmentation Metric

```
FRAGMENTATION INDEX
═══════════════════════════════════════════════════════════════════

Fragmentation = 1 - (largest_free_block / total_free_memory)

Example 1 (No fragmentation):
Total free: 1000B, Largest free: 1000B
Fragmentation = 1 - (1000/1000) = 0.0 (Perfect!)

Example 2 (Some fragmentation):
Total free: 1000B, Largest free: 400B  
Fragmentation = 1 - (400/1000) = 0.6 (60% fragmented)

Example 3 (Bad fragmentation):
Total free: 1000B, Largest free: 100B
Fragmentation = 1 - (100/1000) = 0.9 (90% fragmented!)

Visual representation:
┌─────────────────────────────────────────────────────────────────┐
│  Best:    [════════════════════════════════════] 0% frag       │
│  Good:    [══════════][free][══][free][══]       40% frag      │
│  Bad:     [═][free][═][free][═][free][═][free]   75% frag      │
│  Worst:   [═][free][═][free][═][free][═][free]   90%+ frag     │
└─────────────────────────────────────────────────────────────────┘
```

## Fragmentation Comparison

### Internal vs External

```
COMPARISON TABLE
═══════════════════════════════════════════════════════════════════

Aspect              Internal                    External
───────────────────────────────────────────────────────────────────
Location            Inside allocated block      Between allocated blocks

Visibility          Hidden from program         Visible in free list

Cause               Alignment, min size,        Allocation/free pattern
                    overhead                    

Waste type          Space you can't use         Space you can't reach

Solution            Better alignment, pools,    Compaction, better
                    custom allocators           algorithms, buddy system

Detection           Calculate (allocated -      Measure largest free
                    requested)                  block vs total free

Prevention          Use slab/pool allocators    Use buddy system or
                    for small objects           coalescing

Impact severity     High for small objects      High for long-running
                    (90%+ waste possible)       servers

Test example:       malloc(1) → 32B given       Free 4+4+4 → can't get 10
                    31B waste (96.9%)           contiguous
```

## Solutions to Fragmentation

## Solution 1: Buddy Allocator

### How Buddy Allocator Works

```
BUDDY ALLOCATOR CONCEPT
═══════════════════════════════════════════════════════════════════

Core idea: Split memory into power-of-two blocks only

Initial: 1024 bytes (2^10)
┌─────────────────────────────────────────────────────────────────┐
│                        1024 (FREE)                              │
└─────────────────────────────────────────────────────────────────┘

Allocate 200 bytes (needs 256 = 2^8):
┌───────────────────────────────┬─────────────────────────────────┐
│         256 (allocated)       │         768 (FREE)              │
└───────────────────────────────┴─────────────────────────────────┘

768 is not power of two - split:
┌───────────────┬───────────────┬─────────────────────────────────┐
│ 256 (alloc)   │ 256 (free)    │         512 (FREE)              │
└───────────────┴───────────────┴─────────────────────────────────┘

Allocate another 200 (needs 256):
┌───────────────┬───────────────┬───────────────┬─────────────────┐
│ 256 (alloc)   │ 256 (alloc)   │ 256 (free)    │ 256 (free)      │
└───────────────┴───────────────┴───────────────┴─────────────────┘

Free first block:
┌───────────────┬───────────────┬───────────────┬─────────────────┐
│ 256 (free)    │ 256 (alloc)   │ 256 (free)    │ 256 (free)      │
└───────────────┴───────────────┴───────────────┴─────────────────┘

Buddy coalescing (adjacent free blocks of same size):
Check if block 0 and 1 are buddies and both free → block 1 is allocated, so no
Check if block 2 and 3 are buddies and both free → YES! merge to 512
┌───────────────┬───────────────┬─────────────────────────────────┐
│ 256 (free)    │ 256 (alloc)   │         512 (FREE)              │
└───────────────┴───────────────┴─────────────────────────────────┘
```

### Buddy Allocator Visualization

```
COMPLETE BUDDY SYSTEM EXAMPLE
═══════════════════════════════════════════════════════════════════

Memory size: 1MB (2^20)

Initial free list:
order[20]: [0MB-1MB]
order[19]: empty
order[18]: empty
...

Request 150KB (needs 256KB = 2^18):
Split 1MB → 512KB + 512KB (order 19)
Split first 512KB → 256KB + 256KB (order 18)
Allocate first 256KB

Result:
┌────────────┬────────────────────────────────────────────────────┐
│  256KB     │                   768KB FREE                       │
│ ALLOCATED  │   (split into 256KB + 512KB for future)           │
└────────────┴────────────────────────────────────────────────────┘

Free list after:
order[18]: [256KB-512KB] (second 256KB block)
order[19]: [512KB-1MB]   (512KB block)
order[20]: empty

Allocate another 200KB (needs 256KB):
Take from order[18] (256KB-512KB range)
Result:
┌────────────┬────────────┬──────────────────────────────────────┐
│  256KB     │  256KB     │               512KB FREE             │
│ ALLOC1     │ ALLOC2     │                                      │
└────────────┴────────────┴──────────────────────────────────────┘

Free ALLOC1:
Check buddy (256KB-512KB range) - still allocated, can't coalesce
Result:
┌────────────┬────────────┬──────────────────────────────────────┐
│  256KB     │  256KB     │               512KB FREE             │
│ FREE       │ ALLOC2     │                                      │
└────────────┴────────────┴──────────────────────────────────────┘

Free ALLOC2:
Both 256KB blocks are free and are buddies → coalesce to 512KB
Result:
┌────────────────────────────┬──────────────────────────────────┐
│         512KB FREE         │           512KB FREE             │
└────────────────────────────┴──────────────────────────────────┘
These 512KB blocks are buddies → coalesce to 1MB FREE!

Benefit: No external fragmentation at the cost of internal fragmentation
```

### Buddy Allocator Code Example

```c
// Simplified buddy allocator
#define MAX_ORDER 20  // 2^20 = 1MB
#define MIN_ORDER 4   // 2^4 = 16 bytes

struct buddy_allocator {
    void* memory;
    struct list_head free_lists[MAX_ORDER + 1];
    size_t total_size;
};

// Allocate memory
void* buddy_alloc(struct buddy_allocator* buddy, size_t size) {
    int order = ceil_log2(size);  // Find smallest power of 2 >= size
    
    // Find available block
    int current_order = order;
    while (current_order <= MAX_ORDER) {
        if (!list_empty(&buddy->free_lists[current_order])) {
            // Found block, remove from free list
            struct block* block = list_pop(&buddy->free_lists[current_order]);
            
            // Split down to required order
            while (current_order > order) {
                current_order--;
                // Split block into two buddies
                struct block* buddy_block = (struct block*)((char*)block + (1 << current_order));
                list_add(&buddy->free_lists[current_order], buddy_block);
            }
            return block;
        }
        current_order++;
    }
    return NULL;  // Out of memory
}

// Free memory - coalescing happens automatically
void buddy_free(struct buddy_allocator* buddy, void* ptr, size_t size) {
    int order = ceil_log2(size);
    struct block* block = ptr;
    
    // Try to coalesce with buddy
    while (order < MAX_ORDER) {
        struct block* buddy = get_buddy(block, order);
        if (!is_free(buddy, order)) break;  // Buddy not free
        
        // Remove buddy from its free list
        list_remove(buddy);
        
        // Coalesce: block becomes the merged block
        block = (block < buddy) ? block : buddy;
        order++;
    }
    
    // Add to appropriate free list
    list_add(&buddy->free_lists[order], block);
}
```

## Solution 2: Slab Allocator

### Slab Allocator Concept

```
SLAB ALLOCATOR DESIGN
═══════════════════════════════════════════════════════════════════

Core idea: Cache frequently used object sizes

┌─────────────────────────────────────────────────────────────────┐
│                          KERNEL SLAB                            │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Slab for struct task_struct (8KB objects)              │   │
│  │  ┌────────┬────────┬────────┬────────┬────────┐       │   │
│  │  │ obj[0] │ obj[1] │ obj[2] │ obj[3] │ obj[4] │       │   │
│  │  │ FREE   │ USED   │ USED   │ FREE   │ USED   │       │   │
│  │  └────────┴────────┴────────┴────────┴────────┘       │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Slab for struct file (256KB objects)                   │   │
│  │  ┌────────┬────────┬────────┬────────┐                 │   │
│  │  │ obj[0] │ obj[1] │ obj[2] │ obj[3] │                 │   │
│  │  │ USED   │ USED   │ FREE   │ USED   │                 │   │
│  │  └────────┴────────┴────────┴────────┘                 │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### How Slab Allocator Works

```
SLAB INTERNALS
═══════════════════════════════════════════════════════════════════

Slab for 64-byte objects (100 objects per slab):

Slab structure:
┌─────────────────────────────────────────────────────────────────┐
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Slab Header                                              │  │
│  │  - next slab pointer                                      │  │
│  │  - objects in use count                                   │  │
│  │  - free list pointer                                      │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │  Object 0  │ Object 1  │ Object 2  │ ... │ Object 99    │  │
│  │  64 bytes  │ 64 bytes  │ 64 bytes  │     │ 64 bytes     │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘

Free list management:
┌─────────────────────────────────────────────────────────────────┐
│  Each free object points to next free object:                  │
│                                                                 │
│  free_ptr → [obj 5] → [obj 12] → [obj 33] → NULL              │
│             (5)       (12)       (33)                          │
│                                                                 │
│  Allocation: O(1) - just return free_ptr and update           │
│  Free: O(1) - add object to front of free list                │
└─────────────────────────────────────────────────────────────────┘

Cache coloring (to avoid cache contention):
┌─────────────────────────────────────────────────────────────────┐
│  Slab 0: starts at offset 0                                    │
│  Slab 1: starts at offset 64 (to use different cache lines)    │
│  Slab 2: starts at offset 128                                  │
│                                                                 │
│  Prevents multiple slabs from mapping to same cache lines      │
└─────────────────────────────────────────────────────────────────┘
```

### Slab Allocator Benefits

```
WHY SLAB ALLOCATOR IS GREAT
═══════════════════════════════════════════════════════════════════

Problem without slab:
┌─────────────────────────────────────────────────────────────────┐
│  Many task_struct allocations (1000s/sec):                     │
│  • Each malloc has metadata overhead (32 bytes)                │
│  • Each free causes fragmentation                               │
│  • Cache misses from scattered allocations                     │
└─────────────────────────────────────────────────────────────────┘

With slab allocator:
┌─────────────────────────────────────────────────────────────────┐
│  • No per-object metadata (handled by slab)                    │
│  • Objects in contiguous array (cache friendly)                │
│  • O(1) allocation/free                                        │
│  • No fragmentation within slab                                │
│  • Object reuse keeps caches warm                              │
└─────────────────────────────────────────────────────────────────┘

Performance numbers:
┌─────────────────────────────────────────────────────────────────┐
│  Operation           malloc/free        Slab allocator         │
│  ─────────────────────────────────────────────────────────────  │
│  Allocate 64 bytes       80 ns               15 ns             │
│  Free 64 bytes           60 ns               12 ns             │
│  Cache misses            high                low               │
│  Fragmentation           moderate            none              │
└─────────────────────────────────────────────────────────────────┘
```

## Solution 3: Pool Allocator

### Pool Allocator Concept

```
POOL ALLOCATOR - SIMPLE & EFFECTIVE
═══════════════════════════════════════════════════════════════════

Fixed-size block pool:

┌─────────────────────────────────────────────────────────────────┐
│                         MEMORY POOL                             │
│  Total size: 100 blocks of 64 bytes = 6400 bytes               │
│                                                                 │
│  Free bitmap (100 bits = 13 bytes):                            │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ 1 1 0 1 0 0 1 1 1 0 0 0 1 1 0 1 ...                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  Block allocation (bit = 0):                                    │
│  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐          │
│  │ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │ 6  │ 7  │ 8  │ 9  │ ...      │
│  │FREE│USED│USED│FREE│USED│FREE│USED│USED│USED│FREE│          │
│  └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘          │
│    ↑                                                           │
│  Allocate from first free block                                │
└─────────────────────────────────────────────────────────────────┘

Benefits:
┌─────────────────────────────────────────────────────────────────┐
│  ✓ Zero fragmentation (all blocks same size)                   │
│  ✓ O(1) allocation (just find free bit)                        │
│  ✓ O(1) free (just set bit)                                    │
│  ✓ Perfect for message queues, bullet pools, entity systems   │
│  ✓ Very low overhead (just bitmap)                             │
└─────────────────────────────────────────────────────────────────┘
```

### Pool Allocator Implementation

```c
// Simple bitmap pool allocator
typedef struct {
    size_t block_size;
    size_t block_count;
    uint8_t* memory;
    uint8_t* bitmap;  // 1 bit per block
} pool_t;

void* pool_alloc(pool_t* pool) {
    // Find first free block
    for (size_t i = 0; i < pool->block_count; i++) {
        size_t byte_index = i / 8;
        size_t bit_index = i % 8;
        
        if (!(pool->bitmap[byte_index] & (1 << bit_index))) {
            // Mark as used
            pool->bitmap[byte_index] |= (1 << bit_index);
            return &pool->memory[i * pool->block_size];
        }
    }
    return NULL;  // Pool exhausted
}

void pool_free(pool_t* pool, void* ptr) {
    size_t offset = (uint8_t*)ptr - pool->memory;
    size_t block_index = offset / pool->block_size;
    size_t byte_index = block_index / 8;
    size_t bit_index = block_index % 8;
    
    // Mark as free
    pool->bitmap[byte_index] &= ~(1 << bit_index);
}

// For even better performance, use free list:
typedef struct {
    void* memory;
    void* free_list;  // Linked list of free blocks
    size_t block_size;
} fast_pool_t;

void* fast_pool_alloc(fast_pool_t* pool) {
    if (!pool->free_list) return NULL;
    void* block = pool->free_list;
    pool->free_list = *(void**)block;  // Store next pointer in block
    return block;
}

void fast_pool_free(fast_pool_t* pool, void* ptr) {
    *(void**)ptr = pool->free_list;  // Store next pointer
    pool->free_list = ptr;
}
```

## Allocation Strategies Comparison

### Which to Use When

```
DECISION TREE FOR ALLOCATOR CHOICE
═══════════════════════════════════════════════════════════════════

                    START
                      │
                      ▼
            ┌─────────────────┐
            │ Fixed size      │
            │ objects?        │
            └─────────────────┘
              │            │
             YES           NO
              │            │
              ▼            ▼
      ┌───────────┐  ┌─────────────────┐
      │Pool/Slab  │  │ Variable size   │
      │Allocator  │  │ allocations?    │
      └───────────┘  └─────────────────┘
                        │            │
                      Small        Large
                      (avg)        (size)
                        │            │
                        ▼            ▼
              ┌─────────────┐  ┌──────────┐
              │ Buddy       │  │ General  │
              │ System      │  │ malloc   │
              └─────────────┘  └──────────┘

Use cases:
┌─────────────────────────────────────────────────────────────────┐
│  Pool Allocator:                                               │
│  • Game entities (particles, enemies, bullets)                │
│  • Network packets (fixed size buffers)                       │
│  • Message queues                                              │
│                                                                 │
│  Slab Allocator:                                               │
│  • Kernel objects (task_struct, inodes, dentries)             │
│  • Database connection pools                                   │
│  • Object caches                                               │
│                                                                 │
│  Buddy Allocator:                                              │
│  • Physical memory management (kernel page allocator)         │
│  • Memory-mapped files                                         │
│  • DMA buffers                                                │
│                                                                 │
│  General malloc:                                               │
│  • General purpose applications                                │
│  • Variable-sized, unpredictable allocations                   │
└─────────────────────────────────────────────────────────────────┘
```

## Fragmentation Prevention Strategies

### Best Practices

```
FRAGMENTATION PREVENTION CHECKLIST
═══════════════════════════════════════════════════════════════════

✅ 1. Use Pool Allocators for Fixed Sizes
   - Network buffers, message queues, entity components
   - Completely eliminates external fragmentation

✅ 2. Group Allocations by Size Class
   - Small objects (< 64 bytes): special allocator
   - Medium objects (64-1024 bytes): slab allocator  
   - Large objects (> 1KB): general malloc

✅ 3. Free in Reverse Allocation Order
   - Allocate A, B, C → free C, B, A (LIFO)
   - Prevents creation of many small holes

✅ 4. Use Memory Pools for Temporary Allocations
   - Stack allocator for phase-specific allocations
   - Reset entire pool after phase completes

✅ 5. Avoid Many Small Allocations
   - Batch allocations when possible
   - Use vector instead of linked list (contiguous memory)

✅ 6. Monitor Fragmentation
   - Track largest_free / total_free ratio
   - Alert when fragmentation > 50%

✅ 7. Consider Compaction (w/ relocatable objects)
   - Move objects to consolidate free space
   - Requires handles or relocatable pointers

✅ 8. Use Custom Allocators Per Module
   - Different allocator for different allocation patterns
   - Isolate fragmentation to specific subsystems
```

## Real-World Fragmentation Example

### Long-Running Server Problem

```
SERVER FRAGMENTATION CASE STUDY
═══════════════════════════════════════════════════════════════════

Problem: Web server runs for weeks → performance degrades, then crashes

Analysis after 30 days:
┌─────────────────────────────────────────────────────────────────┐
│  Memory snapshot:                                               │
│  Total heap: 4GB                                               │
│  Free memory: 1.2GB (30% free!)                                │
│  Largest free block: 50KB (0.001% of free!)                   │
│                                                                 │
│  Fragmentation = 1 - (50KB/1.2GB) = 99.996%!                   │
│                                                                 │
│  Allocation patterns:                                          │
│  • Request sizes: 100B, 200B, 400B, 800B, ..., 16KB          │
│  • Allocation order: random sizes, random lifetime            │
│  • 100,000s allocations per second                            │
└─────────────────────────────────────────────────────────────────┘

Heap visualization (simplified):
┌─────────────────────────────────────────────────────────────────┐
│  After 30 days:                                                │
│  [A][free][B][free][C][free][D][free][E][free]...             │
│   8K  4K   16K 8K   32K 16K  64K 32K  128K 64K                │
│                                                                 │
│  Each free block is smaller than the next allocation!         │
│  Cannot satisfy any request > 64KB despite 1.2GB free!        │
└─────────────────────────────────────────────────────────────────┘

Solutions implemented:
┌─────────────────────────────────────────────────────────────────┐
│  1. Pool allocator for common sizes (100B-16KB)                │
│  2. Custom allocator per connection (reset after disconnect)  │
│  3. Slab caches for database objects (user sessions, queries) │
│                                                                 │
│  Result after fixes:                                           │
│  • 0% fragmentation in critical paths                         │
│  • Server runs for years without degradation                  │
└─────────────────────────────────────────────────────────────────┘
```

## Interview Questions & Answers

### Q1: Explain internal vs external fragmentation with real examples.

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Internal Fragmentation = Waste INSIDE allocated blocks
Example: malloc(5) → allocator gives 16 bytes due to alignment
        11 bytes wasted (internal fragmentation)

External Fragmentation = Waste BETWEEN allocated blocks  
Example: Free blocks: 4KB, 4KB, 4KB (total 12KB free)
         Need: 10KB contiguous
         Can't allocate despite 12KB free!

Key distinction:
- Internal: allocator gave you more than you asked
- External: allocator can't give you what you need because free space is split
```

### Q2: How does buddy allocator solve fragmentation?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Buddy allocator eliminates external fragmentation by:
1. Only splitting memory into power-of-two sizes
2. Always merging free buddies back together

Example:
Initial: 1024KB free
Allocate 300KB → allocate 512KB block (256KB waste internal)
Free later → merges with its 512KB buddy back to 1024KB

Result: No external fragmentation ever!
Cost: Internal fragmentation (waste due to rounding up to power of two)
```

### Q3: When is fragmentation most dangerous?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Most dangerous in:
1. Long-running servers (memory slowly fragments over weeks)
2. Embedded systems (no virtual memory to remap)
3. Real-time systems (fragmentation causes unpredictable latency)

Example: Medical device monitoring system
- Runs 24/7 for months
- Gradual fragmentation leads to allocation failures
- Device must reboot → patient monitoring interrupted!

Mitigation: Use pool/slab allocators for all runtime allocations
```

### Q4: Can you have fragmentation in stack?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

No! Stack has NO fragmentation because:
- Stack is LIFO (Last-In-First-Out)
- Allocations/deallocations are perfectly nested
- Only top of stack moves

Example:
function A() { int a; function B() { int b; } }
Allocate a, then b, then free b, then free a
Perfectly sequential → no holes possible

Stack has other limits (size), but never fragmentation!
```

### Q5: How to measure fragmentation?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Fragmentation Metrics:

1. External Fragmentation Ratio:
   frag = 1 - (largest_free / total_free)
   
   Example: total_free = 1000, largest = 100
   frag = 1 - 0.1 = 0.9 (90% fragmented!)

2. Internal Fragmentation Ratio:
   waste = (allocated - requested) / allocated
   
   Example: malloc(10) → allocated 32
   waste = (32-10)/32 = 68.75%

3. Memory Efficiency:
   efficiency = (useful_memory / total_memory) × 100%
   
   Includes both internal + external waste

Implementation:
void measure_fragmentation(allocator_t* a) {
    size_t total_free = get_total_free(a);
    size_t largest = get_largest_free_block(a);
    float fragmentation = 1.0 - (float)largest / total_free;
    printf("Fragmentation: %.1f%%\n", fragmentation * 100);
}
```

This comprehensive guide covers all aspects of fragmentation with detailed visualizations, making it perfect for deep technical interviews!
