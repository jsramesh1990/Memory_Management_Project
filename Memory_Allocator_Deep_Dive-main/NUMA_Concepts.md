# NUMA Concepts (Non-Uniform Memory Access)

# Table of Contents

1. Introduction
2. What is NUMA?
3. UMA vs NUMA
4. NUMA Architecture
5. CPU–Memory Topology
6. Local vs Remote Memory Access
7. NUMA Node Concept
8. Memory Latency Differences
9. NUMA in Multi-Core Systems
10. NUMA-Aware Scheduling
11. NUMA Memory Allocation Policies
12. First-Touch Policy
13. Interleaving Policy
14. Migrate Policy
15. NUMA in Linux
16. Tools to Inspect NUMA
17. Performance Implications
18. NUMA Optimization Techniques
19. NUMA in Embedded Systems & Servers
20. Common Pitfalls
21. Advantages & Disadvantages
22. Interview Questions
23. Real-World Applications
24. Summary

---

# 1. Introduction

**NUMA (Non-Uniform Memory Access)** is a computer memory design used in multi-processor systems where memory access time depends on the location of the memory relative to the CPU.

In simple terms:

```text id="numa1"
Some memory is “closer” to a CPU → faster access  
Some memory is “farther” → slower access
```

---

# 2. What is NUMA?

In NUMA systems:

* Each CPU (or CPU group) has its own local memory
* CPUs can also access other CPUs’ memory (remote memory)
* But remote access is slower

---

# 3. UMA vs NUMA

| Feature        | UMA (Uniform Memory Access) | NUMA                   |
| -------------- | --------------------------- | ---------------------- |
| Memory latency | Same for all CPUs           | Depends on location    |
| Scalability    | Limited                     | High                   |
| Architecture   | Simple                      | Complex                |
| Performance    | Flat                        | Optimized per locality |

---

# 4. NUMA Architecture

Typical NUMA system:

```text id="numa2"
CPU0 ---- Memory0 (local)
CPU1 ---- Memory1 (local)

CPU0 ↔ Memory1 (remote, slower)
CPU1 ↔ Memory0 (remote, slower)
```

---

# 5. CPU–Memory Topology

Modern servers group resources:

```text id="numa3"
Node 0: CPU cores + RAM
Node 1: CPU cores + RAM
Node 2: CPU cores + RAM
```

Each group is called a **NUMA node**.

---

# 6. Local vs Remote Memory Access

### Local Access

```text id="numa4"
CPU accesses its own node memory
→ fast (low latency)
```

### Remote Access

```text id="numa5"
CPU accesses another node’s memory
→ slower (higher latency)
```

Difference can be:

* 1.5x to 3x slower depending on system

---

# 7. NUMA Node Concept

A NUMA node contains:

* CPU cores
* Local RAM
* Cache hierarchy

Example:

```text id="numa6"
Node 0 → CPU0-7 + RAM0
Node 1 → CPU8-15 + RAM1
```

---

# 8. Memory Latency Differences

Typical latency hierarchy:

```text id="numa7"
L1 Cache → fastest
L2 Cache
L3 Cache
Local RAM → slower
Remote RAM → slowest
```

---

# 9. NUMA in Multi-Core Systems

As core counts increase:

* shared memory becomes bottleneck
* NUMA reduces contention
* improves scalability

Used in:

* servers
* data centers
* HPC systems

---

# 10. NUMA-Aware Scheduling

Operating systems try to:

✔ schedule threads on CPUs close to their memory
✔ avoid remote memory access

Example:

```text id="numa8"
Thread runs on CPU0 → prefers Node0 memory
```

---

# 11. NUMA Memory Allocation Policies

OS controls how memory is assigned:

---

## Policy 1: Default (local allocation)

Memory allocated from current node.

---

## Policy 2: Bind

Memory restricted to specific NUMA node.

---

## Policy 3: Interleave

Memory distributed across nodes.

---

## Policy 4: Preferred

Try one node first, fallback if needed.

---

# 12. First-Touch Policy

Very important concept:

```text id="numa9"
Memory belongs to the NUMA node of the CPU that first writes to it
```

Example:

* Thread on CPU0 initializes memory → Node0 owns it

---

# 13. Interleaving Policy

Memory is distributed:

```text id="numa10"
Page 1 → Node0  
Page 2 → Node1  
Page 3 → Node0  
```

Used for:

* bandwidth balancing

---

# 14. Migrate Policy

OS can move memory:

* from remote → local node
* to improve performance

---

# 15. NUMA in Linux

Linux supports NUMA via:

* `numactl`
* `libnuma`
* cpuset
* taskset

Example:

```bash
numactl --cpunodebind=0 --membind=0 ./app
```

---

# 16. Tools to Inspect NUMA

Linux tools:

* `numactl --hardware`
* `lscpu`
* `numastat`
* `/proc/self/numa_maps`

---

# 17. Performance Implications

Bad NUMA design causes:

❌ cache misses
❌ high latency
❌ memory bandwidth bottlenecks
❌ thread contention

Good NUMA design gives:

✔ scalability
✔ predictable latency
✔ high throughput

---

# 18. NUMA Optimization Techniques

---

## 1. CPU Affinity

Bind threads to cores:

```text id="numa11"
thread → specific CPU
```

---

## 2. Memory Affinity

Bind memory to node:

```text id="numa12"
local allocation preferred
```

---

## 3. Thread + Memory Co-location

Keep data near computation.

---

## 4. Partitioning Workloads

Split application per NUMA node:

```text id="numa13"
Node0 handles users A–M  
Node1 handles users N–Z
```

---

## 5. Avoid Shared Hotspots

Reduce cross-node contention.

---

# 19. NUMA in Embedded Systems & Servers

### Servers:

* databases (MySQL, PostgreSQL)
* cloud systems
* HPC clusters
* AI training systems

### Embedded (high-end SoCs):

* multi-cluster ARM systems
* heterogeneous compute systems

---

# 20. Common Pitfalls

❌ Ignoring NUMA layout
❌ Global shared queues
❌ Remote memory-heavy threads
❌ Poor thread scheduling
❌ False sharing across nodes

---

# 21. Advantages & Disadvantages

## Advantages

✔ scalable memory architecture
✔ higher throughput
✔ reduced contention
✔ better parallel performance

---

## Disadvantages

❌ complexity
❌ tuning required
❌ performance variability
❌ debugging difficulty

---

# 22. Interview Questions

### Q1. What is NUMA?

Memory architecture where access time depends on memory location relative to CPU.

---

### Q2. Difference between UMA and NUMA?

UMA = uniform latency
NUMA = non-uniform latency

---

### Q3. What is first-touch policy?

Memory belongs to the node of first writing CPU.

---

### Q4. Why NUMA improves performance?

Because it reduces shared memory bottlenecks.

---

### Q5. What is remote memory access?

Accessing memory from another NUMA node (slower).

---

# 23. Real-World Applications

* cloud servers
* database engines
* distributed systems
* AI/ML training clusters
* high-performance networking
* operating system kernels

---

# 24. Summary

NUMA is a memory architecture where:

```text id="numa14"
Memory access time depends on CPU–memory distance
```

Key ideas:

✔ NUMA nodes
✔ local vs remote memory
✔ memory affinity
✔ CPU scheduling awareness

It is critical in:

**modern multi-core servers, high-performance computing, and scalable system design**
