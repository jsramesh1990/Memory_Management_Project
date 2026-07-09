# Kernel Memory Allocation

## Overview

Linux kernel memory allocation is fundamentally different from user-space. The kernel must manage memory with strict constraints: no swapping, no lazy allocation, must handle atomic contexts, and must be highly efficient for frequently-used structures.

## User-Space vs Kernel-Space Memory

```
USER-SPACE VS KERNEL-SPACE ALLOCATION
═══════════════════════════════════════════════════════════════════

USER-SPACE (Process)              KERNEL-SPACE (Single image)
───────────────────────────────────────────────────────────────────

┌─────────────────────────┐       ┌─────────────────────────┐
│ Virtual Address Space   │       │ Kernel Virtual Space    │
│                         │       │                         │
│  Process 1 (3GB)        │       │  Kernel (1GB on 32-bit) │
│  • Own page tables      │       │  • Shared by all CPUs   │
│  • Can swap             │       │  • No swapping          │
│  • Lazy allocation      │       │  • Direct mapping       │
│  • Overcommit allowed   │       │  • No overcommit        │
└─────────────────────────┘       └─────────────────────────┘

Allocation Behavior:
───────────────────────────────────────────────────────────────────
User-space malloc():        Kernel-space kmalloc():
• Returns virtual address    • Returns physical address (direct mapped)
• May overcommit            • Always backs with physical pages
• Can sleep (paging)        • May sleep or atomic (GFP flags)
• Uses libc allocator       • Uses SLAB/SLUB/SLOB
• Per-process heap          • Global kernel heap
```

## Kernel Memory Map

```
LINUX KERNEL MEMORY LAYOUT (x86_64)
═══════════════════════════════════════════════════════════════════

                     VIRTUAL ADDRESS SPACE
                     ─────────────────────
0xFFFFFFFFFFFFFFFF  ┌─────────────────────────────────────────────┐
                     │         Modules (dynamic)                  │
0xFFFFFFFFA0000000  ├─────────────────────────────────────────────┤
                     │         vmalloc region                     │
                     │    (virtually contiguous, may be           │
                     │     physically scattered)                  │
0xFFFFFFFFC0000000  ├─────────────────────────────────────────────┤
                     │         Fixmap (fixed mappings)            │
0xFFFFFFFFC0000000  ├─────────────────────────────────────────────┤
                     │         Direct Map (linear mapping)        │
                     │     0xFFFF880000000000 - 64TB              │
                     │     Direct mapping of all physical RAM     │
                     │     Physical address X → Virtual X + offset│
0xFFFF800000000000  ├─────────────────────────────────────────────┤
                     │                                             │
                     │         (unused space)                     │
                     │                                             │
0xFFFF000000000000  ├─────────────────────────────────────────────┤
                     │         Kernel text, data, BSS             │
                     │         (mapped at compile-time)           │
0xFFFFFFFF80000000  └─────────────────────────────────────────────┘

Direct Map (kmalloc region):
═══════════════════════════════════════════════════════════════════
• Virtual address = Physical address + PAGE_OFFSET (0xFFFF880000000000)
• Simple translation: virt_to_phys() / phys_to_virt()
• Used for kmalloc() and slab allocations
• Physically contiguous (important for DMA!)
• Size limited by physical RAM

vmalloc Region:
═══════════════════════════════════════════════════════════════════
• Virtually contiguous, physically scattered
• Used for large, non-DMA buffers
• Slower than kmalloc (needs page table updates)
• Can allocate larger regions
```

## kmalloc() - Kernel Heap Allocation

### GFP Flags Classification

```
GFP (Get Free Pages) FLAGS
═══════════════════════════════════════════════════════════════════

GFP_KERNEL (Normal allocation)
───────────────────────────────────────────────────────────────────
• Default flag for process context
• Can sleep (wait for memory)
• May initiate reclaim (swap, page cache)
• Example: Driver initialization, file operations

Usage: ptr = kmalloc(size, GFP_KERNEL);

GFP_ATOMIC (Atomic context)
───────────────────────────────────────────────────────────────────
• Cannot sleep (interrupts, spinlocks)
• Limited memory reserve
• Higher failure rate
• Uses emergency pools

Usage: ptr = kmalloc(size, GFP_ATOMIC);
When: Interrupt handlers, softirqs, spinlock held

GFP_DMA / GFP_DMA32
───────────────────────────────────────────────────────────────────
• Memory usable for DMA (certain address range)
• Legacy ISA devices (4GB for GFP_DMA32)
• Physically contiguous requirements

Usage: ptr = kmalloc(size, GFP_DMA | GFP_KERNEL);

GFP_NOWAIT
───────────────────────────────────────────────────────────────────
• Like GFP_ATOMIC but weaker guarantees
• No IO or fs operations
• May still sleep in rare cases

GFP_NOIO / GFP_NOFS
───────────────────────────────────────────────────────────────────
• Prevent recursive reclaim
• Used in block/FS layer
• GFP_NOIO = no disk I/O during allocation

GFP_USER
───────────────────────────────────────────────────────────────────
• For userspace memory allocations
• Subject to user limits

GFP_FLAGS DECISION TREE:
═══════════════════════════════════════════════════════════════════

                    Can sleep?
                         │
              ┌──────────┴──────────┐
             YES                    NO
              │                      │
              ▼                      ▼
        Process context?         GFP_ATOMIC
              │                   GFP_NOWAIT
         ┌────┴────┐
        YES       NO
         │         │
         ▼         ▼
    GFP_KERNEL   (can't
                 happen)

              Special Requirements?
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
    DMA needed?      FS layer?       User memory?
         │               │               │
         ▼               ▼               ▼
    GFP_DMA          GFP_NOFS         GFP_USER
    GFP_DMA32
```

### kmalloc Implementation

```
kmalloc INTERNALS
═══════════════════════════════════════════════════════════════════

SLAB API LAYER:
───────────────────────────────────────────────────────────────────
kmalloc() → slab_alloc() → kmem_cache_alloc() → page allocator

Size buckets (kmalloc_caches):
┌─────────────────────────────────────────────────────────────────┐
│  Index  Size   Cache name                                      │
│  ─────────────────────────────────────────────────────────────  │
│  0      8      kmalloc-8                                       │
│  1      16     kmalloc-16                                      │
│  2      32     kmalloc-32                                      │
│  3      64     kmalloc-64                                      │
│  4      128    kmalloc-128                                     │
│  5      256    kmalloc-256                                     │
│  6      512    kmalloc-512                                     │
│  7      1024   kmalloc-1024                                    │
│  8      2048   kmalloc-2048                                    │
│  9      4096   kmalloc-4096 (page size)                        │
│  ...    ...                                                    │
│  N      8MB    kmalloc-8M (largest)                            │
└─────────────────────────────────────────────────────────────────┘

ALLOCATION FLOW:
───────────────────────────────────────────────────────────────────
kmalloc(100, GFP_KERNEL)
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│ 1. Map size to cache (128-byte kmalloc-128)                   │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. kmem_cache_alloc(kmalloc-128)                              │
│    • Check per-CPU array cache                                │
│    • Check slab partial list if cache empty                   │
│    • Allocate new slab if needed                              │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. Return pointer (already zeroed if __GFP_ZERO)              │
└─────────────────────────────────────────────────────────────────┘

DIRECT MAPPING ADVANTAGE:
───────────────────────────────────────────────────────────────────
• kmalloc returns directly mapped address
• Translation: phys_to_virt(phys_address)
• No TLB misses for kernel address
• Very fast (~50-100ns typical)
```

## vmalloc() - Virtual Contiguous Allocation

### vmalloc Concept

```
vmalloc ARCHITECTURE
═══════════════════════════════════════════════════════════════════

PHYSICAL MEMORY (fragmented):
───────────────────────────────────────────────────────────────────
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│Page0 │Page1 │Page2 │Page3 │Page4 │Page5 │Page6 │Page7 │
│(free)│(used)│(free)│(used)│(free)│(used)│(free)│(free)│
└──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘

vmalloc(3 pages) → Virtual view:
───────────────────────────────────────────────────────────────────
VIRTUAL ADDRESS SPACE:
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│ V0   │ V1   │ V2   │ V3   │ V4   │ V5   │ V6   │ V7   │
│(VMAP)│(VMAP)│(VMAP)│(free)│(free)│(free)│(free)│(free)│
└──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
   │      │      │
   │      │      └──────┐
   │      └─────────┐   │
   │                │   │
   ▼                ▼   ▼
PHYSICAL: Page2    Page0 Page6
(physically scattered but virtually contiguous!)

vmalloc vs kmalloc COMPARISON:
───────────────────────────────────────────────────────────────────
Aspect              kmalloc                 vmalloc
───────────────────────────────────────────────────────────────────
Physical memory     Contiguous              Scattered
Virtual memory      Direct Mapped           Custom mapped
Max size            Limited (phys RAM)      Large (virtual space)
Performance         Very fast               Slower (TLB fills)
DMA support         Yes (needs physical)    No
Overhead            Low (metadata only)     High (page tables)
Use case            Small, frequent,DMA     Large, rare buffers
```

### vmalloc Usage Example

```
vmalloc USE CASES
═══════════════════════════════════════════════════════════════════

When to use vmalloc:
───────────────────────────────────────────────────────────────────
1. Very large buffers (> 128KB)
2. Memory where physical contiguity not needed
3. Allocations that may fail as kmalloc
4. Modules loading (needs virtual space)

Example: Framebuffer driver
───────────────────────────────────────────────────────────────────
struct fb_info {
    void *screen_buffer;  // 1920x1080x4 = 8MB
};
// kmalloc would fail (too large, needs contiguity)
// vmalloc succeeds (scattered physical pages)

Example: Network packet capture
───────────────────────────────────────────────────────────────────
struct capture_buffer {
    void *buffer;  // 1GB ring buffer
};
// vmalloc can allocate 1GB virtually contiguous
// kmalloc would likely fail

Performance impact:
───────────────────────────────────────────────────────────────────
• Each vmalloc access may miss in TLB
• More page table walks
• 5-10% slower than kmalloc
• Use kmalloc when possible
```

## SLAB Allocator Family

### Evolution Overview

```
SLAB ALLOCATOR FAMILY TREE
═══════════════════════════════════════════════════════════════════

                    Linux 2.2+
                          │
                    SLAB (original)
                    (complex, high metadata)
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
        ▼                 ▼                 ▼
    SLUB (Linux 2.6.22) SLOB (embedded)  SLAB (legacy)
    (simpler, less      (tiny, simple)    (still used)
     metadata)
        │
        ▼
    Default since v2.6.22
    (most common today)

COMPARISON TABLE:
═══════════════════════════════════════════════════════════════════
Feature              SLAB            SLUB            SLOB
───────────────────────────────────────────────────────────────────
Metadata complexity  High            Low             Very low
Memory overhead      High (~8%)      Low (~2-3%)     Minimal
Performance (large)  Fast            Very fast       Slow
Performance (small)  Good            Good            Poor
SMP scalability      Good            Excellent       Poor
Debug features       Good            Very good       None
Code size            Large           Medium          Tiny (~2KB)
Use case             General         General (def)   Small embedded
NUMA awareness       Yes             Yes             No
Page allocator usage Conservative    Efficient       Minimal
```

### SLUB Allocator (Default since 2.6.22)

```
SLUB ARCHITECTURE
═══════════════════════════════════════════════════════════════════

SIMPLIFIED DATA STRUCTURES:
───────────────────────────────────────────────────────────────────
Instead of complex SLAB metadata, SLUB uses simple per-page struct:

struct page {
    struct kmem_cache *slab_cache;  // Which cache owns it
    void *freelist;                 // First free object
    int inuse;                      // Objects in use
    int objects;                    // Total objects in page
    atomic_t frozen;                // CPU has this page
    // ... (much simpler than SLAB)
};

PER-CPU CACHES (fast path):
───────────────────────────────────────────────────────────────────
CPU 0:                          CPU 1:
┌─────────────────────┐         ┌─────────────────────┐
│ kmem_cache_cpu      │         │ kmem_cache_cpu      │
│ ┌─────────────────┐ │         │ ┌─────────────────┐ │
│ │ page (current)  │ │         │ │ page (current)  │ │
│ │ freelist ───────┼─┼─┐       │ │ freelist ───────┼─┼─┐
│ │ tid (generation)│ │ │       │ │ tid (generation)│ │ │
│ └─────────────────┘ │ │       │ └─────────────────┘ │ │
└─────────────────────┘ │       └─────────────────────┘ │
                        │                               │
                        ▼                               ▼
                   [obj][obj]                        [obj][obj]
                    [free]                            [free]

SIMPLE ALLOCATION FLOW:
───────────────────────────────────────────────────────────────────
1. Check per-CPU cache (fast path, no lock)
2. If empty, get "partial" slab from node list
3. Refill per-CPU cache (batch allocate)
4. Allocate object

FREE FLOW EQUALLY SIMPLE:
───────────────────────────────────────────────────────────────────
1. Add object to per-CPU freelist
2. If slab becomes completely free → return to page allocator
3. Simplified logic = faster!

ADVANTAGES OVER SLAB:
═══════════════════════════════════════════════════════════════════
• Less metadata (each page independent)
• No internal fragmentation tracking
• Better NUMA support
• Simpler debugging
• Faster allocation/free (no complex queues)
• Less lock contention
```

### SLOB Allocator (For Tiny Embedded)

```
SLOB DESIGN PHILOSOPHY
═══════════════════════════════════════════════════════════════════

Goal: Minimal memory overhead for VERY small systems (4MB RAM)

ARCHITECTURE:
───────────────────────────────────────────────────────────────────
Simple linked-list of free blocks (like user-space malloc):

SLOB AREA (single page list):
┌─────────────────────────────────────────────────────────────────┐
│ [size][next][size][next][size][next][size][next]               │
│  free    free    used    free                                   │
└─────────────────────────────────────────────────────────────────┘

Block structure (minimal):
┌─────────────────────────────────────────────────────────────────┐
│  size (2 bytes)  │  next (2 bytes) │  user data                │
└─────────────────────────────────────────────────────────────────┘

FEATURES:
═══════════════════════════════════════════════════════════════════
✓ Extremely small (~2KB code)
✓ Simple first-fit algorithm
✓ No separate object caches
✓ No per-CPU caches
✓ Minimal metadata (4 bytes overhead)

DRAWBACKS:
═══════════════════════════════════════════════════════════════════
✗ Very slow (O(n) allocations)
✗ No SMP support
✗ No NUMA awareness
✗ Fragmentation issues

WHEN TO USE SLOB:
───────────────────────────────────────────────────────────────────
• Embedded systems with < 4MB RAM
• Single-core systems
• Low-performance requirements
• CONFIG_SLOB=y in kernel config
```

## Kernel Object Caching

### Creating Custom Slab Caches

```
CREATING DEDICATED OBJECT CACHES
═══════════════════════════════════════════════════════════════════

WHY CUSTOM CACHES?
───────────────────────────────────────────────────────────────────
• Frequently allocated structures (task_struct, inode, dentry)
• Better cache locality
• Reduced fragmentation
• Faster allocation (O(1) with per-CPU caches)

EXAMPLE: Custom Device Cache
───────────────────────────────────────────────────────────────────
struct my_device {
    int id;
    char name[32];
    struct list_head list;
    void (*callback)(void);
    // ... more fields
};

static struct kmem_cache *my_device_cache;

// Initialization (module init)
static int __init my_init(void) {
    my_device_cache = kmem_cache_create(
        "my_device",                 // Name for /proc/slabinfo
        sizeof(struct my_device),    // Object size
        __alignof__(struct my_device), // Alignment
        0,                           // Flags
        NULL                         // Constructor
    );
    
    if (!my_device_cache)
        return -ENOMEM;
    return 0;
}

// Allocation
struct my_device *dev = kmem_cache_alloc(my_device_cache, GFP_KERNEL);
if (!dev)
    return -ENOMEM;

// Initialize (if no constructor)
dev->id = 42;
INIT_LIST_HEAD(&dev->list);

// Free
kmem_cache_free(my_device_cache, dev);

// Cleanup (module exit)
static void __exit my_exit(void) {
    kmem_cache_destroy(my_device_cache);
}
```

### Slab Flags and Options

```
SLAB CACHE FLAGS
═══════════════════════════════════════════════════════════════════

SLAB_HWCACHE_ALIGN
───────────────────────────────────────────────────────────────────
• Align objects to hardware cache lines
• Prevents false sharing in SMP
• Wastes memory (padding)

Usage: kmem_cache_create(..., SLAB_HWCACHE_ALIGN, ...);

SLAB_PANIC
───────────────────────────────────────────────────────────────────
• Panic if cache creation fails
• For critical system caches

Usage: kmem_cache_create(..., SLAB_PANIC, ...);

SLAB_POISON
───────────────────────────────────────────────────────────────────
• Fill freed memory with poison pattern
• Detect use-after-free
• Debug only (performance impact)

SLAB_RED_ZONE
───────────────────────────────────────────────────────────────────
• Add red zones around objects
• Detect buffer overflows
• Debug only

SLAB_ACCOUNT
───────────────────────────────────────────────────────────────────
• Memory accounted to cgroup
• For container environments

COMBINING FLAGS:
───────────────────────────────────────────────────────────────────
kmem_cache_create(name, size, align,
                  SLAB_HWCACHE_ALIGN | SLAB_PANIC,
                  NULL);
```

## Kernel Memory Statistics

### Inspecting Slab Usage

```
VIEWING SLAB STATISTICS (/proc/slabinfo)
═══════════════════════════════════════════════════════════════════

Example output:
───────────────────────────────────────────────────────────────────
# name            <active_objs> <num_objs> <objsize> <objperslab>
kmalloc-128          5120       5120        128         32
task_struct          256         256       7040          4
inode_cache         1024        1024        608          6
dentry              2048        2048        192         16
vm_area_struct       512         512        216         18

Meaning:
───────────────────────────────────────────────────────────────────
active_objs: Currently allocated objects
num_objs: Total objects in cache (including free)
objsize: Size of each object
objperslab: Objects per slab page

Command to view:
$ cat /proc/slabinfo
$ slabtop (interactive)

PROCESS-SPECIFIC SLAB USAGE:
───────────────────────────────────────────────────────────────────
$ cat /proc/<pid>/smaps | grep -i slab

USEFUL TOOLS:
───────────────────────────────────────────────────────────────────
slabtop -o      # Show slab utilization, sort by cache size
echo 1 > /proc/sys/vm/slab_debug  # Enable slab debug
cat /proc/meminfo | grep Slab    # Total slab memory
```

### Debugging Kernel Memory Issues

```
KERNEL MEMORY DEBUGGING
═══════════════════════════════════════════════════════════════════

CONFIG Options for Debug:
───────────────────────────────────────────────────────────────────
CONFIG_SLUB_DEBUG=y           # Enable SLUB debugging
CONFIG_DEBUG_SLAB=y           # Legacy SLAB debugging
CONFIG_DEBUG_KMEMLEAK=y       # Detect memory leaks
CONFIG_DEBUG_PAGEALLOC=y      # Detect use-after-free
CONFIG_KASAN=y                # Kernel Address Sanitizer

KASAN (Kernel Address Sanitizer):
───────────────────────────────────────────────────────────────────
• Detects use-after-free, buffer overflow
• Runtime overhead (~2x slower)
• Shadow memory technique

Usage:
  CONFIG_KASAN=y
  CONFIG_KASAN_INLINE=y

SLUB Debug Features:
───────────────────────────────────────────────────────────────────
• Red zones around objects
• Poison pattern on free (0x6b)
• Object tracking (stack traces)

Enable at boot:
  slub_debug=FZPU  # Full debugging
  slub_debug=      # Disable

KMEMLEAK (Memory Leak Detection):
───────────────────────────────────────────────────────────────────
echo scan > /sys/kernel/debug/kmemleak
cat /sys/kernel/debug/kmemleak

Reports potential leaks with stack traces
```

## Real-World Examples

### Example 1: Network Packet Allocation

```
NETWORK DRIVER BUFFER MANAGEMENT
═══════════════════════════════════════════════════════════════════

// RX descriptor ring (DMA needs contiguous)
struct rx_desc {
    u32 status;
    u32 length;
    dma_addr_t dma_addr;
};

// Pre-allocate RX buffers
static int init_rx_ring(struct net_device *dev) {
    struct my_private *priv = netdev_priv(dev);
    
    // kmalloc for descriptor rings (DMA needs contiguity)
    priv->rx_ring = kmalloc_array(RX_RING_SIZE, sizeof(*priv->rx_ring),
                                   GFP_KERNEL | GFP_DMA);
    if (!priv->rx_ring)
        return -ENOMEM;
    
    // Allocate packet buffers (can be vmalloc - no DMA)
    for (int i = 0; i < RX_RING_SIZE; i++) {
        priv->rx_buffers[i] = kmalloc(2048, GFP_ATOMIC);  // ISR context!
        if (!priv->rx_buffers[i])
            goto err_free;
    }
    return 0;
}
```

### Example 2: Filesystem Cache

```
FILESYSTEM INODE CACHING
═══════════════════════════════════════════════════════════════════

// Kernel's dedicated inode cache
static struct kmem_cache *inode_cachep;

void __init inode_init(void) {
    // Create cache for struct inode
    inode_cachep = kmem_cache_create("inode_cache",
                                     sizeof(struct inode),
                                     __alignof__(struct inode),
                                     SLAB_HWCACHE_ALIGN | SLAB_PANIC,
                                     NULL);
}

struct inode *alloc_inode(struct super_block *sb) {
    struct inode *inode;
    
    // GFP_NOFS because we're in filesystem path (no recursion!)
    inode = kmem_cache_alloc(inode_cachep, GFP_NOFS);
    if (!inode)
        return NULL;
    
    // Initialize inode
    inode->i_sb = sb;
    atomic_set(&inode->i_count, 1);
    // ...
    
    return inode;
}

void destroy_inode(struct inode *inode) {
    // Cleanup
    // ...
    kmem_cache_free(inode_cachep, inode);
}
```

### Example 3: DMA Buffer Allocation

```
DMA BUFFER MANAGEMENT
═══════════════════════════════════════════════════════════════════

// DMA requires physically contiguous memory
int dma_alloc_example(struct device *dev) {
    dma_addr_t dma_handle;
    void *cpu_addr;
    
    // Option 1: Consistent (coherent) DMA
    cpu_addr = dma_alloc_coherent(dev, BUFFER_SIZE, &dma_handle, GFP_KERNEL);
    if (!cpu_addr)
        return -ENOMEM;
    
    // Use for streaming DMA (cached, need sync)
    cpu_addr = kmalloc(BUFFER_SIZE, GFP_KERNEL | GFP_DMA);
    if (!cpu_addr)
        return -ENOMEM;
    
    dma_handle = virt_to_phys(cpu_addr);  // Direct mapping!
    
    // After DMA completion
    dma_sync_single_for_cpu(dev, dma_handle, BUFFER_SIZE, DMA_FROM_DEVICE);
    
    // Free
    dma_free_coherent(dev, BUFFER_SIZE, cpu_addr, dma_handle);
    kfree(cpu_addr);
    
    return 0;
}
```

## Interview Questions & Answers

### Q1: What's the difference between kmalloc and vmalloc?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

kmalloc                              vmalloc
───────────────────────────────────────────────────────────────────
Physically contiguous                 Physically scattered
Direct mapped (virt→phys simple)      Custom page table mapping
Fast (no TLB misses for kernel)       Slower (page table walks)
Limited size (max physical RAM)       Can be very large
Suitable for DMA                      Not DMA-suitable
Lazy allocation? No                   No (allocates pages)
Returns lowmem addresses              Returns vmalloc region addresses
< 128KB typical                       Any size

Performance ratio:
kmalloc: 50-100ns
vmalloc: 200-300ns

When to use:
- kmalloc: Most kernel allocations, DMA buffers
- vmalloc: Large buffers, modules, rare allocations
```

### Q2: Explain GFP_KERNEL vs GFP_ATOMIC.

```
ANSWER:
═══════════════════════════════════════════════════════════════════

GFP_KERNEL                          GFP_ATOMIC
───────────────────────────────────────────────────────────────────
Can sleep                           Cannot sleep
Process context only                Interrupts, spinlocks, any context
May reclaim memory                  Uses emergency pool
May initiate I/O                    No I/O
Higher success rate                 Lower success rate
Default for normal allocations      Required for atomic context

Usage:
spin_lock(&lock);
ptr = kmalloc(100, GFP_ATOMIC);  // Must be atomic!
spin_unlock(&lock);

In interrupt handler:
irqreturn_t handler(int irq, void *dev) {
    ptr = kmalloc(64, GFP_ATOMIC);  // NO SLEEP!
}

Memory reserves:
GFP_ATOMIC can access emergency pools (MEMCG reserves)
```

### Q3: What problem does SLAB/SLUB solve?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Problem: Frequent allocation/destruction of same structures
- kmalloc has overhead (metadata, alignment)
- Fragmentation from mixed sizes
- Poor cache locality

Solution: Object caching
1. Pre-allocate slabs of pages (typically 4KB)
2. Divide into equal-sized objects
3. Cache objects in per-CPU arrays
4. Fast O(1) allocation/free

Benefits:
✓ 50-80% faster than raw page allocator
✓ Better cache locality (objects together)
✓ No fragmentation per slab
✓ Object reuse (constructor/destructor)

Example: task_struct
- Created for every thread
- Destroyed on thread exit
- Slab cache makes this 100x faster
```

### Q4: Why can't you use vmalloc for DMA?

```
ANSWER:
───────────────────────────────────────────────────────────────────

DMA controllers need physical addresses, not virtual!

Problem with vmalloc:
- vmalloc returns scattered physical pages
- Most DMA controllers need contiguous physical memory
- Some support scatter-gather (SG) DMA, but rare

Virtual address      Physical pages
[Vaddr + 0]      →   Page 0 (phys 0x1000)
[Vaddr + 4096]   →   Page 7 (phys 0x7000)  // Not contiguous!
[Vaddr + 8192]   →   Page 3 (phys 0x3000)
DMA reads 3 pages → would read from scattered addresses!

Solution:
- Use kmalloc (physically contiguous)
- Use dma_alloc_coherent (coherent DMA)
- Use SG DMA (if hardware supports)

Exception: IOMMU can remap scatter pages (VFIO, virtualization)
```

### Q5: How do you debug kernel memory leaks?

```
ANSWER:
───────────────────────────────────────────────────────────────────

1. CONFIG_DEBUG_KMEMLEAK (kernel built-in)
   echo scan > /sys/kernel/debug/kmemleak
   cat /sys/kernel/debug/kmemleak
   # Reports unreferenced allocations

2. SLUB Debugging
   slub_debug=FZPU (boot param)
   # Enables red zones, poisoning, tracking

3. KASAN (Kernel Address Sanitizer)
   CONFIG_KASAN=y
   # Detects use-after-free, out-of-bounds

4. kmemleak annotations
   kmemleak_alloc(ptr, size, 0, GFP_KERNEL);
   kmemleak_free(ptr);
   kmemleak_ignore(ptr);

5. Valgrind for kernel (limited)
   Uses KEDR framework

6. Custom tracking
   Add allocation tracking to your code
   Print stats in cleanup

Typical leak detection flow:
1. Enable CONFIG_DEBUG_KMEMLEAK
2. Run workload
3. Check /sys/kernel/debug/kmemleak
4. Fix leaks (missing kfree/kmem_cache_free)
```

### Q6: What's the maximum size for kmalloc?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Depends on configuration:

32-bit systems:
- KMALLOC_MAX_SIZE = 4MB (typically)
- Limited by virtual space + fragmentation

64-bit systems:
- KMALLOC_MAX_SIZE = 32MB (default)
- Can be increased via CONFIG_KMALLOC_MAX_SIZE

Real limits (fragmentation imposes lower limit):
- < 128KB: Always works (most common)
- 128KB-4MB: Usually works (may fail under pressure)
- > 4MB: Consider vmalloc

Check at runtime:
#include <linux/slab.h>
int max_size = KMALLOC_MAX_SIZE;

When kmalloc fails:
1. Try vmalloc for large buffers
2. Reduce allocation size (use scatter-gather)
3. Pre-allocate at boot (DMA pools)

Example: 1MB allocation
ptr = kmalloc(1024*1024, GFP_KERNEL | __GFP_NORETRY);
if (!ptr) {
    // Fallback to vmalloc
    ptr = vmalloc(1024*1024);
}
```

### Q7: What's the purpose of per-CPU caches in SLUB?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Purpose: Lockless allocation for hot paths

Traditional (without per-CPU):
Thread 1 → Lock → Allocate → Unlock
Thread 2 → Lock → Allocate → Unlock
(Serialized, high contention)

With per-CPU caches:
Thread 1 → CPU0: [obj][obj][obj] (no lock)
Thread 2 → CPU1: [obj][obj][obj] (no lock)
(Parallel, no contention)

Implementation:
struct kmem_cache_cpu {
    void **freelist;    // Pointer to free objects
    struct page *page;  // Current slab page
    tid_t tid;          // Cache generation ID
};

Operation:
1. Load current CPU's cache
2. Pop freelist (NO LOCK NEEDED!)
3. If empty, refill from node cache (with lock)
4. Return object

Benefits:
- 100x faster for frequent allocations
- Scales linearly with CPU count
- Reduces cache line bouncing

Downside: Memory overhead per CPU
```

### Q8: How does SLUB differ from SLAB?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Complexity (SLAB more complex):
- SLAB: 3-level queues (full, partial, empty)
- SLUB: Single per-node partial list

Metadata location:
- SLAB: Separate kmem_cache structure (complex)
- SLUB: Per-page metadata (simple struct page fields)

Locking:
- SLAB: Per-cache lock + per-slab locks
- SLUB: Per-node lock + per-cpu cache (lockless)

Debug features:
- SLAB: Limited
- SLUB: Excellent (red zones, poisoning, tracking)

Code size:
- SLAB: ~5000+ lines
- SLUB: ~3000 lines
- SLOB: ~500 lines

Performance (SLUB wins):
- Faster allocation (~20%)
- Less cache footprint
- Better scalability

Migration path:
- SLAB (original, complex)
- SLUB (simpler, default since 2.6.22)
- SLUB is now maintenance-focused, SLAB is legacy

Current state: Use SLUB unless you have specific needs
```

This comprehensive guide covers all kernel memory allocation topics with detailed architecture, examples, and interview-focused explanations perfect for Linux kernel interviews!
