# Linux Allocators

## Overview

Linux systems have multiple memory allocator implementations, each optimized for different workloads. Understanding their differences is crucial for performance optimization and debugging memory issues.

## Allocator Landscape

```
LINUX ALLOCATOR ECOSYSTEM
═══════════════════════════════════════════════════════════════════

                    APPLICATION CODE
                           │
                           ▼
              ┌─────────────────────────┐
              │     malloc()/free()     │
              │     (Standard API)      │
              └────────────┬────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│   glibc malloc│  │   jemalloc    │  │   tcmalloc    │
│   (ptmalloc2) │  │               │  │               │
├───────────────┤  ├───────────────┤  ├───────────────┤
│ • Default     │  │ • Facebook   │  │ • Google      │
│ • Arena-based │  │ • Firefox    │  │ • Chrome      │
│ • Bins        │  │ • Rust       │  │ • TensorFlow  │
│ • mmap for    │  │ • FreeBSD    │  │ • Thread-     │
│   large       │  │ • Redis      │  │   caching     │
└───────────────┘  └───────────────┘  └───────────────┘

COMPARISON SUMMARY:
═══════════════════════════════════════════════════════════════════
Feature              glibc malloc    jemalloc        tcmalloc
───────────────────────────────────────────────────────────────────
Thread scaling      Good            Excellent       Excellent
Memory overhead     Moderate        Lower           Lower
Fragmentation       Moderate        Very low        Low
Speed (multi-thread) Good           Very good       Best
Speed (single-thread) Baseline      Slightly slower Faster
Metadata size       16-32 bytes     8-16 bytes      8-16 bytes
Release year        1987 (dlmalloc) 2005            2005
Used by            Most Linux      Firefox, Redis  Chrome, TensorFlow
```

## glibc malloc (ptmalloc2)

### Architecture Overview

```
glibc malloc ARCHITECTURE
═══════════════════════════════════════════════════════════════════

                    MAIN ARENA (Thread 1)
                    ┌─────────────────────────────────────────┐
                    │  Heap 1 (sbrk - up to 2MB default)     │
                    │  ┌───────────────────────────────────┐  │
                    │  │ Fastbins │ Small bins │ Large bins │  │
                    │  └───────────────────────────────────┘  │
                    └─────────────────────────────────────────┘
                                  │
                    ┌─────────────┼─────────────┐
                    │             │             │
                    ▼             ▼             ▼
              THREAD 2        THREAD 3       THREAD 4
              ARENA 2         ARENA 3        ARENA 4
              (mmap'd)        (mmap'd)       (mmap'd)
                    │             │             │
                    ▼             ▼             ▼
              ┌─────────┐   ┌─────────┐   ┌─────────┐
              │ mmapped │   │ mmapped │   │ mmapped │
              │  heap   │   │  heap   │   │  heap   │
              └─────────┘   └─────────┘   └─────────┘

Thread-Arena Assignment:
• Main thread → Main arena (sbrk heap)
• Each thread attempts to get its own arena
• Maximum arenas = 8 × CPU cores
• Arenas are reused when threads exit
• Locking per arena (reduces contention)
```

### Bin Structure Detailed

```
glibc malloc BIN HIERARCHY
═══════════════════════════════════════════════════════════════════

FASTBINS (LIFO, no coalescing)
───────────────────────────────────────────────────────────────────
Fixed-size bins for common small allocations (< 80 bytes)

Fastbin index   Size range    Typical use
───────────────────────────────────────────────────────────────────
fastbin[0]      16 bytes      char[16], small structs
fastbin[1]      24 bytes      short strings
fastbin[2]      32 bytes      int[8], pointers
fastbin[3]      40 bytes      
fastbin[4]      48 bytes      
fastbin[5]      56 bytes      
fastbin[6]      64 bytes      cache lines
fastbin[7]      72 bytes      
fastbin[8]      80 bytes      

Characteristics:
• LIFO (last freed is first allocated) - cache friendly
• No coalescing (kept separate)
• Very fast (no chunk splitting)
• Memory not returned to OS

SMALL BINS (FIFO, coalescing)
───────────────────────────────────────────────────────────────────
Sizes from 80 bytes to 512 bytes (step 8 bytes)
Number of bins: 62

bin[0]:   80-88 bytes
bin[1]:   88-96 bytes  
bin[2]:   96-104 bytes
...
bin[61]: 504-512 bytes

Characteristics:
• FIFO (round-robin) - better fragmentation
• Coalescing performed
• Doubly-linked lists

LARGE BINS (sorted by size)
───────────────────────────────────────────────────────────────────
Sizes > 512 bytes
Number of bins: 32 (ranges)

Characteristics:
• Sorted by size for best-fit
• Range-based bins
• May need to search multiple bins

UNSORTED BIN (recently freed)
───────────────────────────────────────────────────────────────────
• Recently freed chunks (cache)
• Checked before other bins
• Improves performance for short-lived objects
```

### Arena Memory Layout

```
ARENA MEMORY ORGANIZATION
═══════════════════════════════════════════════════════════════════

Main Arena (Thread 1):
┌─────────────────────────────────────────────────────────────────┐
│  sbrk Heap (grows upward)                                     │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  fastbin[0] │ fastbin[1] │ ... │ small bins │ large bins │   │
│  │  (LIFO)     │            │     │ (FIFO)     │ (sorted)    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Top chunk (last remainder)                             │   │
│  │  (expanded via sbrk when needed)                        │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘

Thread Arena (Thread N):
┌─────────────────────────────────────────────────────────────────┐
│  mmap'd Heap (separate memory region)                         │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Arena Header │ Bins │ Fastbins │ Top chunk             │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  • Created when thread first allocates                         │
│  • Grows with additional mmap calls                            │
│  • Freed with munmap when arena empty                          │
│  • Can be reused by other threads                              │
└─────────────────────────────────────────────────────────────────┘
```

### Allocation Path Decision Tree

```
glibc malloc DECISION FLOW
═══════════════════════════════════════════════════════════════════

                    malloc(size)
                         │
                         ▼
              ┌─────────────────────┐
              │  size <= 80 bytes?  │
              └─────────────────────┘
                    │           │
                   YES          NO
                    │           │
                    ▼           ▼
            Check Fastbin    Use Arena
                 │              Lock
                 ▼              │
         ┌──────────────┐      │
         │ Fastbin has  │      ▼
         │ free chunk?  │  ┌─────────────────┐
         └──────────────┘  │ Check small bin │
              │      │     │  range (>512?)  │
             YES     NO    └─────────────────┘
              │      │           │       │
              ▼      ▼          YES      NO
            Return  Continue   (large)  (small)
              │       │          │       │
              │       ▼          ▼       ▼
              │   Check         Use     Use
              │   Unsorted      Large   Small
              │     Bin          Bins    Bins
              │       │          │       │
              │       ▼          ▼       ▼
              │   Best-fit     Best-fit FIFO
              │   search        scan    (first)
              │       │          │       │
              │       └──────────┼───────┘
              │                  │
              │                  ▼
              │         ┌─────────────────┐
              │         │  Found chunk?   │
              │         └─────────────────┘
              │              │         │
              │             YES        NO
              │              │         │
              │              ▼         ▼
              │           Split    Expand with
              │           chunk    sbrk or mmap
              │              │         │
              │              ▼         ▼
              │           Return   New chunk
              │                      from top
              │                        │
              └────────────────────────┘
                         │
                         ▼
                Return to caller
```

### Chunk Structure

```
glibc malloc CHUNK LAYOUT
═══════════════════════════════════════════════════════════════════

Allocated Chunk (in use):
┌─────────────────────────────────────────────────────────────────┐
│  Prev Size (if prev free) │ Size (with flags)                  │
├─────────────────────────────────────────────────────────────────┤
│  User Data (what malloc returns)                              │
│  ...                                                           │
└─────────────────────────────────────────────────────────────────┘

Free Chunk (in bin):
┌─────────────────────────────────────────────────────────────────┐
│  Prev Size                     │ Size (with PREV_INUSE cleared) │
├─────────────────────────────────────────────────────────────────┤
│  fd (forward pointer - next free)                              │
├─────────────────────────────────────────────────────────────────┤
│  bk (backward pointer - prev free)                             │
├─────────────────────────────────────────────────────────────────┤
│  (rest of user data - unused)                                  │
└─────────────────────────────────────────────────────────────────┘

Size Flags (lowest bits):
┌─────────────────────────────────────────────────────────────────┐
│  Bit 0: PREV_INUSE (1 = previous chunk allocated)             │
│  Bit 1: IS_MMAPPED (1 = allocated via mmap)                   │
│  Bit 2: NON_MAIN_ARENA (1 = belongs to thread arena)          │
└─────────────────────────────────────────────────────────────────┘

Visual Example (64-bit):
Allocated 100 bytes:
┌─────────────────────────────────────────────────────────────────┐
│ 0x0: prev_size (0)       │ size = 128 (with flags)           │
│ 0x8: user data (100 bytes)                                    │
│ ...                                                           │
└─────────────────────────────────────────────────────────────────┘

Free 100-byte chunk:
┌─────────────────────────────────────────────────────────────────┐
│ 0x0: prev_size (0)       │ size = 128 (prev_inuse=0)         │
│ 0x8: fd (0x7f123456)     │ bk (0x7f123456)                   │
│ 0x10: user data (unused)                                      │
└─────────────────────────────────────────────────────────────────┘
```

## jemalloc - Next Generation Allocator

### Architecture Overview

```
jemalloc ARCHITECTURE
═══════════════════════════════════════════════════════════════════

                    PROCESS
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
   Thread 1        Thread 2        Thread 3
   Arena 1         Arena 2         Arena 3
   (cache)         (cache)         (cache)
        │              │              │
        └──────────────┼──────────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
    Small bins     Large bins    Huge pages
    (< 4KB)        (< 4MB)       (> 4MB)
        │              │              │
        └──────────────┼──────────────┘
                       │
                       ▼
              ┌─────────────────┐
              │   Page-level    │
              │   Allocator     │
              │   (4KB chunks)  │
              └─────────────────┘

Key Innovations:
• Run-length encoding for metadata
• Red-black trees for large allocations
• Explicit best-fit without searching all bins
• Slab-based small allocation
• Thread-local caches (tcache)
```

### Size Class Hierarchy

```
jemalloc SIZE CLASSES
═══════════════════════════════════════════════════════════════════

SMALL (slab allocated):
───────────────────────────────────────────────────────────────────
Size classes grouped by run size:

Size Group    Examples (typical)
───────────────────────────────────────────────────────────────────
8              8
16             16
32             32
48             48
64             64
80             80
96             96
112            112
128            128
...            ...
3584           3584
4096           4096

LARGE (page-aligned):
───────────────────────────────────────────────────────────────────
8KB, 12KB, 16KB, ..., up to 4MB
(Specific sizes may vary by configuration)

HUGE (mmap'd directly):
───────────────────────────────────────────────────────────────────
> 4MB - mapped directly with mmap
Immediately returned to OS when freed

SIZE CLASS BENEFITS:
═══════════════════════════════════════════════════════════════════
• Reduced internal fragmentation
• Better cache efficiency
• Faster allocation for common sizes
• Compact metadata (per-run, not per-object)
```

### Thread Caching (tcache)

```
jemalloc THREAD CACHE
═══════════════════════════════════════════════════════════════════

Per-thread cache structure:

THREAD 1:
┌─────────────────────────────────────────────────────────────────┐
│  tcache_t                                                      │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ bins[0]: [ptr][ptr][ptr] (3 cached 8-byte objects)       │ │
│  │ bins[1]: [ptr][ptr] (2 cached 16-byte objects)           │ │
│  │ bins[2]: (empty)                                          │ │
│  │ ...                                                       │ │
│  │ bins[N]: (cached large objects)                          │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘

CACHE OPERATIONS:
───────────────────────────────────────────────────────────────────
Allocation:
1. Check tcache bin
2. If available → return (no lock, fast!)
3. Otherwise → fill from arena (batch allocation)

Deallocation:  
1. Add to tcache bin
2. If bin full → flush oldest to arena

CACHE BENEFITS:
═══════════════════════════════════════════════════════════════════
• No locking for most allocations
• 10-100x faster than arena allocation
• Reduces contention dramatically
• Cache size tunable (default ~64KB per thread)

CACHE VISUALIZATION:
───────────────────────────────────────────────────────────────────
Without tcache:         With tcache:
Thread → Arena Lock     Thread → tcache (fast)
   ↓                         ↓
[██████]                [✓✓✓] cache hit
   ↓                         ↓
Return                  Return

Each allocation        No lock needed!
requires lock
```

### Arena Management

```
jemalloc ARENA STRUCTURE
═══════════════════════════════════════════════════════════════════

Arena (per NUMA domain typically):

┌─────────────────────────────────────────────────────────────────┐
│  ARENA                                                         │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  runs_avail[4KB] → red-black tree (small runs)           │ │
│  │  runs_avail[8KB] → red-black tree                        │ │
│  │  runs_avail[12KB] → red-black tree                       │ │
│  │  ...                                                      │ │
│  │  runs_avail[4MB] → red-black tree (large runs)          │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  huge (mmap'd arenas)                                     │ │
│  │  extents_cache (deferred coalescing)                      │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘

KEY FEATURES:
═══════════════════════════════════════════════════════════════════
• Multiple arenas (typically 4× CPU cores)
• NUMA-aware (one arena per memory node)
• Red-black trees for O(log n) large allocation
• Run-length encoded metadata
• Extent coalescing (fewer syscalls)
```

## tcmalloc - Google's Allocator

### Architecture

```
tcmalloc ARCHITECTURE
═══════════════════════════════════════════════════════════════════

                    PROCESS
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
   Thread 1        Thread 2        Thread 3
   Thread Cache    Thread Cache    Thread Cache
   (free list)     (free list)     (free list)
        │              │              │
        └──────────────┼──────────────┘
                       │
                       ▼
              ┌─────────────────┐
              │   Central       │
              │   Free List     │
              │  (binned)       │
              └─────────────────┘
                       │
                       ▼
              ┌─────────────────┐
              │   Page Heap     │
              │ (span list)     │
              └─────────────────┘
                       │
                       ▼
                    mmap()

DESIGN PHILOSOPHY:
═══════════════════════════════════════════════════════════════════
• Optimize for multi-threaded performance
• Minimize locking overhead
• Fast path: thread cache (no locks)
• Small objects aggregated into "spans"
```

### Thread Cache Structure

```
tcmalloc THREAD CACHE
═══════════════════════════════════════════════════════════════════

PER-THREAD CACHE (64KB default):
───────────────────────────────────────────────────────────────────
┌─────────────────────────────────────────────────────────────────┐
│  ThreadCache (per thread)                                      │
│                                                                 │
│  size class 0 (8 bytes):   [obj] [obj] [obj] (list)           │
│  size class 1 (16 bytes):  [obj] [obj] (list)                 │
│  size class 2 (32 bytes):  (empty)                            │
│  size class 3 (48 bytes):  [obj] (list)                       │
│  ...                                                           │
│  size class n (256KB):     (empty)                            │
└─────────────────────────────────────────────────────────────────┘

CACHE MANAGEMENT:
───────────────────────────────────────────────────────────────────
Allocation (fast path):
1. Get size class index
2. Pop from thread cache list
3. Return object (no locks!) 

Deallocation (fast path):
1. Push to thread cache list
2. If list length > limit:
   - Move excess to central free list
   - May release pages

CACHE SIZE:
• Dynamic (adjusts based on usage)
• Steals from other threads when needed
• Central list acts as balancing mechanism
```

### Span-Based Management

```
tcmalloc SPAN CONCEPT
═══════════════════════════════════════════════════════════════════

Memory is organized into 8KB pages; contiguous pages form a "span":

┌─────────────────────────────────────────────────────────────────┐
│  Page Heap (managed by spans)                                 │
│                                                                 │
│  Span 1 (2 pages, free):                                       │
│  ┌─────────┬─────────┐                                         │
│  │ page 0  │ page 1  │                                         │
│  └─────────┴─────────┘                                         │
│                                                                 │
│  Span 2 (1 page, allocated for sizes 8-32):                    │
│  ┌─────────┐                                                   │
│  │ page 2  │ → [8-byte objects] [8-byte] [8-byte] ...         │
│  └─────────┘                                                   │
│                                                                 │
│  Span 3 (4 pages, allocated for sizes 33-64):                  │
│  ┌─────────┬─────────┬─────────┬─────────┐                    │
│  │ page 3  │ page 4  │ page 5  │ page 6  │                    │
│  └─────────┴─────────┴─────────┴─────────┘                    │
│                                                                 │
│  Span 4 (free, 3 pages):                                       │
│  ┌─────────┬─────────┬─────────┐                              │
│  │ page 7  │ page 8  │ page 9  │                              │
│  └─────────┴─────────┴─────────┘                              │
└─────────────────────────────────────────────────────────────────┘

SPAN FEATURES:
═══════════════════════════════════════════════════════════════════
• Each span knows size class (or "free")
• Pages within span are contiguous
• Coalescing: merge adjacent free spans
• O(1) size class lookup
• Minimal metadata (per-span, not per-object)
```

### Size Class Optimization

```
tcmalloc SIZE CLASSES (64-bit)
═══════════════════════════════════════════════════════════════════

CAREFULLY TUNED SIZE CLASSES (example):

Class   Size    Objects/page    Internal fragmentation
───────────────────────────────────────────────────────────────────
0       8       512             0%
1       16      256             0%
2       32      128             0%
3       48      85              14%
4       64      64              0%
5       80      51              1.5%
6       96      42              3%
7       112     36              3.5%
8       128     32              0%
9       144     28              2%
10      160     25              2%
11      176     23              1%
12      192     21              1%
13      208     19              2%
14      224     18              2%
15      240     17              1%
16      256     16              0%
... (up to 256KB)

OPTIMIZATION GOAL:
═══════════════════════════════════════════════════════════════════
• Minimize internal fragmentation
• Fit as many objects as possible per page
• Efficient CPU cache usage
• Avoid odd sizes that waste space

RESULT:
───────────────────────────────────────────────────────────────────
• < 15% internal fragmentation typical
• Excellent cache line alignment
• Fast size class calculation (array lookup)
```

## Performance Comparison

### Benchmark Results

```
ALLOCATOR PERFORMANCE COMPARISON
═══════════════════════════════════════════════════════════════════

SINGLE-THREADED (nanoseconds per operation):
───────────────────────────────────────────────────────────────────
Operation           glibc       jemalloc    tcmalloc
malloc(32)          45          52          38
free(32)            35          48          32
malloc(128)         52          58          45  
free(128)           42          50          40
malloc(1024)        78          85          70
free(1024)          65          72          60

MULTI-THREADED (4 threads, ns/op):
───────────────────────────────────────────────────────────────────
Threads     glibc       jemalloc    tcmalloc
1           48          55          42
2           85          62          48
4           180         75          55
8           320         95          72
16          550         120         95

MEMORY OVERHEAD (bytes per allocation):
───────────────────────────────────────────────────────────────────
Size        glibc       jemalloc    tcmalloc
8           16          8           8
32          16          8           8  
128         16          8           8
1024        16          16          8

FRAGMENTATION (after random alloc/free):
───────────────────────────────────────────────────────────────────
Allocator    External    Internal    Total
glibc        8%          12%         20%
jemalloc     4%          6%          10%
tcmalloc     3%          8%          11%
```

### Workload Suitability

```
ALLOCATOR USE CASE MATRIX
═══════════════════════════════════════════════════════════════════

Workload Type          Best Allocator    Reason
───────────────────────────────────────────────────────────────────
Web Server (nginx)     jemalloc          Long-running, many threads
Database (Redis)       jemalloc          Fragmentation resistance
Browser (Chrome)       tcmalloc          Many small allocations
C++ Applications       tcmalloc          Fast thread caching
General Linux          glibc             Default, good enough
HPC/Simulation         tcmalloc          Speed critical
Embedded               glibc (custom)    Smaller footprint
Real-time              None (pool)       Deterministic needed

RECOMMENDATION FLOW:
───────────────────────────────────────────────────────────────────
                    START
                      │
                      ▼
              Default (glibc)
              works well?
                    │
               ┌────┴────┐
              YES       NO
               │         │
               │         ▼
               │    Multi-threaded?
               │         │
               │    ┌────┴────┐
               │   YES      NO
               │    │         │
               │    ▼         ▼
               │  tcmalloc  jemalloc
               │  (speed)   (fragmentation)
               │    │         │
               └────┴─────────┘
                    │
                    ▼
              Benchmark & Test
```

## Configuration and Tuning

### glibc malloc Tuning

```
GLIBC MALLOC ENVIRONMENT VARIABLES
═══════════════════════════════════════════════════════════════════

MALLOC_ARENA_MAX
───────────────────────────────────────────────────────────────────
Controls maximum number of arenas
Example: export MALLOC_ARENA_MAX=4
Use: Reduce memory overhead for many threads

MALLOC_MMAP_THRESHOLD_
───────────────────────────────────────────────────────────────────
Size threshold for mmap (default 128KB)
Example: export MALLOC_MMAP_THRESHOLD_=262144
Use: Tune mmap usage for workload

MALLOC_MMAP_MAX_
───────────────────────────────────────────────────────────────────
Maximum mmap mappings
Example: export MALLOC_MMAP_MAX_=65536
Use: Prevent too many mmap segments

MALLOC_TRACE
───────────────────────────────────────────────────────────────────
Enable allocation tracing
Example: export MALLOC_TRACE=/tmp/mtrace.log
Use: Debug memory leaks

MALLOC_CHECK_
───────────────────────────────────────────────────────────────────
Heap corruption checking:
0 - ignore (fast)
1 - print errors
2 - abort on error

Example: export MALLOC_CHECK_=2
Use: Debug heap corruption
```

### jemalloc Configuration

```
JEMALLOC TUNING OPTIONS
═══════════════════════════════════════════════════════════════════

Environment Variables:
───────────────────────────────────────────────────────────────────
MALLOC_CONF="narenas:8,tcache:true,dirty_decay_ms:5000"

narenas:8
• Number of arenas
• Default: 4× CPU cores

tcache:true
• Enable thread caching
• Default: true

dirty_decay_ms:5000
• Time before releasing dirty pages
• Longer = better cache, more memory

background_thread:true
• Enable background thread for purging
• Better application responsiveness

lg_tcache_max:15
• Maximum size for thread cache (2^15 = 32KB)

OPTIONS FILE:
───────────────────────────────────────────────────────────────────
/etc/malloc.conf or /etc/jemalloc.conf
```

### tcmalloc Configuration

```
TCMALLOC TUNING
═══════════════════════════════════════════════════════════════════

Environment Variables:
───────────────────────────────────────────────────────────────────
TCMALLOC_MAX_TOTAL_THREAD_CACHE_BYTES
• Total thread cache size (default 32MB)
• export TCMALLOC_MAX_TOTAL_THREAD_CACHE_BYTES=67108864

TCMALLOC_AGGRESSIVE_DECOMMIT
• Release memory to OS aggressively
• export TCMALLOC_AGGRESSIVE_DECOMMIT=true

TCMALLOC_RELEASE_RATE
• Rate of memory release (default 1.0)
• Lower = faster but more memory

Code Configuration:
───────────────────────────────────────────────────────────────────
#include <gperftools/malloc_extension.h>

// Set thread cache size
MallocExtension::instance()->SetMaxTotalThreadCacheBytes(64*1024*1024);

// Release free memory
MallocExtension::instance()->ReleaseFreeMemory();
```

## Switching Allocators

### How to Use Different Allocators

```
CHANGING ALLOCATOR (Linux)
═══════════════════════════════════════════════════════════════════

Method 1: LD_PRELOAD (runtime)
───────────────────────────────────────────────────────────────────
Use jemalloc:
  LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so ./myprogram

Use tcmalloc:
  LD_PRELOAD=/usr/lib/libtcmalloc.so ./myprogram

Method 2: Link-time (build)
───────────────────────────────────────────────────────────────────
GCC with tcmalloc:
  gcc myprogram.c -ltcmalloc -o myprogram

Method 3: System-wide (optional)
───────────────────────────────────────────────────────────────────
Configure alternatives (Debian/Ubuntu):
  update-alternatives --config libjemalloc.so

Method 4: Package manager
───────────────────────────────────────────────────────────────────
Ubuntu/Debian:
  apt-get install libjemalloc-dev
  apt-get install libgoogle-perftools-dev

CentOS/RHEL:
  yum install jemalloc-devel
  yum install gperftools-devel
```

## Debugging and Profiling

### Allocation Tracing

```
TRACING MALLOC CALLS
═══════════════════════════════════════════════════════════════════

Using mtrace (glibc):
───────────────────────────────────────────────────────────────────
#include <mcheck.h>
mtrace();  // Start tracing
// ... allocations ...
muntrace();

// View results:
mtrace ./program mtrace.log

Using valgrind (all allocators):
───────────────────────────────────────────────────────────────────
valgrind --tool=massif ./program
# Generates massif.out.<pid>
ms_print massif.out.<pid> > profile.txt

Using heaptrack (modern):
───────────────────────────────────────────────────────────────────
heaptrack ./program
heaptrack_gui heaptrack.program.XXXX.gz

Using perf (Linux):
───────────────────────────────────────────────────────────────────
perf record -e memory:*/record -ag ./program
perf report
```

### Memory Profiling Comparison

```
PROFILING TOOLS BY ALLOCATOR
═══════════════════════════════════════════════════════════════════

Allocator    Tool                    What it shows
───────────────────────────────────────────────────────────────────
glibc        mtrace                  Call stack, size
glibc        malloc_hook             Custom tracing
jemalloc     jeprof                  Heap profile (pprof format)
jemalloc     stats_print()           Detailed arena stats
tcmalloc     pprof                   Heap profile
tcmalloc     MallocExtension::GetHeapSample()
All          valgrind                Full memory debug
All          heaptrack               Time-based allocation
All          address-sanitizer       Bugs detection
```

## Interview Questions & Answers

### Q1: Why does glibc malloc slow down with many threads?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Contention on arena locks:

Problem:
• Each arena has its own lock
• With more threads than arenas, sharing occurs
• Lock contention increases

Example (4 cores, 16 threads):
- Default arenas = 4 × cores = 16 arenas
- 16 threads, 16 arenas → no sharing → good
- But arena mapping has overhead

Real slowdown factors:
1. Lock acquisition/release overhead
2. Cache line bouncing (false sharing)
3. Arena stealing (threads checking multiple arenas)

Solutions:
- Increase arena count: MALLOC_ARENA_MAX=32
- Switch to jemalloc/tcmalloc (better scaling)
- Use thread-local caching (tcmalloc's approach)
```

### Q2: How does tcmalloc achieve better multithreaded performance?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Three key optimizations:

1. Per-thread caches (tcache)
   - Most allocations: pop from tcache (no lock!)
   - Avoids arena entirely for common case

2. Central free list with per-size-class locks  
   - Different size classes have independent locks
   - Reduces contention vs single arena lock

3. Span-based allocation
   - Large allocations: page-aligned spans
   - Coalescing at page level
   - Fewer lock acquisitions

Performance impact:
- 4 threads: 2-3x faster than glibc
- 8 threads: 5-10x faster than glibc
- Near-linear scaling up to CPU count
```

### Q3: When should you choose jemalloc over tcmalloc?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Choose jemalloc when:

1. Long-running server (weeks/months)
   - Better fragmentation resistance
   - Background thread for purging
   - Running stats to monitor

2. Variable allocation sizes
   - More size classes (up to 4MB)
   - Better fit for odd sizes

3. Memory overcommit concerns  
   - More conservative with memory
   - Better at returning to OS

Choose tcmalloc when:

1. CPU-bound workload
   - Slightly faster per operation
   - Lower latency

2. Predictable allocation patterns
   - Small fixed-size objects
   - Web browser work pattern

3. Already using Google libraries
   - Part of gperftools
   - Integrated profiling
```

### Q4: How does jemalloc reduce fragmentation?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Multiple techniques:

1. Size class optimization
   - Carefully chosen classes minimize waste
   - Example: 48-byte class (14% waste) vs 64 (0% waste)

2. Run-length encoding
   - Small objects grouped in "runs" (pages)
   - Metadata per-run, not per-object

3. Extent coalescing
   - Adjacent free extents merged
   - Background thread consolidates

4. Best-fit with red-black trees
   - O(log n) search for best size
   - Prevents small hole accumulation

5. Dirty page purging
   - Deferred coalescing (dirty_decay_ms)
   - Returns unused pages to OS

Result: 8-12% total fragmentation vs 20%+ for glibc
```

### Q5: Can you use multiple allocators in one program?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Yes, but careful:

Methods:

1. Interpose (LD_PRELOAD)
   - Overrides all malloc/free calls
   - Single allocator for everything

2. Explicit API for custom allocator
   - Libraries: jemalloc has je_malloc() API
   - tcmalloc has tc_malloc() API
   - Mix with system malloc cautiously

3. Per-library loading (not recommended)
   - Different libraries can link different allocators
   - Risk: mixing frees from different allocators!

4. Nested allocators (advanced)
   - Use jemalloc for small objects
   - Use system for huge allocations
   - Requires careful separation

DANGER: Never free pointer from different allocator!
Result: Heap corruption, crashes, silent data corruption
```

### Q6: How to debug memory corruption with different allocators?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

General approach by allocator:

glibc malloc:
- MALLOC_CHECK_=2 (abort on corruption)
- MALLOC_PERTURB_=0xFF (poison freed memory)
- mtrace() (trace allocations)

jemalloc:
- --enable-debug (build option)
- MALLOC_CONF="abort:true"
- stats_print() for corruption detection

tcmalloc:
- HEAPCHECK=normal (detect leaks)
- HEAP_CHECK_ABORT=true
- TCMALLOC_RELEASE_RATE (tune)

Universal tools:
- Valgrind (slower but catches everything)
- Address Sanitizer (fast, good errors)
- Heaptrack (timeline of allocations)

Best practice: Use Address Sanitizer during development:
  gcc -fsanitize=address -g myprogram.c
  Detects use-after-free, buffer overflow, leaks
```

This comprehensive guide covers all major Linux allocators with detailed architecture, comparison, and practical usage information perfect for systems programming interviews!
