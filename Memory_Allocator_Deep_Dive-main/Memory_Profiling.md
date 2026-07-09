# Memory Profiling

# Table of Contents

1. Introduction
2. What is Memory Profiling?
3. Why Memory Profiling is Important
4. Memory Lifecycle in a Program
5. Types of Memory Problems

   * 5.1 Memory Leak
   * 5.2 Heap Fragmentation
   * 5.3 Buffer Overflow
   * 5.4 Use-after-Free
   * 5.5 Double Free
   * 5.6 Memory Hoarding
6. Memory Usage Metrics
7. Heap vs Stack Profiling
8. Tools for Memory Profiling
9. Profiling Techniques
10. Memory Profiling in C/C++
11. Memory Profiling in Embedded Systems
12. Memory Profiling in Linux
13. Runtime vs Static Analysis
14. Sampling vs Instrumentation
15. Leak Detection Strategies
16. Performance Bottlenecks in Memory
17. Custom Allocator Profiling
18. Real-Time System Considerations
19. Best Practices
20. Interview Questions
21. Real-World Applications
22. Summary

---

# 1. Introduction

**Memory Profiling** is the process of analyzing how a program uses memory during execution.

It answers questions like:

* How much memory is used?
* Where is memory allocated?
* Which components leak memory?
* Which functions allocate the most memory?
* Is memory usage stable or growing?

---

# 2. What is Memory Profiling?

Memory profiling tracks:

```text id="mp1"
Allocation → Usage → Deallocation → Lifetime
```

It helps identify inefficiencies and bugs in memory usage.

---

# 3. Why Memory Profiling is Important

Without profiling:

* memory leaks go unnoticed
* systems crash under load
* embedded devices run out of RAM
* servers degrade over time

---

With profiling:

✔ stable systems
✔ optimized memory usage
✔ predictable performance

---

# 4. Memory Lifecycle in a Program

```text id="mp2"
malloc → use → free
```

or:

```text id="mp3"
stack frame → execution → return
```

Profiling tracks every stage.

---

# 5. Types of Memory Problems

---

## 5.1 Memory Leak

Memory is allocated but never freed.

```text id="mp4"
malloc → no free → memory grows
```

---

## 5.2 Heap Fragmentation

Free memory becomes scattered.

```text id="mp5"
[used][free][used][free][small gaps]
```

---

## 5.3 Buffer Overflow

Writing beyond allocated memory.

```text id="mp6"
buffer[10] → writing buffer[11] ❌
```

---

## 5.4 Use-after-Free

Accessing freed memory.

```text id="mp7"
free(ptr)
use(ptr) ❌
```

---

## 5.5 Double Free

Freeing memory twice.

```text id="mp8"
free(ptr)
free(ptr) ❌
```

---

## 5.6 Memory Hoarding

Memory retained unnecessarily.

---

# 6. Memory Usage Metrics

Key metrics:

* Total heap usage
* Peak memory usage
* Allocation count
* Allocation rate
* Fragmentation level
* Lifetime distribution

---

# 7. Heap vs Stack Profiling

| Feature  | Heap           | Stack          |
| -------- | -------------- | -------------- |
| Control  | Manual         | Automatic      |
| Lifetime | Dynamic        | Function scope |
| Errors   | Leaks possible | Rare           |
| Speed    | Slow           | Fast           |

---

# 8. Tools for Memory Profiling

Common tools:

* Valgrind
* AddressSanitizer (ASan)
* LeakSanitizer (LSan)
* gperftools
* heaptrack
* perf
* Visual Studio Profiler

---

Embedded tools:

* Segger SystemView
* FreeRTOS trace tools
* custom logging allocators

---

# 9. Profiling Techniques

---

## 1. Instrumentation

Modify code:

```c id="mp9"
malloc → tracked_malloc()
```

---

## 2. Sampling

Check memory periodically.

---

## 3. Hardware Counters

Use CPU performance counters.

---

# 10. Memory Profiling in C/C++

Typical approach:

```c id="mp10"
override malloc/free
```

Example:

```c id="mp11"
void* tracked_malloc(size_t size)
{
    void* p = malloc(size);
    log_alloc(p, size);
    return p;
}
```

---

# 11. Memory Profiling in Embedded Systems

Embedded systems face constraints:

* limited RAM
* real-time deadlines
* no OS support sometimes

Approaches:

✔ static analysis
✔ custom allocators
✔ logging wrappers
✔ buffer usage tracking

---

Example:

```text id="mp12"
UART logs memory allocation events
```

---

# 12. Memory Profiling in Linux

Linux tools:

* /proc/meminfo
* /proc/[pid]/status
* smaps
* perf
* ps/top/htop

Kernel-level:

* kmemleak
* slab debugging

---

# 13. Runtime vs Static Analysis

| Type    | When         | Accuracy |
| ------- | ------------ | -------- |
| Static  | Compile time | Partial  |
| Runtime | Execution    | Accurate |

---

# 14. Sampling vs Instrumentation

---

## Sampling

* Low overhead
* Less accurate

---

## Instrumentation

* High overhead
* Very accurate

---

# 15. Leak Detection Strategies

Common methods:

* track allocation table
* reference counting
* garbage collection (in managed systems)
* heap snapshot comparison

---

# 16. Performance Bottlenecks in Memory

Memory issues affecting performance:

* frequent allocations
* cache misses
* fragmentation
* lock contention in heap
* NUMA locality issues

---

# 17. Custom Allocator Profiling

Custom allocators often include:

```text id="mp13"
allocation counters
free counters
pool usage stats
fragmentation metrics
```

Example:

```c id="mp14"
printf("Allocations: %d", alloc_count);
```

---

# 18. Real-Time System Considerations

RT systems require:

✔ deterministic allocation time
✔ no dynamic heap unpredictability
✔ pre-allocated pools

Memory profiling ensures:

* no latency spikes
* no leaks over long uptime
* stable memory usage

---

# 19. Best Practices

✔ always track allocations in debug builds
✔ use address sanitizers during testing
✔ prefer pool allocators in embedded systems
✔ avoid frequent malloc/free in hot paths
✔ monitor peak memory usage
✔ validate buffer boundaries

---

# 20. Interview Questions

### Q1. What is memory profiling?

Analyzing memory usage behavior of a program during execution.

---

### Q2. Difference between leak and fragmentation?

Leak = memory not freed
Fragmentation = memory unusable due to gaps

---

### Q3. Best tool for leak detection?

Valgrind or AddressSanitizer.

---

### Q4. Why is memory profiling important in embedded systems?

Because RAM is limited and leaks cause system failure.

---

### Q5. Instrumentation vs sampling?

Instrumentation = accurate but slow
Sampling = fast but approximate

---

# 21. Real-World Applications

* Operating systems
* Embedded firmware
* Game engines
* Web servers
* Database engines
* Networking stacks
* High-frequency trading systems

---

# 22. Summary

Memory profiling is the process of tracking and analyzing memory usage in a program.

It helps detect:

✔ leaks
✔ fragmentation
✔ overuse
✔ unsafe memory access

It is essential in:

**Embedded systems + Linux kernel + high-performance computing + real-time systems + large-scale server applications**
