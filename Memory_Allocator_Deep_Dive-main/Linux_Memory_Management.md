# Linux Memory Management

## Overview

Linux memory management is a sophisticated system that provides each process with its own virtual address space, isolates processes from each other, and efficiently utilizes physical RAM. Understanding these concepts is essential for Linux systems programming, debugging, and performance optimization.

## Virtual Memory

### What is Virtual Memory?

```
VIRTUAL MEMORY CONCEPT
═══════════════════════════════════════════════════════════════════

Virtual memory is an abstraction that gives each process its own private address space, independent of physical RAM.

THE ILLUSION:
───────────────────────────────────────────────────────────────────
Each process thinks it has:
┌─────────────────────────────────────────────────────────────────┐
│  Address 0x00000000                                             │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │            Entire 64-bit address space                  │    │
│  │         (16 exabytes on 64-bit systems)                 │    │
│  │                                                          │    │
│  │  • Own code at 0x400000                                 │    │
│  │  • Own heap growing up                                   │    │
│  │  • Own stack growing down                               │    │
│  │  • Continuous, private memory                           │    │
│  └─────────────────────────────────────────────────────────┘    │
│  Address 0xFFFFFFFFFFFFFFFF                                      │
└─────────────────────────────────────────────────────────────────┘

REALITY:
───────────────────────────────────────────────────────────────────
What actually exists:
┌─────────────────────────────────────────────────────────────────┐
│  Physical RAM (limited, shared among processes)                │
│  ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐    │
│  │Page 0│Page 1│Page 2│Page 3│Page 4│Page 5│Page 6│Page 7│    │
│  │ from │ from │ free │ from │ from │ from │ free │ from │    │
│  │Proc A│Proc B│      │Proc A│Proc C│Proc B│      │Proc A│    │
│  └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘    │
│                                                                  │
│  • Physical pages are scattered                                 │
│  • Different processes interleaved                              │
│  • Some pages on disk (swap)                                    │
└─────────────────────────────────────────────────────────────────┘
```

### Virtual Address Space Layout

```
PROCESS VIRTUAL ADDRESS SPACE (64-bit Linux)
═══════════════════════════════════════════════════════════════════

                    HIGH MEMORY (0xFFFFFFFFFFFFFFFF)
                    ┌─────────────────────────────────────────────┐
                    │         Kernel Space (1 byte?)              │
                    │    (actually 128TB on 64-bit Linux)        │
                    │    - Kernel code and data                   │
                    │    - Process-specific kernel structures    │
                    │    - Physical memory mapping               │
                    ├─────────────────────────────────────────────┤
                    │         Stack (8MB typical)                 │
                    │    - Grows downward                         │
                    │    - Function frames, local variables      │
                    │    - Environment variables                  │
                    ├─────────────────────────────────────────────┤
                    │              Memory Mapping Region          │
                    │         (mmap arena - grows both ways)      │
                    │    - Shared libraries                       │
                    │    - mmap() allocations                     │
                    │    - Anonymous mappings                     │
                    │    - File mappings                          │
                    ├─────────────────────────────────────────────┤
                    │                                                 
                    │                 ↓ (gap) ↓                      
                    │                                                 
                    ├─────────────────────────────────────────────┤
                    │                  Heap                        │
                    │    - Grows upward via brk/sbrk              │
                    │    - malloc() allocations                   │
                    ├─────────────────────────────────────────────┤
                    │                  BSS                         │
                    │    - Uninitialized globals/statics          │
                    │    - Zero-initialized at startup            │
                    ├─────────────────────────────────────────────┤
                    │                  Data                        │
                    │    - Initialized globals/statics            │
                    ├─────────────────────────────────────────────┤
                    │                  Text (Code)                 │
                    │    - Executable instructions                │
                    │    - Read-only                              │
                    └─────────────────────────────────────────────┘
                    LOW MEMORY (0x0000000000000000)

TYPICAL 32-BIT LAYOUT (Simplified):
───────────────────────────────────────────────────────────────────
0xFFFFFFFF ┌─────────────────────────────────────────────────────┐
           │           Kernel (1GB)                              │
0xC0000000 ├─────────────────────────────────────────────────────┤
           │                                                     │
           │           User Space (3GB)                          │
           │                                                     │
           │  Stack ↓                                            │
           │                                                     │
           │  mmap ↑                                             │
           │                                                     │
           │  Heap ↑                                             │
           │                                                     │
           │  Data + BSS                                         │
           │                                                     │
           │  Text                                               │
0x00000000 └─────────────────────────────────────────────────────┘
```

## Page Tables and MMU

### Address Translation Flow

```
VIRTUAL TO PHYSICAL ADDRESS TRANSLATION
═══════════════════════════════════════════════════════════════════

HOW A VIRTUAL ADDRESS BECOMES PHYSICAL RAM:

Virtual Address (48 bits on x86_64):
┌─────────────────────────────────────────────────────────────────┐
│  [47]────[39] [38]────[30] [29]────[21] [20]────[12] [11]───[0]│
│      PML4       Page Dir      Page Table     Offset            │
│       9 bits       9 bits        9 bits      12 bits           │
└─────────────────────────────────────────────────────────────────┘

TRANSLATION PROCESS:
═══════════════════════════════════════════════════════════════════

STEP 1: PML4 (Page Map Level 4)
Virtual Address → PML4 entry → Points to Page Directory

STEP 2: Page Directory
PML4 entry → Page Directory → Points to Page Table

STEP 3: Page Table  
Page Directory → Page Table → Points to Physical Page Frame

STEP 4: Offset
Physical Page Frame + Offset = Physical Address

VISUAL REPRESENTATION:
───────────────────────────────────────────────────────────────────

Virtual Address
     │
     ▼
┌─────────────────────────────────────────────────────────────────┐
│                     MMU (Memory Management Unit)                │
│                                                          │      │
│  PML4 ──→ Page Directory ──→ Page Table ──→ Physical Page      │
│    ↓           ↓                ↓                ↓              │
│  Index       Index            Index           Offset            │
│    ↓           ↓                ↓                ↓              │
│  ┌────┐      ┌────┐           ┌────┐           ┌────┐           │
│  │Entry│────→│Entry│──────────→│Entry│─────────→│Frame│          │
│  └────┘      └────┘           └────┘           └────┘           │
│                            Total Physical Address ──────────────┘
└─────────────────────────────────────────────────────────────────┘

PROCESS VIEW                    PHYSICAL VIEW
───────────────────────────────────────────────────────────────────
Process A:                     RAM:
0x0000: [code page]            0x0000: [kernel]
0x1000: [data page]            0x1000: [Process B data]
0x2000: [heap page]            0x2000: [Process C code]
0x3000: [stack page]           0x3000: [Process A code]
                              0x4000: [free]
                              0x5000: [Process A heap]
                              0x6000: [Process B stack]

Each process sees contiguous addresses, but physical pages
are scattered and shared!
```

### Page Table Hierarchy Visualization

```
4-LEVEL PAGE TABLE HIERARCHY (x86_64)
═══════════════════════════════════════════════════════════════════

                    PML4 (Level 4)
                         │
        ┌────────────────┼────────────────┐
        │                │                │
     Entry 0          Entry 1          Entry 511
        │                │                │
        ▼                ▼                ▼
   Page Dir          Page Dir         Page Dir
   (Level 3)         (Level 3)         (Level 3)
        │                │                │
   ┌────┼────┐       ┌────┼────┐          
   │    │    │       │    │    │          
   ▼    ▼    ▼       ▼    ▼    ▼          
  PT    PT   PT      PT   PT   PT         
(L2)  (L2) (L2)    (L2) (L2) (L2)        
   │    │    │       │    │    │          
   ▼    ▼    ▼       ▼    ▼    ▼          
 Page Page Page    Page Page Page         
(L1) (L1) (L1)    (L1) (L1) (L1)         
   │    │    │       │    │    │          
   ▼    ▼    ▼       ▼    ▼    ▼          
 Phys Phys Phys    Phys Phys Phys         
 Frame Frame Frame  Frame Frame Frame      

Each level indexes into tables of 512 entries (9 bits each)
Total pages: 512 × 512 × 512 × 512 = 68 billion pages!
Each page = 4KB → 256TB addressable (x86-64 uses 48 bits)
```

## mmap() System Call

### mmap() Overview

```
mmap() CONCEPT
═══════════════════════════════════════════════════════════════════

mmap() creates a mapping in the virtual address space.

BEFORE mmap():
┌─────────────────────────────────────────────────────────────────┐
│ Process Virtual Address Space                                   │
│ ┌────────────────────────────────────────────────────────────┐  │
│ │ Text │ Data │ Heap │          ??? (unmapped)              │  │
│ └────────────────────────────────────────────────────────────┘  │
│                         ↑                                        │
│                  No mapping here                                 │
└─────────────────────────────────────────────────────────────────┘

AFTER mmap():
┌─────────────────────────────────────────────────────────────────┐
│ Process Virtual Address Space                                   │
│ ┌────────────────────────────────────────────────────────────┐  │
│ │ Text │ Data │ Heap │■■■■■■■■■■■■■■■■■■│     (unmapped)     │  │
│ └────────────────────────────────────────────────────────────┘  │
│                         ↑                                        │
│                   New mapping created                            │
│                   (backed by file or anonymous)                  │
└─────────────────────────────────────────────────────────────────┘
```

### Types of mmap() Mappings

```
MMAP MAPPING TYPES
═══════════════════════════════════════════════════════════════════

1. FILE-BACKED MAPPING (MAP_SHARED)
───────────────────────────────────────────────────────────────────
Process Virtual Address        Physical RAM           Disk File
┌─────────────────┐           ┌─────────┐           ┌─────────┐
│ 0x700000000000  │──────────→│ Page 0  │←──────────│ File    │
│      ↓          │           └─────────┘           │ Block 0 │
│  File Content   │           ┌─────────┐           ├─────────┤
│                 │──────────→│ Page 1  │←──────────│ File    │
│                 │           └─────────┘           │ Block 1 │
└─────────────────┘                                └─────────┘

Changes written back to file (shared between processes)

2. FILE-BACKED MAPPING (MAP_PRIVATE)
───────────────────────────────────────────────────────────────────
Process Virtual Address        Physical RAM           Disk File
┌─────────────────┐           ┌─────────┐           ┌─────────┐
│ 0x700000000000  │──────────→│ Page 0  │(Copy)     │ File    │
│      ↓          │           │(private)│←─┐       │ Block 0 │
│  File Content   │           └─────────┘  │       └─────────┘
│  (copy-on-write)│                        │ (read)
│                 │                        │       ┌─────────┐
│                 │           ┌─────────┐  │       │ File    │
│                 │──────────→│ Page 1  │(Copy)   │ Block 1 │
│                 │           │(private)│←─┘      └─────────┘
└─────────────────┘           └─────────┘           

Changes stay private (COW), not written to file

3. ANONYMOUS MAPPING (MAP_ANONYMOUS)
───────────────────────────────────────────────────────────────────
Process Virtual Address        Physical RAM
┌─────────────────┐           ┌─────────┐
│ 0x700000000000  │──────────→│ Page 0  │ (zero-filled)
│      ↓          │           └─────────┘
│  Anonymous      │           ┌─────────┐
│  Memory         │──────────→│ Page 1  │ (zero-filled)
│                 │           └─────────┘
└─────────────────┘

Not backed by any file - used for large allocations, shared memory

4. SHARED MEMORY (MAP_SHARED | MAP_ANONYMOUS)
───────────────────────────────────────────────────────────────────
Process A                     Physical RAM           Process B
┌─────────────────┐           ┌─────────┐           ┌─────────────────┐
│ 0x700000000000  │──────────→│ Page 0  │←──────────│ 0x700000000000  │
│      ↓          │           └─────────┘           │      ↓          │
│  Shared Data    │           ┌─────────┐           │  Shared Data    │
│                 │──────────→│ Page 1  │←──────────│                 │
└─────────────────┘           └─────────┘           └─────────────────┘

Same physical pages mapped into multiple processes
Changes in one process visible to others
```

### mmap() Regions in Process Space

```
WHERE mmap() FITS IN VIRTUAL ADDRESS SPACE
═══════════════════════════════════════════════════════════════════

                    HIGH ADDRESSES
                    ┌─────────────────────────────────────────────┐
                    │                 Stack                       │
                    │            (grows downward)                 │
                    ├─────────────────────────────────────────────┤
                    │                                             │
                    │         Memory Mapping Region               │
                    │              (mmap arena)                   │
                    │                                             │
                    │  ┌─────────────────────────────────────┐    │
                    │  │  libc.so (shared library)           │    │
                    │  ├─────────────────────────────────────┤    │
                    │  │  libpthread.so                       │    │
                    │  ├─────────────────────────────────────┤    │
                    │  │  Anonymous mapping (malloc large)   │    │
                    │  ├─────────────────────────────────────┤    │
                    │  │  File mapping (shared library data) │    │
                    │  ├─────────────────────────────────────┤    │
                    │  │  Thread stack (pthread_create)      │    │
                    │  └─────────────────────────────────────┘    │
                    │                                             │
                    │                  ↓ gap ↓                    │
                    │                                             │
                    ├─────────────────────────────────────────────┤
                    │                 Heap                        │
                    │            (grows upward)                   │
                    ├─────────────────────────────────────────────┤
                    │                BSS + Data                   │
                    ├─────────────────────────────────────────────┤
                    │                 Text                        │
                    └─────────────────────────────────────────────┘
                    LOW ADDRESSES

Key points:
• Heap (brk) grows upward from low addresses
• mmap region starts above heap, grows both ways
• mmap used for:
  - Shared libraries
  - Large malloc allocations (>128KB)
  - Thread stacks
  - File mappings
  - Shared memory
```

## brk() and sbrk() - Heap Management

### Traditional Heap Expansion

```
brk() / sbrk() HEAP MANAGEMENT
═══════════════════════════════════════════════════════════════════

The "program break" marks the end of the process's data segment.

HEAP STATE TRANSITIONS:
───────────────────────────────────────────────────────────────────

1. INITIAL STATE:
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │                 (unmapped)                │
│                      ↑                                          │
│                   program break                                 │
└─────────────────────────────────────────────────────────────────┘

2. AFTER sbrk(1024) - EXTEND HEAP:
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │█████ new heap █████│     (unmapped)       │
│                      ↑                   ↑                      │
│                  old break          new break                   │
└─────────────────────────────────────────────────────────────────┘

3. AFTER MORE ALLOCATIONS:
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │█████ heap █████████│    (unmapped)        │
│                      ↑                   ↑                      │
│                  used heap           program break              │
└─────────────────────────────────────────────────────────────────┘

4. AFTER free() AT TOP (can shrink):
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │█████ heap ███│         (unmapped)         │
│                      ↑            ↑                             │
│                  used heap    new break                         │
└─────────────────────────────────────────────────────────────────┘

5. AFTER FREE() IN MIDDLE (no shrink - fragmentation):
┌─────────────────────────────────────────────────────────────────┐
│  Text │ Data │ BSS │█████░FREE░█████│         (unmapped)       │
│                      ↑              ↑                           │
│                  used heap      program break                   │
└─────────────────────────────────────────────────────────────────┘
```

### brk vs mmap for Allocations

```
ALLOCATION STRATEGY COMPARISON
═══════════════════════════════════════════════════════════════════

                     malloc(size)
                         │
                         ▼
              ┌─────────────────────┐
              │   size < 128KB?     │
              └─────────────────────┘
                    │           │
                  YES           NO
                    │           │
                    ▼           ▼
            ┌───────────┐  ┌───────────┐
            │   brk()   │  │  mmap()   │
            │  (heap)   │  │ (private) │
            └───────────┘  └───────────┘
                 │               │
                 ▼               ▼
        Small allocations    Large allocations
        (fast, but can't     (can return to OS
        return to OS)        when freed)

CHARACTERISTICS:
───────────────────────────────────────────────────────────────────
Aspect              brk() heap              mmap()
───────────────────────────────────────────────────────────────────
Allocation size     < 128KB (typical)       > 128KB
Free behavior       May not return to OS    Returns to OS immediately
Fragmentation       Can cause holes         No fragmentation
Speed               Faster (no syscall)     Slower (syscall overhead)
Sbrk need           Can't shrink middle     Can unmap any region
Use case            General allocations     Large buffers, shared mem
```

## Demand Paging

### Lazy Loading Concept

```
DEMAND PAGING - PAGES LOADED ON ACCESS
═══════════════════════════════════════════════════════════════════

Pages are NOT loaded into RAM until actually accessed.

PROGRAM STARTUP (No pages loaded):
───────────────────────────────────────────────────────────────────
Virtual Address Space         Physical RAM
┌─────────────────┐           ┌─────────────┐
│ Text (code)     │ (unloaded)│  (empty)    │
│ Data            │           │             │
│ Heap            │           │             │
│ Stack           │           │             │
└─────────────────┘           └─────────────┘
No physical pages allocated yet!

ACCESS FIRST INSTRUCTION (Page fault):
───────────────────────────────────────────────────────────────────
CPU tries to execute at 0x400000
        │
        ▼
┌─────────────────────────────────────────────────────────────────┐
│  MMU: Page not present → Page Fault → Kernel handler           │
│                                                                 │
│  Kernel:                                                       │
│  1. Find page in executable file on disk                       │
│  2. Allocate physical page frame                               │
│  3. Load page content from disk                                │
│  4. Update page table                                          │
│  5. Resume process                                             │
└─────────────────────────────────────────────────────────────────┘
        │
        ▼
Now page is loaded and execution continues.

DEMAND PAGING IN ACTION:
───────────────────────────────────────────────────────────────────
Time    Event                          Physical RAM
───────────────────────────────────────────────────────────────────
t=0     Program starts                 (no pages)
t=1     Access code page 0 → Fault     Load code page 0
t=2     Access data page 0 → Fault     Load data page 0  
t=3     Call function → Fault          Load code page 1
t=4     Access global var → Fault      Load data page 1
t=5     malloc() → Fault               Allocate zero page
...
Result: Only pages actually used are loaded!
```

### Page Fault Handling Flow

```
PAGE FAULT HANDLING SEQUENCE
═══════════════════════════════════════════════════════════════════

                    PROCESS ACCESSES MEMORY
                           │
                           ▼
              ┌────────────────────────┐
              │   MMU looks up page    │
              │   in TLB/Page Tables   │
              └────────────┬───────────┘
                           │
            ┌──────────────┼──────────────┐
            │              │              │
         Present        Not Present    Protection
            │              │            Violation
            ▼              ▼              ▼
        Continue      Page Fault      Segmentation
                       Handler          Fault
                           │
                           ▼
              ┌────────────────────────┐
              │   Determine fault type  │
              └────────────┬───────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
    Valid access       Invalid access    Copy-on-write
        │                  │                  │
        ▼                  ▼                  ▼
    Load page          SIGSEGV           Handle COW
    from disk          (crash)              │
        │                                   ▼
        ▼                              Copy page
    Allocate                           (fork)
    frame                                 │
        │                                   ▼
        ▼                              Update page
    Update                               tables
    page table
        │
        ▼
    Resume process
```

## Copy-on-Write (COW)

### Fork Optimization

```
COPY-ON-WRITE IN fork() SYSTEM CALL
═══════════════════════════════════════════════════════════════════

Traditional fork() would copy all parent memory to child (expensive!)

COW STRATEGY: Share pages until someone writes

BEFORE fork():
───────────────────────────────────────────────────────────────────
Parent Process              Physical RAM
┌─────────────────┐         ┌─────────────┐
│ Page A (data)   │────────→│ Page Frame  │
│ Page B (heap)   │────────→│ Page Frame  │
│ Page C (stack)  │────────→│ Page Frame  │
└─────────────────┘         └─────────────┘

AFTER fork() - COW (no copying yet):
───────────────────────────────────────────────────────────────────
Parent Process              Physical RAM            Child Process
┌─────────────────┐         ┌─────────────┐         ┌─────────────────┐
│ Page A (data)   │────────→│ Page Frame  │←────────│ Page A (data)   │
│ Page B (heap)   │────────→│ Page Frame  │←────────│ Page B (heap)   │
│ Page C (stack)  │────────→│ Page Frame  │←────────│ Page C (stack)  │
└─────────────────┘         └─────────────┘         └─────────────────┘
ALL PAGES SHARED (read-only)! Only page tables duplicated.

PARENT WRITES TO PAGE A (triggers COW):
───────────────────────────────────────────────────────────────────
Parent Process              Physical RAM            Child Process
┌─────────────────┐         ┌─────────────┐         ┌─────────────────┐
│ Page A (data)   │────┐    │ Old Frame   │←────────│ Page A (data)   │
│                 │    │    │ (copy for   │         │ (still shared)  │
│ Page B (heap)   │────┼───→│  parent)    │         │ Page B (heap)   │
│ Page C (stack)  │────┼───→│ New Frame   │         │ Page C (stack)  │
└─────────────────┘    │    │ (parent's   │         └─────────────────┘
                       │    │  private)   │
                       └───→│             │
                            └─────────────┘

1. Page fault on write
2. Kernel copies page (allocate new frame)
3. Parent gets private copy
4. Child continues sharing original
5. Only copied pages that are written!
```

### COW Efficiency

```
COPY-ON-WRITE BENEFITS
═══════════════════════════════════════════════════════════════════

BEFORE COW (Traditional fork):
fork() → Parent (100MB)    → Copy 100MB → Child (100MB)
         Time: 100ms       Memory: 200MB total

WITH COW:
fork() → Parent (100MB)    → Share → Child (0MB physical)
         Time: 1ms         Memory: 100MB total (until writes)

REAL-WORLD EXAMPLE - Shell fork/exec:
───────────────────────────────────────────────────────────────────
1. Shell forks
2. Child calls exec() immediately
3. With COW: Almost no copying (child writes nothing before exec)
4. Traditional fork: Would copy entire shell memory (wasted!)

COW IN ACTION VISUALIZATION:
═══════════════════════════════════════════════════════════════════

Time:              Shared Physical Pages     Parent    Child
───────────────────────────────────────────────────────────────────
After fork:        [████████████████████]    RO        RO
                           (shared)

Child writes 1 page: [████████]              RO        RO
                      [──private──]                     RW
                           (copied)

Parent writes 1 page: [──private──]          RW        RO
                      [████████]                       RO
                    
Result: Only 2 pages copied (8KB) instead of 100MB!
```

## Memory Overcommit

### What is Overcommit?

```
MEMORY OVERCOMMIT EXPLANATION
═══════════════════════════════════════════════════════════════════

Linux allows processes to allocate more virtual memory than physical RAM + swap.

THE PROBLEM WITHOUT OVERCOMMIT:
───────────────────────────────────────────────────────────────────
Application requests 1GB (reserved, not used)
Physical RAM: 512MB
Swap: 256MB
Total: 768MB

Without overcommit: malloc(1GB) → FAILS even though app won't use it all!

WITH OVERCOMMIT:
───────────────────────────────────────────────────────────────────
Application requests 1GB → SUCCEEDS (virtual memory granted)
Physical RAM: 512MB (only 200MB actually used)
Swap: 256MB

Overcommit = Promise more than you have

VISUAL REPRESENTATION:
═══════════════════════════════════════════════════════════════════

Virtual Allocations (what apps see)    Physical Resources (real)
─────────────────────────────────────  ──────────────────────────
Process A: 2GB allocated               Physical RAM: 1GB
Process B: 2GB allocated               Swap: 1GB
Process C: 1GB allocated               Total: 2GB
─────────────────────────────────────
Total: 5GB allocated                   
                                       Only 2GB actual resources!
OVERCOMMIT = 5GB / 2GB = 250%
```

### Overcommit Modes

```
LINUX OVERCOMMIT MODES
═══════════════════════════════════════════════════════════════════

Mode 0: Heuristic overcommit (default)
───────────────────────────────────────────────────────────────────
• Allows overcommit generally
• Rejects obviously impossible allocations
• "Gentle" approach

Example:
  malloc(100GB) on 1GB RAM → Likely rejects
  malloc(2GB) on 1GB RAM → May accept (hope not all used)

Mode 1: Always overcommit
───────────────────────────────────────────────────────────────────
• Never rejects allocations based on memory limits
• Trusts applications won't use all memory

Example:
  malloc(1TB) on 1GB RAM → SUCCESS (virtual allocation)
  Actual usage would cause OOM later

Mode 2: Strict overcommit (no overcommit)
───────────────────────────────────────────────────────────────────
• Never allocate more than total RAM + swap
• Safe but may reject reasonable allocations

Example:
  malloc(2GB) on 1GB RAM + 1GB swap → Accept (exact limit)
  malloc(2.1GB) → Reject

VISUAL COMPARISON:
═══════════════════════════════════════════════════════════════════

Mode 0 (Heuristic)     Mode 1 (Always)        Mode 2 (Never)
───────────────────────────────────────────────────────────────────
Allocated: 3GB         Allocated: 5GB         Allocated: 2GB
Physical: 2GB          Physical: 2GB          Physical: 2GB
                    ↓                        ↓
                    OOM Killer               Predictable
                    (when used)              (may fail early)
```

### Out-of-Memory (OOM) Killer

```
OOM KILLER MECHANISM
═══════════════════════════════════════════════════════════════════

When overcommit fails (system truly runs out of memory):

TRIGGER CONDITIONS:
───────────────────────────────────────────────────────────────────
• No free pages available
• Cannot reclaim enough pages
• Process tries to allocate but nothing left

OOM KILLER DECISION PROCESS:
───────────────────────────────────────────────────────────────────
          System low on memory
                  │
                  ▼
    ┌─────────────────────────────┐
    │  Calculate badness score    │
    │  for each process           │
    └─────────────┬───────────────┘
                  │
    Factors considered:
    • Memory usage (higher = worse)
    • CPU time (lower = worse)  
    • Nice value (lower priority = worse)
    • Root processes (protected)
    • Time since start (newer = worse)
                  │
                  ▼
    ┌─────────────────────────────┐
    │  Select highest score       │
    │  (worst offender)           │
    └─────────────┬───────────────┘
                  │
                  ▼
    ┌─────────────────────────────┐
    │  Kill the process           │
    │  Send SIGKILL (can't catch) │
    └─────────────────────────────┘

VISUAL OF OOM SITUATION:
═══════════════════════════════════════════════════════════════════

Physical RAM:     [████████████████████] 100% full
Swap:             [████████████████████] 100% full

Processes:
┌─────────────────────────────────────────────────────────────────┐
│ Chrome: 2GB    │  Firefox: 1.5GB  │  Database: 1GB │  Editor: 0.1GB│
└─────────────────────────────────────────────────────────────────┘
                       ↓
            Kernel needs memory for new allocation
                       ↓
              OOM Killer picks Database (highest usage)
                       ↓
              Kills Database → Frees 1GB
                       ↓
              New allocation succeeds
```

## Memory Mapping Visualization Summary

### Complete Virtual to Physical Mapping

```
FULL MEMORY MAPPING EXAMPLE
═══════════════════════════════════════════════════════════════════

VIRTUAL ADDRESS SPACE (Process)     PHYSICAL MEMORY
─────────────────────────────────   ───────────────────────────────

0x00000000 ┌─────────────┐
           │   Text      │──────┐   0x00000000 ┌─────────────┐
0x00400000 ├─────────────┤      │              │   Kernel    │
           │   Data      │──┐   └──→ 0x00A00000 ┌─────────────┐
0x00600000 ├─────────────┤  │                  │ Code (P1)   │
           │   BSS       │  │          0x00B00000 ├─────────────┤
0x00800000 ├─────────────┤  │                  │ Data (P1)   │
           │             │  │          0x00C00000 ├─────────────┤
           │    Heap     │──┼──────→ 0x00D00000 │ (free)      │
0x02000000 ├─────────────┤  │                  ├─────────────┤
           │             │  └──────→ 0x00E00000 │ Heap (P1)   │
           │  mmap area  │──┐       ┌───────────┘             │
0x7F000000 ├─────────────┤  │       │          0x00F00000 ┌─────────────┐
           │             │  └──────→│                     │ Code (P2)   │
           │    Stack    │──────────┼──────→ 0x01000000 ┌─────────────┐
0x7FFFFFFF └─────────────┘          │                    │ Shared Lib  │
                                   └──────────────→ 0x01100000 ┌─────────────┐
                                                              │ (free)      │
                                                              └─────────────┘

Each arrow represents a page table mapping
Multiple virtual pages can map to same physical frame (shared memory, COW)
Physical pages can be in any order (scattered)
```

## Key Linux Memory Commands

```
USEFUL LINUX MEMORY COMMANDS
═══════════════════════════════════════════════════════════════════

View process memory map:
───────────────────────────────────────────────────────────────────
cat /proc/<pid>/maps
# Shows all mappings (text, data, heap, stack, mmap)

Example output:
00400000-00401000 r-xp 00000000 08:01 12345    /bin/myprogram
00600000-00601000 rw-p 00000000 08:01 12345    /bin/myprogram
00700000-00702000 rw-p 00000000 00:00 0        [heap]
7f1234000000-7f1234001000 rw-p 00000000 00:00 0
7fff12340000-7fff12361000 rw-p 00000000 00:00 0 [stack]

View memory statistics:
───────────────────────────────────────────────────────────────────
cat /proc/meminfo
# Shows physical memory, swap, available memory

View process memory usage:
───────────────────────────────────────────────────────────────────
ps aux --sort=-%mem
top -o %MEM
htop

Check overcommit settings:
───────────────────────────────────────────────────────────────────
cat /proc/sys/vm/overcommit_memory
cat /proc/sys/vm/overcommit_ratio

Check page fault statistics:
───────────────────────────────────────────────────────────────────
perf stat -e page-faults ./program
```

## Interview Questions & Answers

### Q1: What happens when malloc() returns memory?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

malloc() returns virtual memory addresses, not physical RAM.
Physical pages are NOT allocated immediately.

Timeline:
1. malloc(1GB) → Returns virtual address (fast, <1μs)
2. No physical pages allocated yet
3. Process writes to address → Page fault
4. Kernel allocates physical page (4KB)
5. Process continues
6. Repeat for each 4KB page touched

Result: Physical memory allocated lazily as needed.
Only pages actually used consume physical RAM.
```

### Q2: Why does fork() return quickly even with large memory?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Copy-on-Write (COW) optimization:

Traditional fork would copy all memory (slow)
COW fork:
1. Duplicate page tables (quick, 4KB per process)
2. Mark all pages read-only
3. Share physical pages
4. Only copy pages that get written

Example: 1GB process
Traditional fork: Copy 1GB (100ms+)
COW fork: Copy 4KB page tables (1ms)

Benefit: Most fork() followed by exec() (child overwrites memory anyway)
No copying needed at all in that case!
```

### Q3: Can a process allocate more memory than physical RAM?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Yes, due to:

1. Virtual Memory
   - Each process has own virtual address space
   - Virtual doesn't need physical backing

2. Demand Paging  
   - Physical pages only allocated when accessed
   - Process can allocate 10GB but use 1GB

3. Overcommit (Linux default)
   - Kernel allows overcommit
   - 10 processes each allocate 1GB on 2GB RAM
   - Works as long as total usage < 2GB

4. Swap Space
   - Inactive pages moved to disk
   - Allows physical RAM overcommit

Risk: If all processes actually use their memory → OOM Killer
```

### Q4: What's the difference between mmap() and malloc()?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

mmap()                          malloc()
───────────────────────────────────────────────────────────────────
System call                     Library function
Maps file or anonymous          Returns from heap or mmap
Always page-aligned             May not be page-aligned
Can share between processes     Process-private
Can map files directly          Copies from files
Individual unmapping possible   Free may not return to OS
Used for large allocations      Used for small allocations
Returns memory immediately      May need sbrk/mmap

When malloc uses mmap:
- Large allocations (>128KB default)
- Can be tuned with mallopt()

Best practice:
- Use mmap for: Shared memory, file I/O, large buffers
- Use malloc for: General allocations <1MB
```

### Q5: How do you check actual memory usage of a process?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Multiple metrics (not just "memory used"):

RSS (Resident Set Size)
- Physical RAM currently used
- Includes shared libraries (counted for each process)
- `ps -o rss` or `/proc/<pid>/statm`

PSS (Proportional Set Size)
- RSS divided by number of sharing processes
- More accurate for shared memory
- `cat /proc/<pid>/smaps | grep PSS`

USS (Unique Set Size)  
- Private, non-shared memory
- Memory that would be freed if process dies

VSZ (Virtual Size)
- Total virtual memory allocated
- May be much larger than physical usage

Example:
Process with 100MB RSS (50MB shared library + 50MB private)
- RSS = 100MB (counts shared library fully)
- USS = 50MB (only private)
- PSS = 50MB + (50MB/2 processes sharing) = 75MB

Best practice: Use PSS for accurate memory accounting
```

This comprehensive guide covers all critical Linux memory management concepts with detailed diagrams perfect for system programming interviews!
