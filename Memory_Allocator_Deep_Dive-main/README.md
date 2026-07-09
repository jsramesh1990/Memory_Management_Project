# Memory Allocator Deep Dive 

> A comprehensive guide to **memory allocator internals** across multiple domains - from embedded systems to kernel memory management. Master how `malloc`, `free`, and custom allocators work under the hood.

![Language](https://img.shields.io/badge/language-C-blue.svg)
![C++](https://img.shields.io/badge/C++-11%20%7C%2014%20%7C%2017-orange)
![Linux](https://img.shields.io/badge/Linux-Kernel%20Memory-yellow)
![Embedded](https://img.shields.io/badge/embedded-ARM%20%7C%20AVR-red)
![Android](https://img.shields.io/badge/Android-Memory%20Management-green)
![License](https://img.shields.io/badge/license-MIT-brightgreen)
![Stars](https://img.shields.io/badge/stars-_1-blue)
![Forks](https://img.shields.io/badge/forks-0-gray)
![Contributions](https://img.shields.io/badge/contributions-welcome-orange)

---

##  Overview

This repository provides an **in-depth exploration** of memory allocator implementations across different domains. From understanding how `malloc` works internally to implementing custom allocators for embedded systems, this guide covers theory, practice, and real-world applications.

###  What You'll Learn

- ✅ **Internal workings** of malloc/free
- ✅ **Custom allocator implementation** (pool, stack, buddy, slab)
- ✅ **Memory fragmentation** causes and prevention
- ✅ **Linux kernel memory management**
- ✅ **C++ smart pointers** and RAII
- ✅ **Embedded memory constraints**
- ✅ **Android memory management**

---

##  Conceptual Architecture

### Memory Allocator Ecosystem

```mermaid
graph TB
    subgraph "Application Layer"
        APP[User Application]
        CPP[C++ new/delete]
        C[C malloc/free]
    end
    
    subgraph "Allocator Layer"
        PT[Pool Allocator]
        ST[Stack Allocator]
        BD[Buddy System]
        SL[Slab Allocator]
    end
    
    subgraph "System Layer"
        GLIBC[glibc malloc]
        KERNEL[Kernel Allocator]
        MMAP[mmap / sbrk]
    end
    
    subgraph "Hardware Layer"
        RAM[Physical RAM]
        CACHE[CPU Cache]
        MMU[MMU/Paging]
    end
    
    APP --> CPP
    APP --> C
    CPP --> PT
    CPP --> ST
    C --> GLIBC
    GLIBC --> KERNEL
    PT --> MMAP
    ST --> MMAP
    BD --> KERNEL
    SL --> KERNEL
    KERNEL --> MMU
    MMU --> RAM
    RAM --> CACHE
```

---

##  Memory Allocation Flow

### Complete malloc() Workflow

```text
┌─────────────────────────────────────────────────────────────────┐
│                      USER CALLS malloc(64)                       │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  1. Check thread-local cache (tcache)                           │
│     → If available, return immediately (fast path)              │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼ (cache miss)
┌─────────────────────────────────────────────────────────────────┐
│  2. Check fast bins (16-80 bytes)                               │
│     → LIFO per-thread, very fast                                │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼ (bin empty)
┌─────────────────────────────────────────────────────────────────┐
│  3. Check small bins (80-512 bytes)                             │
│     → Doubly-linked lists, FIFO                                 │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼ (no suitable block)
┌─────────────────────────────────────────────────────────────────┐
│  4. Check unsorted bin                                          │
│     → Recently freed blocks, coalescing opportunity             │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼ (still nothing)
┌─────────────────────────────────────────────────────────────────┐
│  5. Check large bins (512+ bytes)                               │
│     → Best-fit search with splitting                           │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼ (no free block)
┌─────────────────────────────────────────────────────────────────┐
│  6. Request more memory from OS                                 │
│     → sbrk() for small extension                                │
│     → mmap() for large allocations (>128KB)                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  7. New block added to arena, split if needed                   │
│     → Return pointer to user                                     │
└─────────────────────────────────────────────────────────────────┘
```

### free() Workflow

```text
User calls free(ptr)
        │
        ▼
Validate pointer (check if in arena)
        │
        ▼
Mark chunk as free
        │
        ▼
Check adjacent chunks (coalescing)
        │
        ├───► If previous chunk free → merge
        ├───► If next chunk free → merge
        │
        ▼
Place in appropriate bin
        │
        ├───► Small → small bin
        ├───► Large → unsorted bin
        ├───► Very large → return to OS (munmap)
        │
        ▼
Trim arena if possible
```

---

##  Memory Layout Concept

### Process Virtual Memory Map

```text
Higher Address ┌─────────────────────────────┐
               │      Stack (grows down)      │
               │  ┌─────────────────────────┐ │
               │  │ Function locals, frames │ │
               │  └─────────────────────────┘ │
               │             ▼                │
               │                               │
               │         (free space)          │
               │                               │
               │             ▲                │
               │  ┌─────────────────────────┐ │
               │  │         Heap            │ │
               │  │  (malloc arena)         │ │
               │  └─────────────────────────┘ │
               │      .bss (uninitialized)    │
               │      .data (initialized)     │
               │      .rodata (read-only)     │
               │      .text (code)            │
Lower Address  └─────────────────────────────┘
```

### malloc Arena Structure

```c
// Memory chunk header (typical implementation)
struct malloc_chunk {
    size_t mchunk_prev_size;  // Size of previous chunk (if free)
    size_t mchunk_size;       // Size of current chunk (plus flags)
    struct malloc_chunk* fd;  // Forward pointer (free list)
    struct malloc_chunk* bk;  // Backward pointer (free list)
};

// User memory starts right after header
// |←── header ──→|←── user data ──→|
// [prev_size][size][fd][bk][user data...]
```

---

##  Repository Structure

```bash
memory-allocator-deep-dive/
│
├──  Memory_Fundamentals/
│   ├── memory_layout.md
│   ├── virtual_memory.md
│   └── alignment.md
│
├──  How_malloc_Works_Internally/
│   ├── malloc_algorithm.md
│   ├── bins_and_arenas.md
│   └── glibc_implementation.md
│
├──  Fragmentation/
│   ├── internal_fragmentation.md
│   ├── external_fragmentation.md
│   └── prevention_strategies.md
│
├──  Custom_Memory_Allocator/
│   ├── pool_allocator.c
│   ├── stack_allocator.c
│   ├── buddy_allocator.c
│   ├── slab_allocator.c
│   └── README.md
│
├──  Linux_Memory_Management/
│   ├── page_allocation.md
│   ├── kmalloc_vs_vmalloc.md
│   └── memory_mapping.md
│
├──  Linux_Allocators/
│   ├── buddy_system.md
│   ├── slab_allocator.md
│   └── CMA.md
│
├──  Kernel_Memory_Allocation/
│   ├── kernel_apis.md
│   └── DMA_memory.md
│
├──  Embedded_Memory_Management/
│   ├── mcu_memory.md
│   ├── static_allocation.md
│   └── linker_scripts.md
│
├──  Cpp_Memory_Management/
│   ├── new_delete.md
│   ├── placement_new.md
│   └── custom_allocators_cpp.md
│
├──  Smart_Pointers/
│   ├── unique_ptr.md
│   ├── shared_ptr.md
│   ├── weak_ptr.md
│   └── custom_deleters.md
│
├──  Android_Memory_Management/
│   ├── ART_memory.md
│   ├── native_allocators.md
│   └── memory_profiling.md
│
├──  Memory_Alignment/
│   ├── data_alignment.md
│   ├── struct_padding.md
│   └── aligned_alloc.md
│
├──  dynamic_memory/
│   ├── examples/
│   └── exercises/
│
├── Makefile
└── README.md
```

---

##  Custom Allocators Deep Dive

### 1. Pool Allocator

**Use Case**: Fixed-size objects (game entities, network packets)

```c
typedef struct pool_allocator {
    void* memory_block;
    size_t object_size;
    size_t capacity;
    void** free_list;
    size_t free_count;
} pool_allocator_t;

// Initialize pool for 1000 objects of 64 bytes each
pool_allocator_t* pool_create(size_t obj_size, size_t count);

// O(1) allocation
void* pool_alloc(pool_allocator_t* pool);

// O(1) deallocation
void pool_free(pool_allocator_t* pool, void* ptr);
```

**Memory Layout:**
```text
┌─────────────────────────────────────────────────────┐
│     Free List (linked through free slots)          │
│  [slot0]─►[slot1]─►[slot2]─►...─►[slotN]─►NULL     │
└─────────────────────────────────────────────────────┘

Allocation flow:
1. Pop first free slot
2. Return pointer
3. O(1) time, no fragmentation
```

### 2. Stack Allocator (Linear)

**Use Case**: Temporary allocations, frame-based (games)

```c
typedef struct stack_allocator {
    void* start;
    void* current;
    size_t size;
} stack_allocator_t;

// Mark current position
void* stack_mark(stack_allocator_t* allocator);

// Allocate from stack
void* stack_alloc(stack_allocator_t* allocator, size_t bytes);

// Reset to previous mark (LIFO)
void stack_reset(stack_allocator_t* allocator, void* mark);
```

**Visual Flow:**
```text
Initial:  ┌─────────────────────────────┐  current─►
          │                             │
          └─────────────────────────────┘

After allocations:
          ┌─────────────────────────────┐
          │ [obj1][obj2][obj3]          │  current─►
          └─────────────────────────────┘

After reset:
          ┌─────────────────────────────┐
          │                             │  current─►
          └─────────────────────────────┘
```

### 3. Buddy Allocator

**Use Case**: Physical memory management in kernels

```c
// Split memory into power-of-two blocks
// Block sizes: 4KB, 8KB, 16KB, ..., up to total size

void* buddy_alloc(size_t size);  // Round up to next power of two
void buddy_free(void* ptr);

// Coalescing: when both buddies are free, merge
```

**Buddy System Visualization:**
```text
Initial (1MB block):
┌──────────────────────────────────────────────────┐
│                    1 MB                          │
└──────────────────────────────────────────────────┘

After 128KB allocation:
┌──────────────┬───────────────────────────────────┐
│   128KB      │              896KB free           │
│  (alloc)     │                                   │
└──────────────┴───────────────────────────────────┘

After another 128KB:
┌──────────┬──────┬───────────────────────────────┐
│  128KB   │128KB │          768KB free           │
└──────────┴──────┴───────────────────────────────┘

After freeing first 128KB (buddies merge):
┌──────────────────┬───────────────────────────────┐
│      256KB       │          768KB free           │
└──────────────────┴───────────────────────────────┘
```

### 4. Slab Allocator

**Use Case**: Kernel object caches (inodes, task_struct)

```c
struct slab_cache {
    void* memory;           // Contiguous memory
    struct slab* slabs;     // List of slabs
    size_t object_size;     // Size per object
    size_t objects_per_slab;
};

// Create cache for specific object type
struct kmem_cache* kmem_cache_create(const char* name, size_t size);

// Allocate from cache
void* kmem_cache_alloc(struct kmem_cache* cachep);

// Free back to cache
void kmem_cache_free(struct kmem_cache* cachep, void* objp);
```

---

##  Fragmentation Deep Dive

### Internal Fragmentation

```text
Allocated block (malloc(50) returns 64 bytes)
┌─────────────────────────────────────────────┐
│ Header  │  User (50 bytes)  │  Waste (14)  │
│  16B    │                   │              │
└─────────────────────────────────────────────┘
                    ↑
            Internal fragmentation
```

### External Fragmentation

```text
Memory after some allocations and frees:

┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ A   │free │ B   │free │ C   │free │ D   │free │
│100B │200B │100B │50B  │100B │300B │100B │100B │
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
                          ↑
     Cannot allocate 250B even though 650B free
     (fragmented into small chunks)
```

### Prevention Strategies Comparison

| Strategy | Best For | Trade-off |
|----------|----------|-----------|
| **Pool allocator** | Fixed-size objects | Wastes memory if sizes vary |
| **Buddy system** | Power-of-two sizes | Internal fragmentation |
| **Slab allocator** | Kernel objects | Complex implementation |
| **Segregated fit** | General purpose | Multiple free lists |

---

##  Build & Run Examples

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt install gcc make valgrind

# macOS
xcode-select --install
brew install valgrind

# Verify installation
gcc --version
make --version
```

### Build Custom Allocators

```bash
# Clone repository
git clone https://github.com/jsramesh1990/memory-allocator-deep-dive.git
cd memory-allocator-deep-dive

# Build all allocator examples
make

# Run pool allocator demo
make run-pool

# Run stack allocator demo
make run-stack

# Run buddy allocator demo
make run-buddy

# Run all tests
make test

# Memory leak detection
make valgrind-test

# Clean build
make clean
```

### Example: Pool Allocator in Action

```c
#include "custom_memory_allocator/pool_allocator.h"

typedef struct {
    int x, y;
    int health;
} GameObject;

int main() {
    // Create pool for 1000 GameObjects
    pool_allocator_t* pool = pool_create(sizeof(GameObject), 1000);
    
    // Allocate objects
    GameObject* player = pool_alloc(pool);
    GameObject* enemy = pool_alloc(pool);
    GameObject* bullet = pool_alloc(pool);
    
    player->x = 100;
    player->y = 200;
    player->health = 100;
    
    // Free individually
    pool_free(pool, bullet);
    pool_free(pool, enemy);
    pool_free(pool, player);
    
    // Destroy entire pool
    pool_destroy(pool);
    
    return 0;
}
```

**Expected Output:**
```
Pool created: 1000 objects of 12 bytes = 12000 bytes
Allocated: player @ 0x7f8a5c000080
Allocated: enemy @ 0x7f8a5c00008c
Allocated: bullet @ 0x7f8a5c000098
Freed: bullet
Freed: enemy
Freed: player
Pool destroyed, 12000 bytes released
```

---

##  Testing & Validation

### Memory Leak Detection with Valgrind

```bash
$ valgrind --leak-check=full ./pool_demo
==12345== Memcheck, a memory error detector
==12345== Command: ./pool_demo
==12345== 
Pool created: 1000 objects...
All allocations successful!
Pool destroyed.

==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 1 allocs, 1 frees
==12345== 
==12345== All heap blocks were freed -- no leaks are possible
```

---

##  Learning Path

### Beginner → Intermediate → Advanced

```text
Month 1-2: Memory Fundamentals
├── Virtual vs Physical memory
├── Stack vs Heap
├── Memory alignment
└── Understanding pointers

Month 3-4: Standard Allocators
├── How malloc/free work
├── Understanding fragmentation
├── glibc ptmalloc internals
└── jemalloc vs tcmalloc

Month 5-6: Custom Allocators
├── Implementing pool allocator
├── Stack allocator for games
├── Buddy system
└── Slab allocator

Month 7-8: Platform Specific
├── Linux kernel allocators
├── Embedded systems constraints
├── Android ART memory
└── C++ memory model
```

---

##  Real-World Applications

| Domain | Allocation Pattern | Recommended Allocator |
|--------|-------------------|----------------------|
| **Game Engine** | Many temporary objects per frame | Stack + Pool |
| **Network Stack** | Fixed-size packets | Pool allocator |
| **Database** | Variable-size pages | Buddy system |
| **OS Kernel** | Many small objects of same type | Slab allocator |
| **Real-time Audio** | Predictable latency | Block allocator |
| **Embedded Device** | No dynamic allocation | Static pools |

---

##  Performance Benchmarks

| Allocator | Allocation Time | Deallocation Time | Fragmentation | Memory Overhead |
|-----------|----------------|-------------------|---------------|-----------------|
| glibc malloc | ~100ns | ~80ns | Low-Medium | ~16 bytes/block |
| Pool Allocator | ~10ns | ~10ns | None (internal only) | ~0 bytes |
| Stack Allocator | ~5ns | ~5ns (reset) | None (LIFO only) | ~0 bytes |
| Buddy System | ~200ns | ~150ns | Medium | ~50% (worst case) |
| Slab Allocator | ~50ns | ~40ns | Very Low | ~8 bytes/object |

---

##  Contributing

Contributions are welcome! Areas for contribution:

- [ ] Add more allocator implementations (arena, free list)
- [ ] Add benchmark suite
- [ ] Add visualization tools for fragmentation
- [ ] Add more embedded examples
- [ ] Translate to other languages (Rust, Zig allocators)
- [ ] Add interactive web demo

```bash
# Contribution workflow
git checkout -b feature/amazing-allocator
git commit -m "Add amazing allocator implementation"
git push origin feature/amazing-allocator
# Open Pull Request
```

---

##  References & Further Reading

### Books
- **"The Linux Programming Interface"** - Michael Kerrisk
- **"Understanding the Linux Kernel"** - Bovet & Cesati
- **"Modern C++ Design"** - Andrei Alexandrescu (Custom allocators)
- **"Game Programming Patterns"** - Robert Nystrom (Object pools)

### Papers
- *"The Slab Allocator: An Object-Caching Kernel Memory Allocator"* - Bonwick (1994)
- *"A Memory Allocator"* - Doug Lea (1996)
- *"TCMalloc: Thread-Caching Malloc"* - Google (2005)
- *"jemalloc: A Scalable Concurrent Allocator"* - Evans (2006)

### Online Resources
- [glibc malloc internals](https://sourceware.org/glibc/wiki/MallocInternals)
- [Linux kernel memory management](https://www.kernel.org/doc/html/latest/admin-guide/mm/index.html)
- [David Gay's allocator notes](https://github.com/emeryberger/Malloc-Implementations)

---


##  Acknowledgments

- Linux kernel memory management community
- C++ Standards Committee for allocator model

---



