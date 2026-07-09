# MPSC Ring Buffer (Multiple Producer Single Consumer)

# Table of Contents

1. Introduction
2. What is MPSC Ring Buffer?
3. Why Do We Need MPSC?
4. Basic Concept
5. Memory Layout
6. Producer-Consumer Model
7. Concurrency Challenges
8. Head and Tail Design
9. Lock-Free vs Lock-Based MPSC
10. Memory Ordering Requirements
11. Implementation in C (Conceptual)
12. MPSC vs SPSC
13. MPSC vs MPMC
14. MPSC in Embedded Systems
15. MPSC in Linux Kernel
16. Applications
17. Advantages
18. Disadvantages
19. Best Practices
20. Interview Questions
21. Real-World Examples
22. Summary

---

# 1. Introduction

An **MPSC Ring Buffer** stands for:

```text id="mpsc1"
Multiple Producer Single Consumer Ring Buffer
```

It is a concurrent data structure where:

* Many producers write data concurrently
* Only one consumer reads data
* Memory is reused in circular fashion

This is widely used in:

* Logging systems
* Networking stacks
* Kernel event tracing
* Task queues
* DMA + ISR systems
* Multi-threaded applications

---

# 2. What is MPSC Ring Buffer?

MPSC means:

```text id="mpsc2"
Multiple Producers

Single Consumer
```

Example:

```text id="mpsc3"
Thread A ─┐
Thread B ─┼──► Ring Buffer ───► Consumer Thread
Thread C ─┘
```

All producers push data into the same buffer.

One consumer processes it sequentially.

---

# 3. Why Do We Need MPSC?

In real systems:

* Many sources generate data
* One worker processes it

Example:

```text id="mpsc4"
Multiple sensors / threads / interrupts

↓

Single processing task
```

Without MPSC:

* Shared queue corruption
* Race conditions
* Lock contention

---

# 4. Basic Concept

Structure:

```text id="mpsc5"
Producers → [ Ring Buffer ] → Consumer
```

Each producer:

* Writes data
* Updates shared tail safely

Consumer:

* Reads data
* Updates head

---

# 5. Memory Layout

Example buffer:

```c id="mpsc6"
uint8_t buffer[8];
```

Layout:

```text id="mpsc7"
+---+---+---+---+---+---+---+---+
| A | B | C | D | E | F | G | H |
+---+---+---+---+---+---+---+---+
```

Indices:

```text id="mpsc8"
head → next read
tail → next write (shared among producers)
```

---

# 6. Producer-Consumer Model

Producers:

```text id="mpsc9"
Producer 1: writes logs
Producer 2: writes events
Producer 3: writes sensor data
```

Consumer:

```text id="mpsc10"
Single worker thread processes all data
```

Flow:

```text id="mpsc11"
Producers → Queue → Consumer
```

---

# 7. Concurrency Challenges

Unlike SPSC:

MPSC has multiple writers.

Problems:

* Race condition on tail
* Overwriting data
* Lost updates
* Memory corruption

Example issue:

```text id="mpsc12"
Producer A reads tail = 5
Producer B reads tail = 5
Both write to index 5 ❌
```

---

# 8. Head and Tail Design

Common approach:

* `head` → only consumer modifies
* `tail` → shared among producers

But tail must be protected.

Two approaches:

---

## 1. Lock-Based Tail Update

```text id="mpsc13"
mutex lock → update tail → unlock
```

---

## 2. Lock-Free Atomic Tail

```text id="mpsc14"
atomic_fetch_add(&tail, 1)
```

Each producer gets unique slot.

---

# 9. Lock-Free vs Lock-Based MPSC

| Type       | Performance | Complexity | Safety |
| ---------- | ----------- | ---------- | ------ |
| Lock-Based | Medium      | Easy       | High   |
| Lock-Free  | High        | Complex    | Hard   |

---

Lock-free MPSC uses:

```text id="mpsc15"
atomic operations
memory barriers
compare-and-swap
```

---

# 10. Memory Ordering Requirements

Critical in MPSC:

Producer must ensure:

```text id="mpsc16"
1. Write data
2. Publish index (tail)
```

Consumer must ensure:

```text id="mpsc17"
1. Read tail
2. Read data safely
```

Correct ordering requires:

* release semantics (producer)
* acquire semantics (consumer)

---

Example:

```c id="mpsc18"
atomic_store_release(&tail, new_tail);
```

```c id="mpsc19"
tail = atomic_load_acquire(&tail);
```

---

# 11. Implementation in C (Conceptual)

Simplified MPSC:

```c id="mpsc20"
typedef struct {
    uint8_t *buf;
    uint32_t size;
    atomic_uint tail;
    uint32_t head;
} mpsc_ring_t;
```

---

Producer:

```c id="mpsc21"
int push(mpsc_ring_t *q, uint8_t data)
{
    uint32_t pos = atomic_fetch_add(&q->tail, 1);

    if ((pos - q->head) >= q->size)
        return -1; // full

    q->buf[pos % q->size] = data;
    return 0;
}
```

---

Consumer:

```c id="mpsc22"
int pop(mpsc_ring_t *q, uint8_t *data)
{
    if (q->head == atomic_load(&q->tail))
        return -1;

    *data = q->buf[q->head % q->size];
    q->head++;
    return 0;
}
```

---

# 12. MPSC vs SPSC

| Feature         | SPSC      | MPSC     |
| --------------- | --------- | -------- |
| Producers       | 1         | Many     |
| Consumers       | 1         | 1        |
| Complexity      | Low       | Medium   |
| Performance     | Very High | High     |
| Synchronization | Minimal   | Required |

---

SPSC:

```text id="mpsc23"
Fast + Simple
```

MPSC:

```text id="mpsc24"
Flexible + Scalable
```

---

# 13. MPSC vs MPMC

| Feature          | MPSC   | MPMC      |
| ---------------- | ------ | --------- |
| Producers        | Many   | Many      |
| Consumers        | One    | Many      |
| Complexity       | Medium | Very High |
| Lock-Free Design | Easier | Hardest   |

---

MPMC requires:

```text id="mpsc25"
full concurrency control
CAS loops
hazard pointers
```

---

# 14. MPSC in Embedded Systems

Used in:

---

## Logging Systems

```text id="mpsc26"
ISR logs
RTOS tasks
Debug prints
```

---

## Sensor Fusion

```text id="mpsc27"
Multiple sensors → single processing task
```

---

## UART + DMA + ISR

```text id="mpsc28"
Multiple interrupts → one processing task
```

---

## Event Queues

```text id="mpsc29"
GPIO interrupts → event handler
```

---

# 15. MPSC in Linux Kernel

Linux uses MPSC concepts in:

* printk log buffer
* kernel tracing
* networking
* scheduler queues

---

Example:

```text id="mpsc30"
Multiple CPUs → kernel log buffer → console thread
```

---

Trace subsystem:

```text id="mpsc31"
CPU0 events ┐
CPU1 events ├──► ring buffer → user space tools
CPU2 events ┘
```

---

# 16. Applications

MPSC Ring Buffers are used in:

* Logging systems
* Event dispatch systems
* Network packet processing
* Kernel tracing
* Multi-threaded pipelines
* Sensor data aggregation
* RTOS messaging systems

---

# 17. Advantages

## Scalable Input

Multiple producers supported.

---

## Efficient Processing

Single consumer simplifies logic.

---

## High Throughput

Better than mutex queues.

---

## Suitable for Logging Systems

Common in:

```text id="mpsc32"
debug logs
telemetry systems
event queues
```

---

# 18. Disadvantages

## More Complex than SPSC

Requires:

* atomics
* memory barriers

---

## Tail Contention

All producers compete for:

```text id="mpsc33"
same counter
```

---

## Overflow Risk

If producers > consumer speed:

```text id="mpsc34"
data loss occurs
```

---

## Hard Debugging

Concurrency bugs are subtle.

---

# 19. Best Practices

✔ Use atomic operations for tail

---

✔ Use acquire/release semantics

---

✔ Prefer power-of-2 buffer size

---

✔ Avoid busy waiting

---

✔ Use batching (important optimization)

---

✔ Ensure memory alignment

---

✔ Keep consumer fast

---

# 20. Interview Questions

### Q1. What is MPSC Ring Buffer?

A ring buffer with:

```text id="mpsc35"
Multiple Producers

Single Consumer
```

---

### Q2. Main challenge?

Managing concurrent writes to tail.

---

### Q3. How is it solved?

Using:

* atomic_fetch_add
* locks
* CAS

---

### Q4. MPSC vs SPSC?

SPSC is lock-free and simpler.

MPSC supports multiple producers.

---

### Q5. Where is it used?

* Logging
* Kernel tracing
* Networking
* RTOS systems

---

# 21. Real-World Examples

---

## Kernel Logging

```text id="mpsc36"
CPU cores → printk buffer → console thread
```

---

## Network Stack

```text id="mpsc37"
Multiple NIC interrupts → packet queue → single processing thread
```

---

## Embedded Logging

```text id="mpsc38"
UART ISR + Timer ISR → log buffer → logger task
```

---

## Telemetry Systems

```text id="mpsc39"
Sensors → event buffer → analytics engine
```

---

# 22. Summary

MPSC Ring Buffer is:

```text id="mpsc40"
Multiple Producers

Single Consumer

Circular Buffer
```

It is used when:

* Many threads generate data
* One thread processes data

Key features:

✔ Scalable input

✔ Efficient processing

✔ High throughput

✔ Widely used in OS kernels and embedded systems

However:

* Requires atomic synchronization
* More complex than SPSC
* Can suffer from contention at tail

MPSC is a core building block for:

**Operating Systems + Networking + Embedded Systems + Logging + Real-Time Event Processing.**
