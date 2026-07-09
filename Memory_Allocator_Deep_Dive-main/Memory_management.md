# Memory Management in Embedded Linux

## Overview

Memory management is one of the most important responsibilities of the Linux kernel.

It manages:
- RAM allocation
- virtual memory
- paging
- memory protection
- caching
- process isolation

Memory management ensures:
- efficient memory usage
- multitasking support
- process security
- system stability

---

# 1. What is Memory Management?

Memory management is:

```text
The process of controlling and organizing system memory
```

Handled by:
```text
Linux Memory Manager (MM)
```

inside the kernel.

---

# 2. Why Memory Management is Needed

Modern systems run:
- multiple processes
- kernel code
- device drivers
- caches

Memory manager ensures:
- no memory conflicts
- efficient RAM usage
- isolation between processes

---

# 3. High-Level Memory Management Flow

```text
Application Requests Memory
           ↓
Kernel Allocates Virtual Memory
           ↓
MMU Maps Virtual → Physical Address
           ↓
CPU Accesses Memory
``` id="flow1"

---

# 4. Linux Memory Architecture

```text
+----------------------------+
| User Space                 |
|----------------------------|
| Application Virtual Memory |
+----------------------------+
| Kernel Space               |
|----------------------------|
| Kernel Memory              |
| Device Drivers             |
| Page Cache                 |
+----------------------------+
| Physical RAM               |
+----------------------------+
``` id="arch1"

---

# 5. Physical Memory

Actual RAM hardware.

Example:

```text
DDR4 / LPDDR / SDRAM
```

CPU accesses:
```text
physical addresses
```

through MMU translation.

---

# 6. Virtual Memory

Each process gets:
```text
independent virtual address space
```

Advantages:
- isolation
- security
- easier programming

---

# 7. Virtual Address Space

```text
+----------------------+
| Stack                |
+----------------------+
| Heap                 |
+----------------------+
| Data Segment         |
+----------------------+
| Text Segment         |
+----------------------+
``` id="virt1"

---

# 8. MMU (Memory Management Unit)

Hardware block performing:

```text
Virtual → Physical address translation
```

---

# 9. Address Translation Flow

```text
CPU Virtual Address
        ↓
MMU
        ↓
Page Tables
        ↓
Physical Address
``` id="mmu1"

---

# 10. Page Concept

Memory divided into fixed-size blocks:

```text
pages
```

Typical size:
```text
4 KB
```

---

# 11. Why Pages?

Advantages:
- easier allocation
- efficient swapping
- memory protection

---

# 12. Page Table

Data structure storing:
```text
virtual-to-physical mappings
```

Maintained by kernel.

---

# 13. Page Table Entry (PTE)

Contains:
- physical page address
- permissions
- flags

---

# 14. Memory Protection

Kernel protects:
- processes from each other
- user space from kernel space

---

# 15. User Space vs Kernel Space

| Space | Access |
|------|---------|
| User Space | Applications |
| Kernel Space | Kernel only |

---

# 16. Memory Regions in Process

| Region | Purpose |
|--------|---------|
| Text | Program code |
| Data | Global variables |
| Heap | Dynamic allocation |
| Stack | Function calls |

---

# 17. Stack Memory

Used for:
- local variables
- function calls
- return addresses

Grows:
```text
downward
```

---

# 18. Heap Memory

Dynamic allocation using:
- malloc()
- calloc()
- realloc()

Grows:
```text
upward
```

---

# 19. Dynamic Allocation Flow

```text
malloc()
    ↓
glibc allocator
    ↓
brk()/mmap()
    ↓
Kernel Allocates Pages
``` id="heap1"

---

# 20. Physical Page Allocator

Kernel manages free pages using:
```text
buddy allocator
```

---

# 21. Buddy Allocator

Divides memory into:
```text
power-of-two blocks
```

Efficiently handles:
- allocation
- deallocation

---

# 22. Buddy Allocator Flow

```text
Free Memory Pool
        ↓
Split Larger Blocks
        ↓
Allocate Requested Size
``` id="buddy1"

---

# 23. Slab Allocator

Kernel object allocator.

Optimized for:
- small objects
- kernel structures

---

# 24. Slab Cache Example

Used for:
- task_struct
- inode
- file objects

---

# 25. Slab Allocator Benefits

- reduced fragmentation
- fast allocation
- object reuse

---

# 26. Kernel Memory Allocation APIs

| API | Purpose |
|-----|---------|
| kmalloc | Small allocations |
| vmalloc | Non-contiguous memory |
| kzalloc | Zeroed memory |
| kfree | Free memory |

---

# 27. kmalloc Example

```c
char *buf;

buf = kmalloc(1024, GFP_KERNEL);
``` id="km1"

---

# 28. Freeing Memory

```c
kfree(buf);
``` id="kf1"

---

# 29. vmalloc

Allocates:
```text
virtually contiguous memory
```

Physical pages may be fragmented.

---

# 30. Memory Zones

Linux divides RAM into zones.

| Zone | Purpose |
|------|---------|
| DMA | DMA-capable memory |
| Normal | General memory |
| HighMem | Large memory systems |

---

# 31. DMA Memory

Used for:
```text
Direct Memory Access
```

by hardware devices.

---

# 32. DMA Flow

```text
Device
  ↓
DMA Controller
  ↓
RAM
```

CPU involvement minimized.

---

# 33. Page Cache

Kernel caches:
- filesystem data
- disk blocks

in RAM for performance.

---

# 34. Filesystem Read Flow

```text
Application Read
      ↓
Page Cache Check
      ↓
Disk Access if Needed
``` id="cache1"

---

# 35. Swapping

Inactive pages moved to:
```text
swap space
```

when RAM low.

---

# 36. Swap Flow

```text
RAM Full
   ↓
Inactive Pages Selected
   ↓
Write to Swap
   ↓
Free RAM
``` id="swap1"

---

# 37. Embedded Linux and Swap

Embedded systems often:
```text
disable swap
```

Reasons:
- limited storage
- wear concerns
- deterministic behavior

---

# 38. Memory Fragmentation

Types:
- internal fragmentation
- external fragmentation

---

# 39. Internal Fragmentation

Unused memory inside allocated block.

---

# 40. External Fragmentation

Free memory scattered into small blocks.

---

# 41. Out-of-Memory (OOM)

Occurs when:
```text
system cannot allocate memory
```

Kernel may invoke:
```text
OOM Killer
```

---

# 42. OOM Killer

Selects processes to terminate:
```text
to recover memory
```

---

# 43. Memory Mapping (mmap)

Maps files/devices into memory.

Example:

```c
mmap()
```

---

# 44. mmap Flow

```text
Application mmap()
       ↓
Kernel Creates Mapping
       ↓
MMU Translates Accesses
``` id="mmap1"

---

# 45. Shared Memory

Multiple processes share:
```text
same physical pages
```

Used for:
- IPC
- high-speed communication

---

# 46. Copy-on-Write (COW)

After fork():
- parent/child share pages
- copied only when modified

---

# 47. COW Flow

```text
fork()
   ↓
Shared Pages
   ↓
Write Attempt
   ↓
Page Copy
``` id="cow1"

---

# 48. Page Fault

Occurs when:
```text
requested page not mapped
```

---

# 49. Page Fault Handling

```text
CPU Access
   ↓
Missing Page
   ↓
Page Fault Exception
   ↓
Kernel Handles Fault
``` id="pf1"

---

# 50. Types of Page Faults

| Type | Meaning |
|------|---------|
| Minor | Page already in RAM |
| Major | Disk access required |
| Invalid | Illegal access |

---

# 51. Cache and Buffers

Kernel uses memory for:
- disk cache
- network buffers
- filesystem metadata

---

# 52. Memory in Embedded Linux

Embedded systems often have:
- limited RAM
- no swap
- deterministic requirements

Thus memory management highly optimized.

---

# 53. Device Tree and Memory

Example:

```dts
memory@80000000 {
    reg = <0x80000000 0x40000000>;
};
``` id="dt1"

---

# 54. Kernel Memory Layout

```text
+----------------------+
| User Space           |
+----------------------+
| Kernel Space         |
|----------------------|
| Kernel Code          |
| Slab Allocator       |
| vmalloc Area         |
| Page Cache           |
+----------------------+
``` id="kern1"

---

# 55. Viewing Memory Information

---

## RAM Usage

```bash
free -h
``` id="free1"

---

## Detailed Memory Info

```bash
cat /proc/meminfo
``` id="meminfo1"

---

## Process Memory

```bash
top
``` id="top1"

---

# 56. Kernel Memory Debugging

---

## Slab Information

```bash
cat /proc/slabinfo
``` id="slab1"

---

## Virtual Memory Stats

```bash
vmstat
``` id="vm1"

---

# 57. Common Memory Problems

| Problem | Description |
|---------|-------------|
| Memory leak | Unreleased memory |
| Fragmentation | Broken free memory |
| Buffer overflow | Memory corruption |
| OOM | Out of memory |

---

# 58. Memory Leak Example

```c
ptr = kmalloc(100, GFP_KERNEL);

/* forgot kfree(ptr) */
``` id="leak1"

---

# 59. Complete Memory Allocation Flow

```text
Application Requests Memory
           ↓
Kernel Virtual Allocation
           ↓
Page Allocator
           ↓
Physical Pages Assigned
           ↓
MMU Updates Page Tables
           ↓
CPU Accesses Memory
``` id="final1"

---

# 60. Advantages of Linux Memory Management

- process isolation
- virtual memory abstraction
- efficient RAM usage
- protection mechanisms
- scalability

---

# 61. Summary

- Linux memory manager controls RAM usage
- Uses virtual memory and MMU translation
- Memory divided into pages
- Buddy allocator manages physical pages
- Slab allocator optimizes kernel allocations
- Essential for multitasking and embedded Linux systems

---

````
