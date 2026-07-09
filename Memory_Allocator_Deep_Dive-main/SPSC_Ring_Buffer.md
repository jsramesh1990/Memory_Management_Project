# SPSC Ring Buffer (Single Producer Single Consumer Ring Buffer)

# Table of Contents

1. Introduction
2. What is SPSC Ring Buffer?
3. Why Do We Need SPSC Ring Buffer?
4. Basic Concept
5. Memory Layout
6. Producer and Consumer
7. Head and Tail Pointers
8. Full and Empty Conditions
9. Lock-Free Design
10. Memory Ordering and Barriers
11. Implementation in C
12. SPSC Ring Buffer vs Circular Buffer
13. SPSC vs MPSC vs MPMC
14. SPSC in Embedded Systems
15. SPSC in Linux Kernel
16. Applications
17. Advantages
18. Disadvantages
19. Best Practices
20. Interview Questions
21. Real-World Examples
22. Summary

---

# 1. Introduction

An **SPSC Ring Buffer** is a:

```text
Single Producer

+

Single Consumer

+

Circular Buffer
```

where:

* Exactly **one thread/ISR/core** writes data.
* Exactly **one thread/ISR/core** reads data.
* Buffer memory is reused in a circular fashion.

Because there is:

```text
1 Writer

1 Reader
```

the design can often be:

```text
Lock Free
```

which makes it extremely fast.

---

SPSC Ring Buffers are widely used in:

* UART Drivers
* DMA Drivers
* Audio Streaming
* Networking
* Linux Kernel
* RTOS
* Inter-thread Communication
* ISR to Task Communication

---

# 2. What is SPSC Ring Buffer?

SPSC means:

```text
Single Producer

Single Consumer
```

Example:

```text
UART ISR

↓

Producer

↓

Ring Buffer

↓

Consumer

↓

Application Task
```

Producer:

```text
Only Writes
```

Consumer:

```text
Only Reads
```

No locks required in many systems.

---

# 3. Why Do We Need SPSC Ring Buffer?

Suppose:

UART ISR receives bytes.

ISR:

```text
Receives

↓

Stores data

↓

Returns Quickly
```

Application:

```text
Reads Later
```

Need:

* Fast communication
* Low latency
* No data corruption
* Minimal locking

---

Without Ring Buffer:

```text
ISR

↓

Global Variable

↓

Task
```

Problem:

```text
Race Conditions

Lost Data

Synchronization Issues
```

---

SPSC Ring Buffer:

```text
ISR

↓

Producer

↓

Ring Buffer

↓

Consumer

↓

Task
```

Safe and efficient.

---

# 4. Basic Concept

Suppose:

```text
Buffer Size = 8
```

Initially:

```text
+---+---+---+---+---+---+---+---+

|   |   |   |   |   |   |   |   |

+---+---+---+---+---+---+---+---+

Head = 0

Tail = 0
```

---

Producer writes:

```text
A B C
```

```text
+---+---+---+---+---+---+---+---+

| A | B | C |   |   |   |   |   |

+---+---+---+---+---+---+---+---+

Head = 0

Tail = 3
```

---

Consumer reads:

```text
A
```

Head moves:

```text
Head = 1

Tail = 3
```

---

Producer writes more:

```text
D E F G
```

Tail wraps:

```text
+---+---+---+---+---+---+---+---+

| A | B | C | D | E | F | G |   |

+---+---+---+---+---+---+---+---+

Head = 1

Tail = 7
```

---

Producer:

```text
Writes H
```

Tail:

```text
Tail = 0
```

Wrap Around.

---

# 5. Memory Layout

Example:

```c
uint8_t buffer[8];
```

Memory:

```text
buffer

↓

+---+---+---+---+---+---+---+---+

0   1   2   3   4   5   6   7

↑                           ↓

└──────── Wrap Around ──────┘
```

Physical memory is linear.

Logical structure is circular.

---

# 6. Producer and Consumer

Producer:

```text
Only Writes

Updates Tail
```

Consumer:

```text
Only Reads

Updates Head
```

Rule:

```text
Producer NEVER updates Head

Consumer NEVER updates Tail
```

This is the key reason:

```text
SPSC

=

Lock Free
```

in most cases.

---

# 7. Head and Tail Pointers

Head:

```text
Next Read Position
```

Tail:

```text
Next Write Position
```

Example:

```text
Head = 2

Tail = 5
```

Buffer:

```text
+---+---+---+---+---+---+---+---+

A B C D E

+---+---+---+---+---+---+---+---+

    ↑       ↑

  Head    Tail
```

Data available:

```text
C D E
```

---

Update:

```c
head = (head + 1) % size;

tail = (tail + 1) % size;
```

---

Power of 2 optimization:

```c
head = (head + 1) & (size - 1);

tail = (tail + 1) & (size - 1);
```

Very common.

---

# 8. Full and Empty Conditions

---

## Empty

```c
head == tail
```

No data.

---

Example:

```text
Head = 4

Tail = 4
```

Buffer empty.

---

## Full

Usually:

```c
(tail + 1)%size == head
```

Example:

```text
Head = 3

Tail = 2
```

Next write:

```text
Tail becomes Head
```

Buffer Full.

---

One slot is left empty to distinguish:

```text
Full

vs

Empty
```

---

# 9. Lock-Free Design

Most important feature.

Producer:

```text
Updates:

Tail
```

Consumer:

```text
Updates:

Head
```

Therefore:

```text
No Shared Write Variable
```

No:

* Mutex
* Semaphore
* Spinlock

required.

---

Example:

```text
ISR

↓

Tail


Task

↓

Head
```

Independent.

---

# 10. Memory Ordering and Barriers

Modern CPUs:

```text
Out of Order Execution

Store Buffers

Caches
```

can reorder operations.

---

Wrong:

```c
tail++;

buffer[tail] = data;
```

Consumer may:

```text
See Tail Updated

But Data Not Written
```

Bug.

---

Correct:

```c
buffer[tail] = data;

Memory Barrier;

tail++;
```

---

Consumer:

```c
Read Tail;

Memory Barrier;

Read Buffer;
```

---

Common barriers:

```text
Compiler Barrier

CPU Barrier

Acquire Barrier

Release Barrier
```

Linux:

```c
smp_store_release()

smp_load_acquire()
```

---

ARM:

```text
DMB

DSB

ISB
```

---

# 11. Implementation in C

Structure:

```c
typedef struct
{

uint8_t *buf;

uint32_t size;

volatile uint32_t head;

volatile uint32_t tail;

}spsc_ring_t;
```

---

Initialization:

```c
void init(spsc_ring_t *rb,
          uint8_t *buf,
          uint32_t size)
{

rb->buf = buf;

rb->size = size;

rb->head = 0;

rb->tail = 0;

}
```

---

Push:

```c
int push(spsc_ring_t *rb,
         uint8_t data)
{

uint32_t next;


next =

(rb->tail + 1)

&

(rb->size - 1);


if(next == rb->head)

return -1;


rb->buf[rb->tail]

=

data;


rb->tail

=

next;


return 0;

}
```

---

Pop:

```c
int pop(spsc_ring_t *rb,
        uint8_t *data)
{

if(rb->head

==

rb->tail)

return -1;


*data

=

rb->buf[rb->head];


rb->head

=

(rb->head+1)

&

(rb->size-1);


return 0;

}
```

---

# 12. SPSC Ring Buffer vs Circular Buffer

| Feature         | SPSC Ring | Circular Buffer   |
| --------------- | --------- | ----------------- |
| Producer        | One       | Multiple Possible |
| Consumer        | One       | Multiple Possible |
| Lock Free       | Yes       | Not Always        |
| Synchronization | Minimal   | Required          |
| Performance     | Very High | High              |
| Complexity      | Moderate  | Moderate          |

---

Circular Buffer:

```text
General Purpose
```

---

SPSC:

```text
Optimized

for

1 Producer

1 Consumer
```

---

# 13. SPSC vs MPSC vs MPMC

---

## SPSC

```text
1 Producer

1 Consumer
```

Fastest.

---

## MPSC

```text
Many Producers

1 Consumer
```

Need atomics.

---

## SPMC

```text
1 Producer

Many Consumers
```

Need synchronization.

---

## MPMC

```text
Many Producers

Many Consumers
```

Most complex.

---

# 14. SPSC in Embedded Systems

Used heavily in:

---

## UART Driver

```text
UART ISR

↓

SPSC Ring

↓

Application Task
```

---

## ADC Driver

```text
DMA

↓

Producer

↓

SPSC Ring

↓

DSP Task
```

---

## Sensor Data

```text
ISR

↓

Ring

↓

Main Loop
```

---

## Audio Streaming

```text
DMA

↓

Producer

↓

Ring

↓

DSP

↓

Speaker
```

---

# 15. SPSC in Linux Kernel

Linux uses similar concepts in:

* TTY Drivers
* Networking
* Trace Buffers
* Perf Events
* Audio Drivers
* Lock Free Queues

---

Example:

```text
NIC DMA

↓

RX Ring

↓

Network Stack
```

---

Trace Buffers:

```text
Kernel Events

↓

Ring Buffer

↓

Tracing Tools
```

---

# 16. Applications

SPSC Ring Buffers are used in:

* UART Drivers
* DMA Drivers
* ADC Drivers
* Audio Processing
* Networking
* IPC
* Linux Kernel
* RTOS
* Logging Systems
* Sensor Processing

---

# 17. Advantages

## Lock Free

No:

```text
Mutex

Semaphore

Spinlock
```

required.

---

## Very Fast

Operations:

```text
O(1)
```

Push and Pop.

---

## Low Latency

Ideal for:

* ISR
* DMA
* Real Time Systems

---

## Memory Efficient

Single fixed-size buffer.

---

## Cache Friendly

Continuous memory.

---

# 18. Disadvantages

## Only One Producer

Cannot support:

```text
Multiple Writers
```

without extra synchronization.

---

## Only One Consumer

Cannot support:

```text
Multiple Readers
```

directly.

---

## Buffer Overflow Possible

If:

```text
Producer

>

Consumer
```

overflow occurs.

---

## Memory Ordering Issues

Need:

```text
Barriers

Acquire

Release
```

especially on:

* ARM
* Multi-core CPUs

---

# 19. Best Practices

✔ Use:

```text
Power of 2 Size
```

Example:

```text
64

128

256

512
```

---

✔ Use:

```c
index

&

(size-1)
```

instead of:

```c
index % size
```

---

✔ Use:

```text
Acquire / Release
```

barriers.

---

✔ Producer:

```text
Writes Data

↓

Barrier

↓

Updates Tail
```

---

✔ Consumer:

```text
Reads Tail

↓

Barrier

↓

Reads Data
```

---

# 20. Interview Questions

### Q1. What is SPSC Ring Buffer?

A circular buffer with:

```text
1 Producer

1 Consumer
```

---

### Q2. Why is it lock free?

Because:

```text
Producer updates:

Tail Only


Consumer updates:

Head Only
```

No shared write variable.

---

### Q3. Full Condition?

```c
(tail+1)%size==head
```

---

### Q4. Empty Condition?

```c
head==tail
```

---

### Q5. Why use Power of 2 size?

Because:

```c
index&(size-1)
```

is faster than:

```c
index%size
```

---

### Q6. Why use memory barriers?

To prevent:

```text
Compiler Reordering

CPU Reordering
```

which can cause:

```text
Tail Updated

Before

Data Written
```

---

# 21. Real-World Examples

---

## UART ISR → Task

```text
UART ISR

↓

Producer

↓

SPSC Ring

↓

Consumer

↓

Task
```

---

## DMA → DSP

```text
DMA

↓

Producer

↓

SPSC Ring

↓

DSP Task
```

---

## Linux Trace Buffer

```text
Kernel Events

↓

Ring Buffer

↓

User Space
```

---

## Audio Streaming

```text
DMA

↓

Producer

↓

Ring

↓

DSP

↓

Speaker
```

---

# 22. Summary

SPSC Ring Buffer is:

```text
Single Producer

+

Single Consumer

+

Circular Buffer
```

Features:

✔ Lock Free

✔ O(1) Push

✔ O(1) Pop

✔ Low Latency

✔ Cache Friendly

✔ Real Time Friendly

Widely used in:

* UART Drivers
* DMA Drivers
* Audio Processing
* Networking
* Linux Kernel
* RTOS
* Embedded Systems
* Logging Systems

SPSC Ring Buffer is one of the most important data structures for:

**Embedded Systems + Linux Kernel + DMA + ISR-to-Task Communication + Real-Time Lock-Free Programming.**
