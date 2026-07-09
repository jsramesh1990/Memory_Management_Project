# SPMC Ring Buffer (Single Producer Multiple Consumer)

# Table of Contents

1. Introduction
2. What is SPMC Ring Buffer?
3. Why Do We Need SPMC?
4. Basic Concept
5. Memory Layout
6. Producer-Consumer Model
7. Concurrency Challenges
8. Head and Tail Design
9. Lock-Free vs Lock-Based SPMC
10. Memory Ordering Requirements
11. Implementation Concept in C
12. SPMC vs SPSC
13. SPMC vs MPSC vs MPMC
14. SPMC in Embedded Systems
15. SPMC in Linux Kernel
16. Applications
17. Advantages
18. Disadvantages
19. Best Practices
20. Interview Questions
21. Real-World Examples
22. Summary

---

# 1. Introduction

An **SPMC Ring Buffer** stands for:

```text id="spmc1"
Single Producer Multiple Consumers Ring Buffer
```

It is a concurrent data structure where:

* Exactly **one producer** writes data
* Multiple consumers read data
* Data is stored in a circular buffer

This model is widely used in systems where:

* One source generates data
* Many workers/processes consume it independently

Examples:

* Logging systems (one logger → many readers)
* Publish-subscribe systems
* Event broadcasting
* Networking stacks
* Kernel tracing systems
* Multimedia pipelines

---

# 2. What is SPMC Ring Buffer?

SPMC means:

```text id="spmc2"
Single Producer

Multiple Consumers
```

Example:

```text id="spmc3"
            ┌── Consumer A
Producer ───┼── Consumer B
            └── Consumer C
```

Producer writes once.

Multiple consumers read same data independently.

---

# 3. Why Do We Need SPMC?

Many real systems require **broadcasting data**:

Example:

```text id="spmc4"
Sensor / Event Generator

↓

Multiple processing modules
```

Without SPMC:

* You duplicate data manually
* Waste memory and CPU
* Complex synchronization

---

SPMC solves:

* Efficient data sharing
* Zero-copy broadcasting (logical)
* Parallel consumption

---

# 4. Basic Concept

Structure:

```text id="spmc5"
Single Producer → Ring Buffer → Multiple Consumers
```

Each consumer has:

* Independent read pointer

Producer has:

* Single write pointer

---

# 5. Memory Layout

Example buffer:

```c id="spmc6"
uint8_t buffer[8];
```

Layout:

```text id="spmc7"
+---+---+---+---+---+---+---+---+
| A | B | C | D | E | F | G | H |
+---+---+---+---+---+---+---+---+
```

Each consumer tracks its own head:

```text id="spmc8"
Consumer A head
Consumer B head
Consumer C head
```

Producer tracks:

```text id="spmc9"
tail (write index)
```

---

# 6. Producer-Consumer Model

Producer:

```text id="spmc10"
Writes data once
Advances tail
```

Consumers:

```text id="spmc11"
Each consumer reads independently
Maintains own head pointer
```

Flow:

```text id="spmc12"
Producer → Ring Buffer → Consumer A
                         → Consumer B
                         → Consumer C
```

---

# 7. Concurrency Challenges

SPMC introduces challenges:

### 1. Multiple readers

Each consumer:

* Must not interfere with others

---

### 2. Buffer overwrite risk

Producer may overwrite unread data if:

```text id="spmc13"
slow consumer exists
```

---

### 3. Memory visibility

Consumers must see:

* Correct written data
* Correct tail updates

---

# 8. Head and Tail Design

### Producer:

```text id="spmc14"
Single tail (write index)
```

### Consumers:

```text id="spmc15"
Each consumer has its own head
```

Example:

```text id="spmc16"
Producer tail = 5

Consumer A head = 2
Consumer B head = 3
Consumer C head = 1
```

---

Each consumer progresses independently.

---

# 9. Lock-Based vs Lock-Free SPMC

---

## Lock-Based

```text id="spmc17"
Mutex protects buffer access
```

Pros:

* Simple
* Safe

Cons:

* Slower
* Contention between consumers

---

## Lock-Free

Uses:

```text id="spmc18"
atomic variables
memory barriers
careful ordering
```

Pros:

* High performance
* Low latency

Cons:

* Complex design

---

# 10. Memory Ordering Requirements

Producer must ensure:

```text id="spmc19"
1. Write data
2. Publish tail
```

Consumers must ensure:

```text id="spmc20"
1. Read tail
2. Read data safely
```

---

Use:

* release semantics (producer)
* acquire semantics (consumers)

Example:

```c id="spmc21"
atomic_store_release(&tail, new_tail);
```

```c id="spmc22"
tail = atomic_load_acquire(&tail);
```

---

# 11. Implementation Concept in C

Simplified structure:

```c id="spmc23"
typedef struct {
    uint8_t *buf;
    uint32_t size;
    atomic_uint tail;
    uint32_t *consumer_heads; // array
    uint32_t num_consumers;
} spmc_ring_t;
```

---

Producer:

```c id="spmc24"
int push(spmc_ring_t *r, uint8_t data)
{
    uint32_t next = atomic_fetch_add(&r->tail, 1);

    r->buf[next % r->size] = data;
    return 0;
}
```

---

Consumer:

```c id="spmc25"
int pop(spmc_ring_t *r, int id, uint8_t *data)
{
    uint32_t head = r->consumer_heads[id];

    if (head == atomic_load(&r->tail))
        return -1;

    *data = r->buf[head % r->size];
    r->consumer_heads[id] = head + 1;

    return 0;
}
```

---

# 12. SPMC vs SPSC

| Feature        | SPSC     | SPMC      |
| -------------- | -------- | --------- |
| Producers      | 1        | 1         |
| Consumers      | 1        | Many      |
| Complexity     | Low      | Medium    |
| Memory Sharing | Simple   | Complex   |
| Use Case       | Pipeline | Broadcast |

---

SPSC:

```text id="spmc26"
Pipeline
```

SPMC:

```text id="spmc27"
Fan-out / Broadcast
```

---

# 13. SPMC vs MPSC vs MPMC

| Model | Producers | Consumers | Complexity |
| ----- | --------- | --------- | ---------- |
| SPSC  | 1         | 1         | Low        |
| MPSC  | Many      | 1         | Medium     |
| SPMC  | 1         | Many      | Medium     |
| MPMC  | Many      | Many      | Very High  |

---

SPMC is ideal middle-ground for broadcast systems.

---

# 14. SPMC in Embedded Systems

Used in:

---

## Logging Systems

```text id="spmc28"
One logger → multiple debug viewers
```

---

## Sensor Data Broadcasting

```text id="spmc29"
Sensor → multiple processing tasks
```

---

## RTOS Event System

```text id="spmc30"
One event source → many tasks
```

---

## Audio/Video Systems

```text id="spmc31"
Audio input → multiple DSP pipelines
```

---

# 15. SPMC in Linux Kernel

Used in:

* trace buffers
* event logging
* perf subsystem
* debugging infrastructure

Example:

```text id="spmc32"
Kernel events → multiple user-space consumers
```

---

# 16. Applications

SPMC Ring Buffers are used in:

* Logging systems
* Telemetry systems
* Publish-subscribe frameworks
* Kernel tracing
* Multimedia pipelines
* Event broadcasting systems
* Sensor networks

---

# 17. Advantages

## Broadcast Capability

Single data source → many consumers

---

## Efficient Memory Usage

No duplication of data

---

## Parallel Consumption

Consumers run independently

---

## High Throughput

No repeated writes

---

## Scalable Design

Add more consumers easily

---

# 18. Disadvantages

## Memory Overhead per Consumer

Each consumer needs:

```text id="spmc33"
its own head pointer
```

---

## Slow Consumer Problem

One slow consumer may:

```text id="spmc34"
lag behind producer
```

risking overwrite

---

## Complex Synchronization

Must ensure:

* correct visibility
* safe memory ordering

---

## Hard Debugging

Race conditions can be subtle

---

# 19. Best Practices

✔ Use atomic tail for producer

---

✔ Keep consumer heads independent

---

✔ Use power-of-2 buffer size

---

✔ Apply acquire/release memory ordering

---

✔ Monitor slow consumers

---

✔ Consider overflow protection strategy

---

# 20. Interview Questions

### Q1. What is SPMC Ring Buffer?

A ring buffer with:

```text id="spmc35"
Single Producer

Multiple Consumers
```

---

### Q2. Key challenge?

Managing multiple independent consumer pointers safely.

---

### Q3. How is it different from SPSC?

SPSC has one consumer; SPMC has many.

---

### Q4. Where is SPMC used?

* Logging systems
* Kernel tracing
* Event broadcasting
* Multimedia pipelines

---

### Q5. How do consumers avoid conflicts?

Each consumer maintains its own head pointer.

---

# 21. Real-World Examples

---

## Kernel Tracing

```text id="spmc36"
CPU events → multiple trace readers
```

---

## Logging System

```text id="spmc37"
Application logs → multiple debug tools
```

---

## Multimedia Pipeline

```text id="spmc38"
Camera feed → encoder + preview + analytics
```

---

## Sensor Systems

```text id="spmc39"
Sensor → multiple AI models
```

---

# 22. Summary

SPMC Ring Buffer is:

```text id="spmc40"
Single Producer

Multiple Consumers

Circular Buffer
```

It enables:

✔ Data broadcasting

✔ Parallel consumption

✔ Efficient memory usage

✔ High throughput

Used in:

* Kernel systems
* Logging frameworks
* Multimedia pipelines
* Embedded RTOS systems
* Event-driven architectures

SPMC is a key building block for:

**Broadcast systems + Linux Kernel tracing + Embedded event systems + Multimedia processing pipelines.**
