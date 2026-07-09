# mmap (Memory Mapping in Linux)

## Table of Contents

1. [Introduction](#introduction)
2. [What is mmap()?](#what-is-mmap)
3. [Why Do We Need mmap()?](#why-do-we-need-mmap)
4. [Where is mmap() Used?](#where-is-mmap-used)
5. [Function Prototype](#function-prototype)
6. [Parameters of mmap()](#parameters-of-mmap)
7. [Return Value](#return-value)
8. [How mmap() Works Internally](#how-mmap-works-internally)
9. [Memory Layout Before mmap()](#memory-layout-before-mmap)
10. [Memory Layout After mmap()](#memory-layout-after-mmap)
11. [Types of Memory Mapping](#types-of-memory-mapping)
12. [MAP_SHARED](#map_shared)
13. [MAP_PRIVATE](#map_private)
14. [Anonymous Mapping](#anonymous-mapping)
15. [File Mapping Example](#file-mapping-example)
16. [Anonymous Memory Example](#anonymous-memory-example)
17. [mmap() in Device Drivers](#mmap-in-device-drivers)
18. [Driver mmap() Callback](#driver-mmap-callback)
19. [Advantages](#advantages)
20. [Disadvantages](#disadvantages)
21. [Common Mistakes](#common-mistakes)
22. [Interview Questions](#interview-questions)
23. [Quick Revision](#quick-revision)
24. [Conclusion](#conclusion)

---

# Introduction

`mmap()` stands for:

```text
Memory MAP
```

It is a Linux system call used to map:

- Files
- Devices
- Shared Memory
- Anonymous Memory

into a process's virtual address space.

---

## Used In

- Linux Applications
- Device Drivers
- Shared Memory IPC
- Databases
- Graphics Drivers
- Embedded Linux
- DMA Buffers
- Video Drivers

---

# What is mmap()?

## Definition

`mmap()` creates a mapping between:

```text
Virtual Memory

        ↕

File / Device / Physical Memory
```

allowing applications to access them as normal memory.

---

## Interview Answer

mmap() maps a file or device into a process virtual address space so that the process can access it like normal memory.

---

# Why Do We Need mmap()?

Without mmap():

```text
File

↓

read()

↓

Kernel Buffer

↓

copy_to_user()

↓

User Buffer
```

Two copies happen.

---

With mmap():

```text
File

↓

Kernel Page Cache

↓

Mapped Directly

↓

User Virtual Memory
```

No explicit copy.

---

## Benefits

✔ Faster access

✔ Zero-copy mechanism

✔ Shared memory

✔ Large file handling

✔ Device access

---

# Where is mmap() Used?

Used in:

- File access
- Shared memory IPC
- Graphics
- Databases
- Video streaming
- Linux Device Drivers
- DMA buffers
- Framebuffer drivers

---

# Function Prototype

```c
#include <sys/mman.h>

void *mmap(
        void *addr,
        size_t length,
        int prot,
        int flags,
        int fd,
        off_t offset
);
```

---

# Parameters of mmap()

| Parameter | Description |
|----------|-------------|
| addr | Starting virtual address |
| length | Number of bytes |
| prot | Memory protection |
| flags | Mapping type |
| fd | File descriptor |
| offset | Offset in file |

---

# Return Value

Success:

```c
void *
```

Failure:

```c
MAP_FAILED
```

Example:

```c
if(ptr == MAP_FAILED)
{
    perror("mmap");
}
```

---

# How mmap() Works Internally

```text
User Process

↓

mmap()

↓

System Call

↓

Kernel

↓

Create VMA

↓

Update Page Tables

↓

Map File/Device

↓

Return Virtual Address
```

---

# Memory Layout Before mmap()

```text
Virtual Address Space

+----------------+

| Stack          |

+----------------+

| Heap           |

+----------------+

| Data Segment   |

+----------------+

| Text Segment   |

+----------------+
```

---

# Memory Layout After mmap()

```text
Virtual Address Space

+----------------+

| Stack          |

+----------------+

| Mapped Region  |

| mmap() Area    |

+----------------+

| Heap           |

+----------------+

| Data Segment   |

+----------------+

| Text Segment   |

+----------------+
```

---

# Types of Memory Mapping

```text
mmap()

│

├── MAP_SHARED

├── MAP_PRIVATE

└── MAP_ANONYMOUS
```

---

# MAP_SHARED

## Definition

Updates are visible to:

- Other processes
- Underlying file

---

## Example

```c
ptr = mmap(
        NULL,
        4096,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        fd,
        0
);
```

---

## Use Cases

- Shared Memory
- IPC
- Shared Buffers

---

# MAP_PRIVATE

## Definition

Creates a private copy.

Changes:

```text
Process Only

↓

Not Written Back

↓

Copy-On-Write
```

---

## Example

```c
ptr = mmap(
        NULL,
        4096,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
);
```

---

## Use Cases

- Reading files

- Databases

- Configuration files

---

# Anonymous Mapping

## Definition

Maps memory not associated with a file.

---

## Example

```c
ptr = mmap(
        NULL,
        4096,
        PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS,
        -1,
        0
);
```

---

## Uses

- Dynamic Memory

- Shared Memory

- Process Heaps

---

# File Mapping Example

```c
#include <stdio.h>

#include <sys/mman.h>

#include <fcntl.h>

int main()
{
    int fd;

    char *ptr;

    fd = open("test.txt", O_RDONLY);

    ptr = mmap(
            NULL,
            4096,
            PROT_READ,
            MAP_PRIVATE,
            fd,
            0);

    printf("%s", ptr);

    return 0;
}
```

---

# Anonymous Memory Example

```c
#include <stdio.h>

#include <sys/mman.h>

int main()
{
    int *ptr;

    ptr = mmap(
            NULL,
            sizeof(int),
            PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANONYMOUS,
            -1,
            0);

    *ptr = 100;

    printf("%d", *ptr);

    return 0;
}
```

---

# mmap() in Device Drivers

Device drivers expose hardware memory to user space using mmap.

---

Example:

```text
User Application

↓

mmap()

↓

Driver mmap()

↓

Physical Memory

↓

Device Registers

↓

Hardware
```

---

## Used In

- Framebuffer Drivers

- PCI Drivers

- DMA Buffers

- Video Drivers

- GPU Drivers

- Network Drivers

---

# Driver mmap() Callback

Prototype:

```c
int mmap(
        struct file *file,

        struct vm_area_struct *vma
);
```

---

Example:

```c
static int my_mmap(
        struct file *file,

        struct vm_area_struct *vma)
{
    return remap_pfn_range(
                vma,

                vma->vm_start,

                physical_addr >> PAGE_SHIFT,

                size,

                vma->vm_page_prot);
}
```

---

# Internal Flow

```text
Application

↓

mmap()

↓

Kernel

↓

Driver mmap()

↓

remap_pfn_range()

↓

Page Table Updated

↓

User Space Accesses Hardware
```

---

# Advantages

✔ Zero-copy access

✔ Faster than read/write

✔ Shared memory support

✔ Efficient large file access

✔ Device memory mapping

✔ Reduced CPU overhead

✔ Useful in DMA

---

# Disadvantages

✘ Complex debugging

✘ Page fault overhead

✘ Synchronization issues

✘ Alignment restrictions

✘ Security concerns

---

# Common Mistakes

## 1. Forgetting munmap()

Wrong:

```c
ptr = mmap(...);

/* Missing */

munmap();
```

Leads to:

```text
Memory Leak
```

---

## 2. Invalid Offset

Wrong:

```c
offset = 123;
```

Offset must be:

```text
Page Aligned
```

Usually:

```text
4096 Bytes
```

---

## 3. Accessing MAP_FAILED

Wrong:

```c
ptr = mmap(...);

*ptr = 10;
```

Without checking:

```c
if(ptr == MAP_FAILED)
```

---

# Interview Questions

### What is mmap()?

Memory mapping system call.

---

### Why mmap() is faster than read()?

Because:

```text
No Extra Copy

↓

Zero Copy

↓

Better Performance
```

---

### What is MAP_SHARED?

Changes are visible to:

- Other processes
- Underlying file

---

### What is MAP_PRIVATE?

Private copy using:

```text
Copy-On-Write
```

---

### What is Anonymous Mapping?

Memory mapping without file association.

---

### Which drivers use mmap()?

- Framebuffer Drivers

- PCI Drivers

- DMA Drivers

- GPU Drivers

- Video Drivers

---

### Which kernel API is used inside driver mmap()?

```c
remap_pfn_range()
```

---

# Quick Revision

```text
mmap()

↓

Maps

↓

File

OR

Device

OR

Anonymous Memory

↓

Creates VMA

↓

Updates Page Tables

↓

Returns Virtual Address

↓

Zero Copy Access

↓

Fast Access

↓

Used In

Framebuffer

DMA

PCI

GPU

Video Drivers
```

---

# Conclusion

`mmap()` is one of the most important Linux system calls used for mapping files, devices, and memory into a process's virtual address space.

It provides:

- Zero-copy access
- Faster file operations
- Shared memory support
- Hardware memory mapping
- Efficient DMA buffer sharing

Understanding `mmap()` is essential for:

- Embedded Linux
- Linux Device Drivers
- DMA Drivers
- Framebuffer Drivers
- GPU Drivers
- High Performance Applications
- System Programming
