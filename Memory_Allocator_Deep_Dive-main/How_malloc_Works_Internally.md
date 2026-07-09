# How malloc Works Internally 

## Overview

`malloc` is not a system call but a library function that manages heap memory. It acts as a memory broker between your program and the operating system, requesting large chunks of memory from the OS and efficiently subdividing them for your allocations.

## High-Level Architecture

```
MALLOC ARCHITECTURE OVERVIEW
═══════════════════════════════════════════════════════════════════

                    USER PROGRAM
                         │
                    malloc(100)
                         │
                         ▼
              ┌─────────────────────┐
              │    malloc()         │
              │    (libc)           │
              └──────────┬──────────┘
                         │
                         ▼
              ┌─────────────────────┐
              │  Check free list    │
              │  for suitable block │
              └──────────┬──────────┘
                         │
            ┌────────────┴────────────┐
            │                         │
         Found?                     Not Found?
            │                         │
            ▼                         ▼
    ┌──────────────┐          ┌────────────────┐
    │ Return block │          │ Request memory │
    │ to user      │          │ from OS        │
    └──────────────┘          └────────┬───────┘
                                       │
                         ┌─────────────┴─────────────┐
                         │                           │
                    < 128KB?                      >= 128KB?
                         │                           │
                         ▼                           ▼
                  sbrk/brk                        mmap()
                  (extend heap)                   (separate mapping)
```

## Complete malloc Workflow

### Step-by-Step Allocation Process

```
malloc(100) CALLED
═══════════════════════════════════════════════════════════════════

STEP 1: Align Request Size
┌─────────────────────────────────────────────────────────────────┐
│  Requested: 100 bytes                                           │
│  + Metadata: 32 bytes (size, next, prev, magic, used flag)     │
│  + Alignment: 4 bytes (to 8-byte boundary)                     │
│  Total needed: 136 bytes                                        │
│                                                                 │
│  requested_size = align_to_8bytes(100 + BLOCK_METADATA_SIZE)   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
STEP 2: Check Free List
┌─────────────────────────────────────────────────────────────────┐
│  Free list traversal (first-fit algorithm):                    │
│                                                                 │
│  Free List Head → ┌──────┐    ┌──────┐    ┌──────┐             │
│                   │ 200B │───→│ 500B │───→│ 1000B│             │
│                   │ FREE │    │ FREE │    │ FREE │             │
│                   └──────┘    └──────┘    └──────┘             │
│                        ↑                                        │
│                   Found! (200 >= 136)                           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
STEP 3: Remove from Free List
┌─────────────────────────────────────────────────────────────────┐
│  Before:                                                        │
│  free_list → [block A] → [block B] → [block C]                 │
│                   ↑                                             │
│              removing this                                      │
│                                                                 │
│  After:                                                         │
│  free_list → [block B] → [block C]                             │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
STEP 4: Split Block (if too large)
┌─────────────────────────────────────────────────────────────────┐
│  Block size: 200 bytes                                          │
│  Needed: 136 bytes                                              │
│  Remainder: 64 bytes                                            │
│                                                                 │
│  Before split:                                                  │
│  ┌────────────────────────────────────────────────────────┐    │
│  │                    200 bytes                           │    │
│  └────────────────────────────────────────────────────────┘    │
│                                                                 │
│  After split:                                                   │
│  ┌──────────────────────┬─────────────────────────────┐         │
│  │     136 bytes        │        64 bytes             │         │
│  │   (will allocate)    │    (becomes new free block) │         │
│  └──────────────────────┴─────────────────────────────┘         │
│                                                                 │
│  New free block is reinserted into free list                    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
STEP 5: Mark as Used & Return
┌─────────────────────────────────────────────────────────────────┐
│  Block header configuration:                                    │
│  ┌────────┬────────┬─────────┬────────┬────────┐              │
│  │ size=136│next=NULL│prev=NULL│magic=OK │used=1 │              │
│  └────────┴────────┴─────────┴────────┴────────┘              │
│  ┌────────────────────────────────────────────────────────┐    │
│  │            User Data (100 bytes)                        │    │
│  └────────────────────────────────────────────────────────┘    │
│                            ↑                                    │
│                    Return this address                          │
│                    (block + sizeof(metadata))                   │
└─────────────────────────────────────────────────────────────────┘
```

## Memory Request from OS

### When Free List Has No Suitable Block

```
NO SUITABLE BLOCK FOUND
═══════════════════════════════════════════════════════════════════

                malloc(1000) - No free block large enough
                              │
                              ▼
              ┌───────────────────────────────┐
              │   Request memory from OS      │
              └───────────────┬───────────────┘
                              │
            ┌─────────────────┴─────────────────┐
            │                                   │
        < MMMAP_THRESHOLD                   >= MMMAP_THRESHOLD
        (typically 128KB)                   (large allocation)
            │                                   │
            ▼                                   ▼
    ┌───────────────┐                   ┌───────────────┐
    │    sbrk()     │                   │    mmap()     │
    │  Move program │                   │  Anonymous    │
    │  break        │                   │  mapping      │
    └───────────────┘                   └───────────────┘
```

### sbrk/brk - Heap Extension

```
PROGRAM BREAK (brk) MECHANISM
═══════════════════════════════════════════════════════════════════

Initial process memory layout:
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS ├──────────────────────────┤ Stack          │
│                      ↑                        ↑                │
│                 current_brk              stack pointer          │
└─────────────────────────────────────────────────────────────────┘

After sbrk(1MB):
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │◄─── NEW HEAP ───►│         Stack          │
│                      ↑                 ↑                        │
│                 old_brk           new_brk                       │
└─────────────────────────────────────────────────────────────────┘

Visual sbrk operation:
┌─────────────────────────────────────────────────────────────────┐
│  void* old_brk = sbrk(0);          // Get current break        │
│  if (sbrk(size_increase) == (void*)-1) {                       │
│      return NULL;                    // Out of memory!         │
│  }                                                              │
│  void* new_memory = old_brk;         // Return old break        │
└─────────────────────────────────────────────────────────────────┘

Heap before:
    [memory in use] [free]
                    ↑
                 brk point

After sbrk(increase):
    [memory in use] [free] [◄──── new heap space ────►]
                    ↑                           ↑
                 old_brk                     new_brk
```

### mmap - Large Allocations

```
MMAP FOR LARGE ALLOCATIONS
═══════════════════════════════════════════════════════════════════

Why use mmap for large allocations (>128KB)?
┌─────────────────────────────────────────────────────────────────┐
│  1. Can be freed independently (doesn't affect heap)           │
│  2. Can be returned to OS immediately on free()                │
│  3. Doesn't cause heap fragmentation                           │
│  4. Better for large, short-lived allocations                  │
└─────────────────────────────────────────────────────────────────┘

Memory layout with mmap:
┌─────────────────────────────────────────────────────────────────┐
│  Heap (sbrk)      │  mmap region 1  │  mmap region 2  │ Stack   │
│  [data][free]     │  [1MB buffer]   │  [512KB buffer] │         │
└─────────────────────────────────────────────────────────────────┘
                           ↑
                    Separate mapping, can be
                    unmapped independently

mmap allocation process:
┌─────────────────────────────────────────────────────────────────┐
│  void* ptr = mmap(NULL, size,                                   │
│                   PROT_READ | PROT_WRITE,                       │
│                   MAP_PRIVATE | MAP_ANONYMOUS,                  │
│                   -1, 0);                                       │
│  if (ptr == MAP_FAILED) {                                       │
│      return NULL;                                               │
│  }                                                              │
│  return ptr;                                                    │
└─────────────────────────────────────────────────────────────────┘
```

## Core Data Structures

### Block Metadata Layout

```
BLOCK METADATA STRUCTURE (64-bit system)
═══════════════════════════════════════════════════════════════════

Typical memory block layout:

Address         Content                    Size    Description
───────────────────────────────────────────────────────────────────
0x1000      ┌─────────────────────┐
            │  size               │  8 bytes  Total block size
0x1008      │  next               │  8 bytes  Next block in free list
0x1010      │  prev               │  8 bytes  Previous block in free list
0x1018      │  magic              │  4 bytes  Corruption detection
0x101C      │  used               │  1 byte   Allocation status
0x101D      │  padding            │  3 bytes  Alignment
0x1020      ├─────────────────────┤
            │                     │
            │  USER DATA          │  size-32  Returned to caller
            │  (aligned to 8)     │
            │                     │
            └─────────────────────┘

Total metadata overhead: 32 bytes per allocation
```

### Free List Structure

```
FREE LIST LINKED LIST
═══════════════════════════════════════════════════════════════════

Free list maintains blocks in address order (typically):

    free_list (head)
         │
         ▼
    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
    │ Block A     │    │ Block B     │    │ Block C     │
    │ size: 200   │    │ size: 450   │    │ size: 128   │
    │ next ───────┼───→│ next ───────┼───→│ next ───────┼───→ NULL
    │ prev: NULL  │←───│ prev ───────┼───←│ prev ───────┘
    │ used: 0     │    │ used: 0     │    │ used: 0     │
    └─────────────┘    └─────────────┘    └─────────────┘
    [mem.........]     [mem.........]     [mem.........]

Each free block contains:
- Size of block (including header)
- Next pointer (to next free block)
- Previous pointer (to previous free block)
- User data area (unused, potentially filled with poison)
```

## Block Splitting in Detail

### When and Why Splitting Happens

```
BLOCK SPLITTING VISUALIZATION
═══════════════════════════════════════════════════════════════════

Situation: Found 512-byte free block, need only 128 bytes

BEFORE SPLIT (512-byte block):
┌─────────────────────────────────────────────────────────────────┐
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Metadata  │                                             │  │
│  │  size=512  │           FREE SPACE (480 bytes)           │  │
│  │  used=0    │                                             │  │
│  └────────────┴─────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘

DECISION: Split if remainder >= MIN_BLOCK_SIZE (typically 32 bytes)
Remainder = 512 - 128 = 384 bytes (much larger than 32 → SPLIT!)

AFTER SPLIT:
┌─────────────────────────────────────────────────────────────────┐
│  ┌──────────────┬───────────────────────────────────────────┐  │
│  │  Allocated   │            New Free Block                 │  │
│  │  Block       │                                           │  │
│  │  size=128    │  size=384                                 │  │
│  │  used=1      │  used=0                                   │  │
│  └──────────────┴───────────────────────────────────────────┘  │
│         ↑                                   ↑                   │
│    Return to user                    Insert into free list      │
└─────────────────────────────────────────────────────────────────┘

Splitting prevents waste:
- Without split: 384 bytes wasted (internal fragmentation)
- With split: 384 bytes available for future allocations
```

### Split Algorithm

```c
// Simplified split logic
static block_t* split_block(block_t* block, size_t needed_size) {
    size_t remaining = block->size - needed_size;
    
    // Only split if remainder can be a valid block
    if (remaining < MIN_BLOCK_SIZE) {
        return NULL;  // Don't split - keep as one block
    }
    
    // Create new block after current
    block_t* new_block = (block_t*)((char*)block + needed_size);
    new_block->size = remaining;
    new_block->used = 0;
    new_block->magic = MAGIC_FREE;
    new_block->next = block->next;
    new_block->prev = block;
    
    // Update original block
    block->size = needed_size;
    block->next = new_block;
    
    return new_block;
}
```

## Block Coalescing

### Merging Adjacent Free Blocks

```
COALESCING VISUALIZATION
═══════════════════════════════════════════════════════════════════

SCENARIO: Free block between two allocated blocks initially

Initial state:
┌──────────┬──────────┬──────────┬──────────┬──────────┐
│ Block A  │ Block B  │ Block C  │ Block D  │ Block E  │
│ ALLOC    │ FREE     │ ALLOC    │ FREE     │ ALLOC    │
│ 100B     │ 50B      │ 200B     │ 75B      │ 150B     │
└──────────┴──────────┴──────────┴──────────┴──────────┘

Now free Block C (the allocated block between free blocks):
         ↓
┌──────────┬──────────┬──────────┬──────────┬──────────┐
│ Block A  │ Block B  │ Block C  │ Block D  │ Block E  │
│ ALLOC    │ FREE     │ FREE     │ FREE     │ ALLOC    │
│ 100B     │ 50B      │ 200B     │ 75B      │ 150B     │
└──────────┴──────────┴──────────┴──────────┴──────────┘
                     ↑
                 Mark as FREE

Coalescing B + C + D (adjacent free blocks):
┌──────────┬────────────────────────────┬──────────┐
│ Block A  │     Combined Free Block    │ Block E  │
│ ALLOC    │     B+C+D = 325 bytes      │ ALLOC    │
│ 100B     │                            │ 150B     │
└──────────┴────────────────────────────┴──────────┘

Result: Three small free blocks become one large free block
Benefits: Reduces fragmentation, enables larger allocations
```

### Coalescing Algorithm

```c
static block_t* coalesce(block_t* block) {
    // Coalesce with next block if it's free
    if (block->next && !block->next->used) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Coalesce with previous block if it's free
    if (block->prev && !block->prev->used) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }
    
    return block;
}
```

## Free List Management

### Insertion Strategies

```
FREE LIST INSERTION STRATEGIES
═══════════════════════════════════════════════════════════════════

STRATEGY 1: LIFO (Last-In-First-Out)
┌─────────────────────────────────────────────────────────────────┐
│  Insert at head: O(1) but causes fragmentation                  │
│                                                                 │
│  free_list → [new] → [old1] → [old2] → NULL                    │
│              ↑                                                  │
│          fast insert                                            │
└─────────────────────────────────────────────────────────────────┘

STRATEGY 2: Address-Ordered (Most common)
┌─────────────────────────────────────────────────────────────────┐
│  Maintain sorted by address for better coalescing              │
│                                                                 │
│  Insert 300-byte block between 200B and 450B blocks:           │
│                                                                 │
│  Before: [200B] ⇄ [450B] ⇄ [1000B]                             │
│                                                                 │
│  After:  [200B] ⇄ [300B] ⇄ [450B] ⇄ [1000B]                    │
│               ↑                                                 │
│          inserted here (by address)                             │
└─────────────────────────────────────────────────────────────────┘

STRATEGY 3: Size-Ordered
┌─────────────────────────────────────────────────────────────────┐
│  Maintain sorted by size for best-fit algorithm                │
│                                                                 │
│  Insert 300B into: [100B] → [200B] → [500B] → [1000B]          │
│                                                                 │
│  After:     [100B] → [200B] → [300B] → [500B] → [1000B]        │
└─────────────────────────────────────────────────────────────────┘
```

## Fragmentation Types

### External vs Internal Fragmentation

```
FRAGMENTATION VISUALIZATION
═══════════════════════════════════════════════════════════════════

EXTERNAL FRAGMENTATION (Wasted space BETWEEN blocks)
┌─────────────────────────────────────────────────────────────────┐
│  Memory with many small holes:                                 │
│                                                                 │
│  ┌────┬─┬─────┬─┬────┬─┬─────┬─┬────┐                         │
│  │USER│F│USER │F│USER│F│USER │F│USER│                         │
│  │100B│8│200B │4│150B│6│300B │2│100B│                         │
│  └────┴─┴─────┴─┴────┴─┴─────┴─┴────┘                         │
│        ↑      ↑      ↑      ↑                                  │
│   Holes too small to use (wasted)                              │
│                                                                 │
│  Problem: Total free memory = 20 bytes, but can't allocate     │
│           16 bytes because holes are not contiguous!           │
└─────────────────────────────────────────────────────────────────┘

INTERNAL FRAGMENTATION (Wasted space INSIDE block)
┌─────────────────────────────────────────────────────────────────┐
│  Allocation rounding:                                          │
│                                                                 │
│  Request: 100 bytes                                            │
│  Actual allocated: 136 bytes (after alignment + metadata)      │
│  Wasted: 36 bytes inside the allocated block                   │
│                                                                 │
│  ┌──────────────────────────────────────────┐                 │
│  │  Metadata  │  User Data  │   WASTED      │                 │
│  │  32 bytes  │  100 bytes  │   4 bytes     │                 │
│  └──────────────────────────────────────────┘                 │
│                                            ↑                    │
│                                    Internal fragmentation      │
└─────────────────────────────────────────────────────────────────┘
```

## malloc Implementations Comparison

### Popular malloc Implementations

```
IMPLEMENTATION COMPARISON
═══════════════════════════════════════════════════════════════════

ptmalloc2 (glibc - Linux default)
┌─────────────────────────────────────────────────────────────────┐
│  • Based on dlmalloc (Doug Lea)                                │
│  • Multiple arenas for thread scalability                      │
│  • 64 bins (small, large, fast bins)                          │
│  • Uses mmap for >128KB allocations                           │
│  • Best for general purpose, multi-threaded apps              │
└─────────────────────────────────────────────────────────────────┘

jemalloc (Firefox, Rust, FreeBSD)
┌─────────────────────────────────────────────────────────────────┐
│  • Arena-based with per-thread caches                          │
│  • Red-black trees for large allocations                       │
│  • Excellent for multi-threaded, high-performance apps        │
│  • Better fragmentation resistance                             │
└─────────────────────────────────────────────────────────────────┘

tcmalloc (Google)
┌─────────────────────────────────────────────────────────────────┐
│  • Thread-caching malloc                                       │
│  • Per-thread caches for small allocations                     │
│  • Central heap managed by page-level allocator                │
│  • Very fast for multi-threaded applications                   │
└─────────────────────────────────────────────────────────────────┘

dmalloc (Debug malloc)
┌─────────────────────────────────────────────────────────────────┐
│  • Designed for debugging                                      │
│  • Adds extra guards for buffer overflow detection             │
│  • Slower but catches errors                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Complete free() Workflow

```
free(ptr) DEEP DIVE
═══════════════════════════════════════════════════════════════════

free(ptr) CALLED
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 1: Locate metadata                                       │
│  block = (block_t*)ptr - 1;                                    │
│  Validate magic number                                         │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 2: Poison memory (debug mode)                            │
│  memset(ptr, 0xDEADBEEF, block->size - sizeof(block_t));      │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 3: Mark as free                                          │
│  block->used = 0;                                              │
│  block->magic = MAGIC_FREE;                                    │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 4: Coalesce with neighbors                               │
│  if (next block is free)  → merge                             │
│  if (prev block is free)  → merge                             │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 5: Insert into free list (address-ordered)               │
│  Find position in free list                                    │
│  Insert maintaining sorted order                               │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  STEP 6: Consider returning to OS                              │
│  If top-most free block > threshold (typically 128KB):        │
│      sbrk(-size) or munmap()                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Performance Optimization Techniques

### Fastbins and Small Bin Caches

```
BIN STRUCTURE IN ptmalloc
═══════════════════════════════════════════════════════════════════

Fastbins (LIFO, no coalescing):
┌─────────────────────────────────────────────────────────────────┐
│  Fixed-size bins for common small allocations:                 │
│                                                                 │
│  fastbin[0]: 16 bytes  →  [block] → [block] → NULL            │
│  fastbin[1]: 24 bytes  →  [block] → NULL                       │
│  fastbin[2]: 32 bytes  →  [block] → [block] → [block] → NULL  │
│  fastbin[3]: 40 bytes  →  NULL                                 │
│  ...                                                           │
│  fastbin[9]: 80 bytes  →  [block] → NULL                       │
└─────────────────────────────────────────────────────────────────┘

Small bins (FIFO, coalescing):
┌─────────────────────────────────────────────────────────────────┐
│  Doubly-linked lists for small sizes:                          │
│                                                                 │
│  bin[0]: 17-24 bytes  →  [block] ⇄ [block] ⇄ NULL             │
│  bin[1]: 25-32 bytes  →  NULL                                  │
│  bin[2]: 33-40 bytes  →  [block] ⇄ NULL                        │
└─────────────────────────────────────────────────────────────────┘

Large bins (Range-based):
┌─────────────────────────────────────────────────────────────────┐
│  Sorted by size for best-fit:                                  │
│                                                                 │
│  [200B] ⇄ [350B] ⇄ [512B] ⇄ [1024B] ⇄ [2048B]                 │
└─────────────────────────────────────────────────────────────────┘
```

## Memory Allocation Performance Numbers

```
TYPICAL PERFORMANCE BENCHMARKS
═══════════════════════════════════════════════════════════════════

Operation                    Time (nanoseconds)
───────────────────────────────────────────────────────────────────
malloc(16) (from fastbin)        30-50 ns
malloc(128) (from small bin)     50-80 ns
malloc(4096) (from large bin)    100-150 ns
malloc(1MB) (from mmap)          500-1000 ns

free(16) (to fastbin)            20-40 ns
free(128) (with coalescing)      60-100 ns
free(1MB) (munmap)               400-800 ns

Cache effects:
┌─────────────────────────────────────────────────────────────────┐
│  L1 cache hit:   4 cycles (1-2 ns)                             │
│  L2 cache hit:  12 cycles (3-4 ns)                             │
│  L3 cache hit:  40 cycles (10-12 ns)                           │
│  RAM access:   200+ cycles (50-70 ns)                          │
└─────────────────────────────────────────────────────────────────┘
```

## Common Optimization Techniques

### Arena/Bin Strategy

```
ARENA STRATEGY FOR MULTI-THREADING
═══════════════════════════════════════════════════════════════════

Main Arena (thread 1):
┌─────────────────────────────────────────────────────────────────┐
│  [Heap]                                                         │
Thread 2 → Thread 2 Arena:
┌─────────────────────────────────────────────────────────────────┐
│  [Heap]                                                         │
Thread 3 → Thread 3 Arena:
┌─────────────────────────────────────────────────────────────────┐
│  [Heap]                                                         │

Benefits:
✓ No lock contention between threads
✓ Each thread has its own heap
✓ Memory can be transferred between arenas
```

### Memory Pooling

```
MEMORY POOL FOR FIXED-SIZE ALLOCATIONS
═══════════════════════════════════════════════════════════════════

Traditional malloc (slow, fragmented):
┌─────────────────────────────────────────────────────────────────┐
│  Allocation 1  │ Allocation 2  │ Free  │ Allocation 3  │ Free  │
└─────────────────────────────────────────────────────────────────┘
        ↑              ↑                              ↑
     different       different                     different
      sizes           sizes                          sizes

Memory pool (fast, no fragmentation):
┌─────────────────────────────────────────────────────────────────┐
│  [64B] [64B] [64B] [64B] [64B] [64B] [64B] [64B] [64B] [64B]   │
│   FREE   USED  USED  FREE   USED  FREE   USED   USED  FREE  USED │
└─────────────────────────────────────────────────────────────────┘
        ↑
   All same size, no fragmentation

Speedup: 10-100x faster than malloc for fixed-size objects
```

## Debugging malloc Issues

### Common Detection Techniques

```
HEAP CORRUPTION DETECTION
═══════════════════════════════════════════════════════════════════

1. Magic Numbers:
┌─────────────────────────────────────────────────────────────────┐
│  Block header contains magic value (e.g., 0xDEADBEEF)          │
│  On free, verify magic still matches                           │
│  If mismatch → buffer overflow overwrote metadata!             │
└─────────────────────────────────────────────────────────────────┘

2. Guard Pages (for large allocations):
┌─────────────────────────────────────────────────────────────────┐
│  Allocate with guard pages before/after:                       │
│  [PROT_NONE page] [Allocated data] [PROT_NONE page]           │
│  Accessing beyond bounds → segmentation fault immediately!     │
└─────────────────────────────────────────────────────────────────┘

3. Memory Poisoning:
┌─────────────────────────────────────────────────────────────────┐
│  Fill freed memory with 0xDEADBEEF pattern                     │
│  Detect use-after-free: accessing poisoned memory → crash      │
└─────────────────────────────────────────────────────────────────┘

4. Malloc Hooks (glibc):
┌─────────────────────────────────────────────────────────────────┐
│  __malloc_hook, __free_hook, __realloc_hook                   │
│  Intercept allocations for debugging/tracing                   │
└─────────────────────────────────────────────────────────────────┘
```

## Interview Questions & Answers

### Q: Why does malloc need metadata?

```
METADATA PURPOSE BREAKDOWN
═══════════════════════════════════════════════════════════════════

size:    Know how much memory to free (delete[] needs this!)
next:    Maintain free list linked structure
prev:    Bidirectional traversal for coalescing
magic:   Detect buffer overflows and double frees
used:    Quick allocation status check

Without metadata:
✗ Can't know block size on free()
✗ Can't find adjacent blocks for coalescing
✗ Can't detect corruption
✗ Can't maintain free list
```

### Q: What causes memory fragmentation?

```
FRAGMENTATION CAUSES
═══════════════════════════════════════════════════════════════════

Pattern that causes fragmentation:
┌─────────────────────────────────────────────────────────────────┐
│  1. allocate A (100B)   [A][free]                              │
│  2. allocate B (200B)   [A][B][free]                           │
│  3. allocate C (150B)   [A][B][C][free]                        │
│  4. free B              [A][free][C][free]  ← Holes created!   │
│  5. allocate D (250B)   Cannot fit anywhere!                   │
└─────────────────────────────────────────────────────────────────┘

Prevention strategies:
✓ Use memory pools for same-size objects
✓ Free in reverse allocation order (LIFO)
✓ Use best-fit algorithm
✓ Use slab allocators for frequent structures
```

### Q: How does realloc avoid copying?

```
REALLOC OPTIMIZATION
═══════════════════════════════════════════════════════════════════

realloc(ptr, new_size) looks for:

SCENARIO 1: Expand in place
Before:  [block] [free space]
After:   [block expanded] [less free]

SCENARIO 2: Shrink in place  
Before:  [block] [free]
After:   [smaller block] [new free block]

SCENARIO 3: Move (only when necessary)
Before:  [block] [used] [free]
After:   [free] [used] [new block] (data copied)

realloc avoids copy by:
1. Checking if adjacent space is free
2. Merging with free neighbors
3. Only copying when absolutely necessary
```

This comprehensive guide covers everything about malloc internals with detailed visualizations perfect for deep technical interviews!
