# Custom Memory Allocators

# Table of Contents

1. Introduction
2. What is a Memory Allocator?
3. Why Custom Allocators are Needed
4. Types of Memory Allocation
5. Heap vs Stack vs Custom Allocators
6. Common Problems in Standard Allocators
7. What is a Custom Memory Allocator?
8. Core Design Goals
9. Types of Custom Allocators

   * 9.1 Linear Allocator
   * 9.2 Pool Allocator
   * 9.3 Slab Allocator
   * 9.4 Stack Allocator
   * 9.5 Buddy Allocator
   * 9.6 Free-List Allocator
10. Fragmentation (Internal & External)
11. Alignment and Cache Considerations
12. Allocation Strategy Design
13. Implementation Concepts in C
14. Custom Allocators in Embedded Systems
15. Custom Allocators in Linux Kernel
16. Performance Comparison
17. Advantages
18. Disadvantages
19. Best Practices
20. Interview Questions
21. Real-World Applications
22. Summary

---

# 1. Introduction

A **Custom Memory Allocator** is a memory management system designed specifically for an application, OS, or embedded system instead of using general-purpose allocators like:

```text id="cma1"
malloc / free (libc heap)
```

Custom allocators are designed to:

```text id="cma2"
improve performance + reduce fragmentation + control memory usage
```

---

# 2. What is a Memory Allocator?

A memory allocator manages:

* allocation of memory blocks
* deallocation of memory blocks
* reuse of freed memory

Basic flow:

```text id="cma3"
Application → Allocator → Heap Memory
```

---

# 3. Why Custom Allocators are Needed

Standard allocators have issues:

* unpredictable latency
* fragmentation
* poor cache locality
* thread contention
* general-purpose overhead

Custom allocators solve:

✔ deterministic performance
✔ faster allocation
✔ better cache usage
✔ domain-specific optimization

---

# 4. Types of Memory Allocation

```text id="cma4"
Static Allocation
Stack Allocation
Heap Allocation
Custom Allocation
```

---

# 5. Heap vs Stack vs Custom Allocators

| Type             | Speed   | Flexibility | Fragmentation |
| ---------------- | ------- | ----------- | ------------- |
| Stack            | Fastest | Low         | None          |
| Heap (malloc)    | Medium  | High        | High          |
| Custom Allocator | Fast    | Controlled  | Low           |

---

# 6. Common Problems in Standard Allocators

❌ Fragmentation
❌ Unpredictable latency
❌ Lock contention
❌ Cache inefficiency
❌ General-purpose overhead

---

# 7. What is a Custom Memory Allocator?

A system where memory is pre-managed using a specific strategy:

```text id="cma5"
Pre-allocated memory pool + custom allocation logic
```

Instead of:

```text id="cma6"
malloc/free per request
```

we use:

```text id="cma7"
fast structured allocation system
```

---

# 8. Core Design Goals

✔ Fast allocation
✔ Fast deallocation
✔ Deterministic behavior
✔ Low fragmentation
✔ Cache efficiency
✔ Thread safety (if required)

---

# 9. Types of Custom Allocators

---

## 9.1 Linear Allocator

Allocates memory sequentially.

```text id="cma8"
[##########..........]
```

Allocation:

```text id="cma9"
ptr = base + offset;
offset += size;
```

Freeing:

```text id="cma10"
only reset all at once
```

✔ Extremely fast
❌ No individual free

---

## 9.2 Pool Allocator

Fixed-size blocks.

```text id="cma11"
[ ][ ][ ][ ][ ]
```

Used for:

* objects of same size

✔ No fragmentation
✔ O(1) allocation

---

## 9.3 Slab Allocator

Advanced pool allocator used in OS kernels.

```text id="cma12"
Objects cached in slabs
```

Used in:

* Linux kernel
* object caching

---

## 9.4 Stack Allocator

LIFO allocation:

```text id="cma13"
push → allocate
pop → free
```

✔ Very fast
✔ Cache friendly

---

## 9.5 Buddy Allocator

Splits memory into power-of-two blocks:

```text id="cma14"
1024 → 512 + 512
```

Used in:

* OS memory systems

---

## 9.6 Free-List Allocator

Tracks free blocks:

```text id="cma15"
linked list of free memory
```

Allocation:

* find block
* remove from list

---

# 10. Fragmentation

---

## Internal Fragmentation

Wasted space inside allocated block.

---

## External Fragmentation

Free memory exists but not contiguous.

---

Custom allocators reduce fragmentation by:

✔ fixed block sizes
✔ pooling
✔ controlled reuse

---

# 11. Alignment and Cache Considerations

Important for performance:

```text id="cma16"
4-byte alignment
8-byte alignment
64-byte cache line alignment
```

Bad alignment causes:

* cache misses
* slow access

---

# 12. Allocation Strategy Design

Key decisions:

* block size
* pool count
* reuse strategy
* concurrency model

---

Example design:

```text id="cma17"
1 KB blocks × 1024 entries pool
```

---

# 13. Implementation Concepts in C

---

## Pool Allocator Example

```c id="cma18"
#define POOL_SIZE 1024

typedef struct {
    uint8_t pool[POOL_SIZE][64];
    uint32_t free_list[POOL_SIZE];
    uint32_t top;
} pool_t;
```

---

Allocate:

```c id="cma19"
void* alloc(pool_t *p)
{
    if (p->top == 0)
        return NULL;

    return p->pool[p->free_list[--p->top]];
}
```

---

Free:

```c id="cma20"
void free_block(pool_t *p, int idx)
{
    p->free_list[p->top++] = idx;
}
```

---

# 14. Custom Allocators in Embedded Systems

Used in:

* RTOS tasks
* DMA buffers
* UART buffers
* sensor pipelines

Why:

✔ deterministic timing
✔ no fragmentation
✔ low memory footprint

---

Example:

```text id="cma21"
Fixed buffer pool for UART RX
```

---

# 15. Custom Allocators in Linux Kernel

Linux uses:

* slab allocator
* kmalloc
* vmalloc
* page allocator

Example:

```text id="cma22"
slab caches for objects
```

Benefits:

* fast allocation
* reduced fragmentation
* CPU cache efficiency

---

# 16. Performance Comparison

| Allocator | Speed     | Fragmentation | Use Case      |
| --------- | --------- | ------------- | ------------- |
| malloc    | Medium    | High          | General       |
| Linear    | Very Fast | None          | Temporary     |
| Pool      | Very Fast | None          | Fixed objects |
| Slab      | Fast      | Low           | OS kernel     |
| Buddy     | Medium    | Medium        | OS memory     |

---

# 17. Advantages

✔ High performance
✔ Predictable latency
✔ Reduced fragmentation
✔ Better cache locality
✔ Domain-specific optimization

---

# 18. Disadvantages

❌ Complex design
❌ Memory pre-allocation required
❌ Less flexible than malloc
❌ Risk of memory wastage

---

# 19. Best Practices

✔ Use pool allocator for fixed-size objects
✔ Use slab for kernel-like workloads
✔ Align memory to cache lines
✔ Avoid dynamic resizing
✔ Pre-allocate memory in real-time systems

---

# 20. Interview Questions

### Q1. What is a custom memory allocator?

A memory system tailored for specific application requirements.

---

### Q2. Why not use malloc?

Because malloc has:

* fragmentation
* unpredictable latency
* overhead

---

### Q3. Best allocator for fixed-size objects?

Pool allocator.

---

### Q4. What is slab allocator?

A cached object allocator used in operating systems.

---

### Q5. Why alignment matters?

To avoid cache inefficiency and memory penalties.

---

# 21. Real-World Applications

* Embedded firmware
* Game engines
* Linux kernel
* Networking stacks
* Real-time systems
* Database engines
* High-performance servers

---

# 22. Summary

Custom memory allocators are specialized systems that replace general-purpose heap allocation with optimized strategies like:

```text id="cma23"
Pool Allocator
Slab Allocator
Linear Allocator
Buddy Allocator
Free List Allocator
```

They provide:

✔ deterministic performance
✔ reduced fragmentation
✔ better cache usage
✔ high efficiency in embedded and kernel systems

They are a foundational concept in:

**Operating Systems + Embedded Systems + High-Performance Computing + Real-Time Applications**
