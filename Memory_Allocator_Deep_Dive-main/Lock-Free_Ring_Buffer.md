# Lock-Free Ring Buffer

# Table of Contents

1. Introduction
2. What is a Lock-Free Ring Buffer?
3. Why “Lock-Free” Matters
4. Core Idea
5. Circular Buffer Recap
6. Atomic Operations Foundation
7. Memory Ordering (Critical Concept)
8. Single Producer Single Consumer (SPSC Lock-Free)
9. Multi-Producer Variants
10. Multi-Consumer Variants
11. MPMC Lock-Free Ring Buffer (Advanced)
12. Sequence Number Algorithm (Vyukov MPMC)
13. Implementation Concept in C
14. Common Pitfalls
15. Performance Characteristics
16. Cache Effects (False Sharing)
17. Use in Embedded Systems
18. Use in Linux Kernel
19. Applications
20. Advantages
21. Disadvantages
22. Best Practices
23. Interview Questions
24. Summary

---

# 1. Introduction

A **Lock-Free Ring Buffer** is a circular buffer that allows concurrent access without using traditional locks like:

```text id="lf1"
mutex
semaphore
spinlock
```

Instead, it uses:

```text id="lf2"
atomic operations + memory ordering
```

Goal:

```text id="lf3"
High performance + No blocking + Predictable latency
```

---

# 2. What is a Lock-Free Ring Buffer?

A lock-free ring buffer ensures:

* No thread is permanently blocked
* At least one thread always makes progress
* No critical sections protected by mutex

Structure:

```text id="lf4"
Producer(s) → Ring Buffer → Consumer(s)
```

---

# 3. Why “Lock-Free” Matters

Locks cause:

```text id="lf5"
context switching
priority inversion
deadlocks
contention
```

Lock-free avoids:

✔ Blocking
✔ Sleep/wakeup overhead
✔ Kernel scheduling delays

---

Used in:

* real-time systems
* high-frequency trading
* networking stacks
* embedded systems
* kernel subsystems

---

# 4. Core Idea

Instead of locking:

```text id="lf6"
mutual exclusion → atomic coordination
```

Each operation is split into:

* claim slot
* write/read
* publish update

---

# 5. Circular Buffer Recap

```text id="lf7"
+----+----+----+----+
| A  | B  | C  | D  |
+----+----+----+----+
 head         tail
```

Wrap-around:

```text id="lf8"
index = (index + 1) % size
```

---

# 6. Atomic Operations Foundation

Lock-free systems rely on:

```text id="lf9"
atomic_fetch_add()
compare_and_swap (CAS)
atomic_load()
atomic_store()
```

Example:

```c id="lf10"
atomic_uint tail;
```

---

# 7. Memory Ordering (Critical Concept)

CPU reordering can break correctness.

We use:

* acquire
* release
* fence

Producer:

```text id="lf11"
write data → release publish
```

Consumer:

```text id="lf12"
acquire read → read data
```

---

# 8. SPSC Lock-Free Ring Buffer

Simplest lock-free case:

```text id="lf13"
1 Producer + 1 Consumer
```

No locks needed because:

* no shared writer contention
* no shared reader contention

---

Producer:

```c id="lf14"
buf[tail] = data;
atomic_store_release(&tail, tail+1);
```

Consumer:

```c id="lf15"
tail = atomic_load_acquire(&tail);
data = buf[head];
head++;
```

---

# 9. Multi-Producer Variants

Need atomic reservation:

```text id="lf16"
atomic_fetch_add(&tail, 1)
```

Each producer gets unique index.

---

# 10. Multi-Consumer Variants

Each consumer must:

* avoid double-read
* coordinate head movement

Often requires:

```text id="lf17"
atomic compare-and-swap
```

---

# 11. MPMC Lock-Free Ring Buffer (Advanced)

Hardest case:

```text id="lf18"
Multiple Producers + Multiple Consumers
```

Uses:

✔ sequence numbers per slot
✔ CAS loops
✔ memory barriers

---

# 12. Sequence Number Algorithm (Vyukov Queue)

Each slot:

```c id="lf19"
typedef struct {
    atomic_uint seq;
    uint8_t data;
} slot_t;
```

---

Producer logic:

```text id="lf20"
1. read seq
2. check slot free
3. CAS write
```

Consumer logic:

```text id="lf21"
1. read seq
2. check ready
3. consume
```

---

This avoids global locks entirely.

---

# 13. Implementation Concept in C

Simplified SPSC version:

```c id="lf22"
typedef struct {
    uint8_t *buf;
    uint32_t size;
    atomic_uint head;
    atomic_uint tail;
} ring_t;
```

---

Push:

```c id="lf23"
void push(ring_t *r, uint8_t v)
{
    uint32_t t = atomic_load(&r->tail);

    r->buf[t % r->size] = v;

    atomic_store_release(&r->tail, t + 1);
}
```

---

Pop:

```c id="lf24"
int pop(ring_t *r, uint8_t *v)
{
    uint32_t h = atomic_load_acquire(&r->head);
    uint32_t t = atomic_load(&r->tail);

    if (h == t) return -1;

    *v = r->buf[h % r->size];

    atomic_store(&r->head, h + 1);

    return 0;
}
```

---

# 14. Common Pitfalls

❌ Assuming atomic = thread-safe logic
❌ Ignoring memory ordering
❌ False sharing on cache lines
❌ Overwriting unread data
❌ Wrap-around bugs

---

# 15. Performance Characteristics

Lock-free:

```text id="lf25"
very fast under low contention
```

But:

```text id="lf26"
degrades under extreme contention
```

---

# 16. Cache Effects (False Sharing)

Problem:

```text id="lf27"
producer and consumer modify same cache line
```

Solution:

```text id="lf28"
cache line padding (64 bytes)
```

---

# 17. Use in Embedded Systems

Used in:

* UART DMA buffers
* audio pipelines
* sensor fusion
* ISR → task communication

Example:

```text id="lf29"
ISR → lock-free ring → main loop
```

---

# 18. Use in Linux Kernel

Used in:

* networking (skbuff paths)
* tracing subsystem
* perf events
* log buffers

---

Example:

```text id="lf30"
CPU cores → ring buffer → tracing tools
```

---

# 19. Applications

* High-speed networking
* Real-time audio
* Video streaming
* Kernel logging
* Message queues
* DMA pipelines
* Trading systems

---

# 20. Advantages

✔ No locks
✔ High throughput
✔ Low latency
✔ Deterministic timing
✔ Good cache performance

---

# 21. Disadvantages

❌ Complex design
❌ Hard debugging
❌ Memory ordering sensitive
❌ ABA problems (MPMC)
❌ Hard to implement correctly

---

# 22. Best Practices

✔ Prefer SPSC when possible
✔ Use power-of-two buffer sizes
✔ Use acquire/release semantics
✔ Avoid false sharing
✔ Batch operations for performance
✔ Use proven algorithms (Vyukov queue)

---

# 23. Interview Questions

### Q1. What is lock-free ring buffer?

A concurrent circular buffer using atomic operations instead of locks.

---

### Q2. Is lock-free always wait-free?

No. Lock-free guarantees progress, not per-thread fairness.

---

### Q3. Why use memory barriers?

To prevent CPU reordering issues.

---

### Q4. Best case use?

SPSC (fastest and simplest lock-free design).

---

### Q5. Hardest case?

MPMC lock-free ring buffer.

---

# 24. Summary

A **Lock-Free Ring Buffer** is:

```text id="lf31"
Circular Buffer + Atomic Operations + Memory Ordering
```

It enables:

✔ High-performance concurrency
✔ No mutex overhead
✔ Low latency communication

Used in:

* kernels
* embedded systems
* networking stacks
* real-time systems
* high-performance computing

It is one of the most important building blocks in modern concurrent system design.
