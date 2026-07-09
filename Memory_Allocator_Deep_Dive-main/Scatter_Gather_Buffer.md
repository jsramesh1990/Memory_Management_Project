# Scatter Gather Buffer

# Table of Contents

1. Introduction
2. What is Scatter Gather?
3. Why Do We Need Scatter Gather?
4. Scatter vs Gather
5. Scatter Gather DMA Architecture
6. Scatter Gather Descriptors
7. Memory Layout
8. How Scatter Gather Works
9. Scatter Gather vs Single DMA Buffer
10. Scatter Gather vs Circular Buffer
11. Scatter Gather vs Ping Pong Buffer
12. Implementation Concept
13. Scatter Gather in Embedded Systems
14. Scatter Gather in Linux Device Drivers
15. Scatter Gather APIs in Linux
16. Applications
17. Advantages
18. Disadvantages
19. Best Practices
20. Interview Questions
21. Real World Examples
22. Summary

---

# 1. Introduction

High-speed devices often transfer large amounts of data.

Examples:

* Ethernet Controllers
* SSD/NVMe
* USB Controllers
* Audio Devices
* Camera Interfaces
* GPUs
* Network Cards

A single continuous DMA buffer is not always practical because:

* Memory may be fragmented.
* Large contiguous buffers may not exist.
* Dynamic memory allocation can fail.
* Copying data wastes CPU cycles.

To solve this problem:

```text
Scatter Gather DMA
```

is used.

Scatter Gather allows DMA to transfer data from multiple memory regions using a list of descriptors.

It is one of the most important concepts in:

* Linux Device Drivers
* Networking
* Embedded Systems
* Storage Drivers
* High Speed DMA Systems

---

# 2. What is Scatter Gather?

Scatter Gather is a DMA technique where:

### Scatter

DMA writes incoming data into:

```text
Multiple Buffers
```

instead of one large buffer.

---

### Gather

DMA reads data from:

```text
Multiple Buffers
```

and transmits it as one continuous stream.

---

Simple view:

```text
Multiple Memory Blocks

↓

DMA Engine

↓

Single Data Stream
```

or

```text
Single Stream

↓

DMA Engine

↓

Multiple Memory Blocks
```

---

# 3. Why Do We Need Scatter Gather?

Suppose Ethernet receives:

```text
8 KB Packet
```

Available memory:

```text
Buffer A = 2 KB

Buffer B = 2 KB

Buffer C = 2 KB

Buffer D = 2 KB
```

No continuous:

```text
8 KB
```

memory exists.

---

Without Scatter Gather:

```text
Allocate 8 KB

Copy data

CPU overhead
```

Bad.

---

With Scatter Gather:

```text
DMA

↓

Buffer A

↓

Buffer B

↓

Buffer C

↓

Buffer D
```

No copy required.

High performance.

---

# 4. Scatter vs Gather

---

## Scatter

One stream becomes:

```text
Stream

↓

DMA

↓

Buffer A

Buffer B

Buffer C
```

Incoming data is scattered.

---

Example:

```text
ABCDEFGHIJKL
```

DMA:

```text
ABCD -> Buffer A

EFGH -> Buffer B

IJKL -> Buffer C
```

---

## Gather

Multiple buffers become:

```text
Buffer A

Buffer B

Buffer C

↓

DMA

↓

Single Stream
```

---

Example:

```text
ABCD

EFGH

IJKL
```

DMA sends:

```text
ABCDEFGHIJKL
```

as one stream.

---

# 5. Scatter Gather DMA Architecture

General architecture:

```text
Memory Buffers

↓

Descriptor Table

↓

DMA Controller

↓

Peripheral
```

DMA:

1. Reads Descriptor
2. Transfers Buffer
3. Moves to Next Descriptor
4. Repeats

CPU does not intervene.

---

# 6. Scatter Gather Descriptors

Descriptors describe:

```text
Memory Address

Transfer Length

Control Flags

Next Descriptor
```

---

Typical Descriptor:

```c
struct sg_desc
{

void *addr;

uint32_t length;

uint32_t control;

struct sg_desc *next;

};
```

---

Example:

```text
Descriptor 1

↓

Buffer A


Descriptor 2

↓

Buffer B


Descriptor 3

↓

Buffer C
```

DMA follows the chain.

---

# 7. Memory Layout

Example:

Memory:

```text
1000 - 1999

Buffer A


5000 - 5999

Buffer B


9000 - 9999

Buffer C
```

Notice:

```text
Not Continuous
```

---

Descriptor List:

```text
Desc1

addr = Buffer A

len = 1000


Desc2

addr = Buffer B

len = 1000


Desc3

addr = Buffer C

len = 1000
```

DMA treats them as:

```text
One Logical Buffer
```

---

# 8. How Scatter Gather Works

Step 1

CPU creates:

```text
Descriptor List
```

---

Step 2

DMA gets:

```text
Descriptor 1
```

Transfers:

```text
Buffer A
```

---

Step 3

DMA automatically loads:

```text
Descriptor 2
```

Transfers:

```text
Buffer B
```

---

Step 4

DMA loads:

```text
Descriptor 3
```

Transfers:

```text
Buffer C
```

---

Step 5

DMA interrupt:

```text
Transfer Complete
```

CPU notified.

---

# 9. Scatter Gather vs Single DMA Buffer

| Feature           | Scatter Gather | Single Buffer |
| ----------------- | -------------- | ------------- |
| Contiguous Memory | Not Required   | Required      |
| Memory Efficiency | High           | Low           |
| CPU Copy          | No             | Sometimes     |
| Large Transfers   | Excellent      | Difficult     |
| Complexity        | High           | Simple        |

---

Single DMA:

```text
DMA

↓

8 KB Buffer
```

---

Scatter Gather:

```text
DMA

↓

2 KB

↓

2 KB

↓

2 KB

↓

2 KB
```

---

# 10. Scatter Gather vs Circular Buffer

| Feature     | Scatter Gather | Circular Buffer |
| ----------- | -------------- | --------------- |
| Buffers     | Multiple       | One             |
| Memory Type | Fragmented     | Continuous      |
| Streaming   | Excellent      | Excellent       |
| DMA Support | Excellent      | Excellent       |
| Complexity  | High           | Moderate        |

---

Circular:

```text
Ring Memory
```

---

Scatter Gather:

```text
Descriptor Chain

↓

Fragmented Memory
```

---

# 11. Scatter Gather vs Ping Pong Buffer

| Feature         | Scatter Gather | Ping Pong  |
| --------------- | -------------- | ---------- |
| Buffers         | Many           | Two        |
| Memory Type     | Fragmented     | Continuous |
| Complexity      | High           | Low        |
| Streaming       | Excellent      | Excellent  |
| Large Transfers | Excellent      | Moderate   |

---

Ping Pong:

```text
Ping

↓

Pong

↓

Ping
```

---

Scatter Gather:

```text
Buffer1

↓

Buffer2

↓

Buffer3

↓

Buffer4
```

---

# 12. Implementation Concept

Descriptor:

```c
typedef struct
{

void *addr;

uint32_t len;

struct desc *next;

}desc_t;
```

---

Example:

```c
desc1.addr = buf1;

desc1.next = &desc2;


desc2.addr = buf2;

desc2.next = &desc3;


desc3.addr = buf3;

desc3.next = NULL;
```

DMA starts:

```text
desc1
```

and follows links.

---

# 13. Scatter Gather in Embedded Systems

Used in:

---

## Ethernet MAC

```text
RX DMA

↓

Descriptor Ring

↓

Packet Buffers
```

---

## Camera Interfaces

```text
Camera

↓

DMA

↓

Frame Buffer 1

↓

Frame Buffer 2

↓

Frame Buffer 3
```

---

## USB Controllers

Transfers:

```text
Endpoint Buffers

↓

Descriptor Chain
```

---

## Audio DMA

Stores:

```text
PCM Block 1

PCM Block 2

PCM Block 3
```

---

# 14. Scatter Gather in Linux Device Drivers

Linux uses Scatter Gather extensively.

Examples:

---

## Ethernet Drivers

```text
NIC

↓

DMA

↓

skb fragments

↓

Network Stack
```

---

## USB Drivers

```text
USB Device

↓

DMA

↓

SG List

↓

USB Core
```

---

## Storage Drivers

Examples:

* SATA
* NVMe
* SCSI

---

NVMe:

```text
PRP List

or

SGL

↓

DMA

↓

SSD
```

---

## GPU Drivers

Frame buffers:

```text
Scatter Gather Tables
```

---

# 15. Scatter Gather APIs in Linux

Linux provides:

```c
struct scatterlist;
```

---

Initialize:

```c
sg_init_table();
```

---

Set buffer:

```c
sg_set_buf();
```

---

Map:

```c
dma_map_sg();
```

---

Unmap:

```c
dma_unmap_sg();
```

---

Example:

```c
struct scatterlist sg[4];
```

DMA treats:

```text
4 Buffers

↓

One DMA Transfer
```

---

# 16. Applications

Scatter Gather is used in:

---

### Networking

* Ethernet
* WiFi
* Packet Buffers

---

### Storage

* NVMe
* SATA
* SCSI

---

### USB

* Endpoint Transfers

---

### Audio

* PCM Streams

---

### Video

* Camera Frames
* Video Buffers

---

### Graphics

* Frame Buffers

---

### Embedded Systems

* Ethernet MAC
* USB DMA
* Camera Interfaces

---

# 17. Advantages

## No Large Contiguous Buffer Required

Works with:

```text
Fragmented Memory
```

---

## Zero Copy Transfers

No:

```text
CPU memcpy()
```

required.

---

## High Performance

Ideal for:

```text
MB/s

GB/s
```

transfers.

---

## Efficient Memory Usage

Uses existing buffers.

---

## Excellent for Large Data

Supports:

* Packets
* Frames
* Files
* Audio Streams

---

# 18. Disadvantages

## Complex

Need:

```text
Descriptor Lists

DMA Mapping

Synchronization
```

---

## More Memory for Descriptors

Descriptor tables consume memory.

---

## Cache Coherency Issues

Need:

```text
Clean Cache

Invalidate Cache
```

---

## Hard Debugging

Problems:

* Descriptor corruption
* DMA hangs
* Wrong address mapping

are difficult to debug.

---

# 19. Best Practices

✔ Align descriptors.

---

✔ Keep descriptors in DMA accessible memory.

---

✔ Use Linux:

```c
dma_map_sg()
```

instead of manual mapping.

---

✔ Maintain cache:

```text
Clean

Invalidate
```

for DMA.

---

✔ Use Scatter Gather for:

* Ethernet
* NVMe
* USB
* Large transfers

---

# 20. Interview Questions

### Q1. What is Scatter Gather DMA?

DMA transfers data using multiple memory buffers linked by descriptors.

---

### Q2. Difference between Scatter and Gather?

Scatter:

```text
Single Stream

↓

Multiple Buffers
```

---

Gather:

```text
Multiple Buffers

↓

Single Stream
```

---

### Q3. Why use Scatter Gather?

Because:

* Large contiguous memory may not exist.
* Avoids copying.
* Improves performance.

---

### Q4. Linux structure for Scatter Gather?

```c
struct scatterlist
```

---

### Q5. Linux DMA API?

```c
dma_map_sg()

dma_unmap_sg()
```

---

### Q6. Applications?

* Ethernet
* USB
* NVMe
* Audio
* Camera
* GPU

---

# 21. Real World Examples

---

## Ethernet Driver

```text
NIC

↓

DMA

↓

Packet Fragment 1

Packet Fragment 2

Packet Fragment 3

↓

Network Stack
```

---

## NVMe SSD

```text
PRP List

or

SGL

↓

DMA

↓

SSD Controller
```

---

## USB Controller

```text
DMA

↓

Descriptor List

↓

Endpoint Buffers
```

---

## Camera Driver

```text
Camera

↓

DMA

↓

Frame Buffer A

↓

Frame Buffer B

↓

Frame Buffer C
```

---

# 22. Summary

Scatter Gather Buffer is a DMA technique where:

```text
Multiple Memory Buffers

↓

Descriptor Chain

↓

DMA Engine

↓

Peripheral
```

DMA transfers data without requiring:

* Large contiguous memory
* CPU copying
* Intermediate buffers

Advantages:

✔ Zero Copy

✔ High Throughput

✔ Efficient Memory Usage

✔ Excellent for Large Transfers

Widely used in:

* Ethernet Drivers
* Linux Device Drivers
* NVMe SSD
* USB
* Audio
* Video
* Networking
* Embedded Systems

Scatter Gather DMA is one of the most important concepts in:

**Linux Device Drivers + Networking + Storage Drivers + High-Speed DMA Systems + Embedded Ethernet/USB Controllers.**
