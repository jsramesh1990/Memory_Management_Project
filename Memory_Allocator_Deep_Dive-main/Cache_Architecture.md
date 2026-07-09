# Cache Architecture

## Table of Contents

1. Introduction
2. Definition
3. Why Cache is Needed
4. Memory Hierarchy
5. Cache Terminology
6. Cache Organization
7. Types of Cache
8. Cache Mapping Techniques
9. Cache Replacement Policies
10. Write Policies
11. Cache Coherency
12. Multi-Level Cache (L1, L2, L3)
13. Cache Miss Types
14. Cache Performance Metrics
15. Cache in Embedded Systems
16. Real-Time Examples
17. Advantages
18. Disadvantages
19. Major Project Usage
20. Best Practices
21. Interview Questions
22. Conclusion

---

# 1. Introduction

Suppose a CPU runs at:

```text
2 GHz
```

which means:

```text
1 Cycle

=

0.5 ns
```

---

RAM access:

```text
50 ~ 100 ns
```

---

Without cache:

```text
CPU

↓

Read Variable

↓

RAM

↓

Wait 100 ns

↓

Continue
```

Most CPU cycles are wasted.

---

Solution:

```text
CPU

↓

Cache

↓

RAM
```

Cache stores:

```text
Frequently Used Data

and

Instructions
```

close to the CPU.

---

# 2. Definition

## Definition

**Cache** is a small, high-speed memory located between the CPU and main memory that stores frequently accessed instructions and data to reduce memory access time.

---

## Simple Definition

```text
CPU

↓

Cache

↓

RAM
```

Cache acts as:

```text
Fast Temporary Storage
```

---

# 3. Why Cache is Needed

CPU speed:

```text
GHz
```

Memory speed:

```text
MHz
```

---

Example:

| Component | Access Time |
| --------- | ----------: |
| Register  |      0.2 ns |
| L1 Cache  |        1 ns |
| L2 Cache  |        3 ns |
| L3 Cache  |       10 ns |
| RAM       |   50-100 ns |
| SSD       |       50 µs |
| HDD       |        5 ms |

---

Without cache:

```text
CPU

↓

RAM

↓

Huge Delay
```

---

With cache:

```text
CPU

↓

L1

↓

L2

↓

L3

↓

RAM
```

Most accesses:

```text
Served Quickly
```

---

# 4. Memory Hierarchy

```text
                Fast
                 ↑

Registers

↓

L1 Cache

↓

L2 Cache

↓

L3 Cache

↓

RAM

↓

SSD

↓

HDD

                 ↓

               Slow
```

---

Properties:

| Memory    |     Speed |       Size |    Cost |
| --------- | --------: | ---------: | ------: |
| Registers |   Fastest | Very Small | Highest |
| L1        | Very Fast |      Small |    High |
| L2        |      Fast |     Medium |  Medium |
| L3        |  Moderate |      Large |   Lower |
| RAM       |      Slow |       Huge |   Cheap |

---

# 5. Cache Terminology

---

## Cache Line

Smallest unit stored in cache.

Example:

```text
Cache Line

64 Bytes
```

---

If CPU reads:

```c
int arr[100];
```

Reading:

```text
arr[0]
```

loads:

```text
arr[0]

arr[1]

arr[2]

...

64 Bytes
```

---

## Hit

Data found in cache.

```text
CPU

↓

L1

↓

Found

↓

Cache Hit
```

---

## Miss

Data not found.

```text
CPU

↓

L1

↓

Not Found

↓

RAM

↓

Cache Miss
```

---

# 6. Cache Organization

Cache stores:

```text
Tag

+

Index

+

Data

+

Valid Bit
```

---

Example:

```text
Memory Address


TAG

INDEX

OFFSET
```

---

Cache entry:

```text
TAG

Data Block

Valid Bit
```

---

CPU:

```text
Address

↓

Index

↓

Find Cache Line

↓

Compare TAG

↓

Hit/Miss
```

---

# 7. Types of Cache

---

## Instruction Cache (I-Cache)

Stores:

```text
Instructions

Machine Code
```

---

Example:

```c
for(i=0;i<100;i++)
{
}
```

Loop instructions stored in:

```text
I-Cache
```

---

## Data Cache (D-Cache)

Stores:

```text
Variables

Arrays

Structures
```

---

Example:

```c
int x=10;
```

Variable:

```text
x
```

stored in:

```text
D-Cache
```

---

## Unified Cache

Stores:

```text
Instructions

+

Data
```

in same cache.

---

# 8. Cache Mapping Techniques

Three major types:

---

## Direct Mapping

Each memory block:

```text
Maps To

Exactly One

Cache Location
```

---

Example:

```text
Memory Block 0

↓

Cache Line 0


Memory Block 4

↓

Cache Line 0
```

---

Advantages:

✅ Simple

✅ Fast

---

Disadvantages:

❌ Conflict misses

---

## Fully Associative

Memory:

```text
Can Be Stored

Anywhere
```

---

Example:

```text
Memory Block 7

↓

Any Cache Line
```

---

Advantages:

✅ Few misses

---

Disadvantages:

❌ Expensive

❌ Complex

---

## Set Associative

Most common.

Example:

```text
4-way Set Associative
```

---

Memory:

```text
One Set

↓

Any Of

4 Lines
```

---

Advantages:

✅ Good hit rate

✅ Moderate hardware

---

# 9. Cache Replacement Policies

If cache full:

```text
Replace Old Entry
```

---

## FIFO

```text
First In

↓

First Out
```

---

## LRU

```text
Least Recently Used

↓

Replace
```

---

Most common.

---

## Random

Random replacement.

---

Simple:

```text
Low Hardware Cost
```

---

# 10. Write Policies

When CPU writes:

```c
x=10;
```

What happens?

---

## Write Through

```text
CPU

↓

Cache

↓

RAM
```

Both updated immediately.

---

Advantages:

✅ RAM always updated

---

Disadvantages:

❌ More RAM traffic

---

## Write Back

```text
CPU

↓

Cache

↓

Dirty Bit Set

↓

RAM Updated Later
```

---

Advantages:

✅ Faster

✅ Less RAM access

---

Disadvantages:

❌ More complex

---

# 11. Cache Coherency

In multicore CPUs:

```text
Core 1

↓

Cache 1


Core 2

↓

Cache 2
```

---

Problem:

```text
Core1

x=100
```

---

Core2:

```text
Still Reads

Old x
```

---

Need:

```text
Cache Coherency Protocol
```

---

Popular:

### MESI

States:

```text
Modified

Exclusive

Shared

Invalid
```

---

Maintains:

```text
Same Data

Across Cores
```

---

# 12. Multi-Level Cache

Modern CPUs:

---

## L1 Cache

```text
Closest To CPU

32KB ~ 128KB
```

---

Speed:

```text
1~2 Cycles
```

---

## L2 Cache

```text
256KB

512KB

1MB
```

---

Speed:

```text
5~15 Cycles
```

---

## L3 Cache

```text
4 MB

8 MB

16 MB

32 MB
```

---

Shared among cores.

---

Architecture:

```text
CPU Core

↓

L1

↓

L2

↓

L3

↓

RAM
```

---

# 13. Cache Miss Types

---

## Compulsory Miss

First access:

```text
Never Loaded Before
```

---

Example:

```c
arr[0];
```

First read:

```text
Miss
```

---

## Capacity Miss

Cache too small.

---

Example:

```text
1 MB Data

↓

32 KB Cache
```

---

Some data evicted.

---

## Conflict Miss

Occurs in:

```text
Direct Mapping
```

---

Two blocks:

```text
Map To

Same Cache Line
```

---

One replaces another.

---

# 14. Cache Performance Metrics

---

## Hit Rate

Formula:

```text
Hit Rate

=

Hits

/

Total Accesses
```

---

Example:

```text
900 Hits

1000 Accesses
```

---

Result:

```text
90%
```

---

## Miss Rate

```text
Miss Rate

=

Misses

/

Total Accesses
```

---

Example:

```text
100 Misses

1000 Accesses
```

---

Result:

```text
10%
```

---

## Average Memory Access Time

Formula:

```text
AMAT

=

Hit Time

+

Miss Rate × Miss Penalty
```

---

Used extensively in:

```text
CPU Design

Performance Analysis
```

---

# 15. Cache in Embedded Systems

Common in:

---

### ARM Cortex-A

```text
L1

L2

L3
```

---

### Raspberry Pi

```text
Instruction Cache

Data Cache

L2 Cache
```

---

### STM32H7

```text
I-Cache

D-Cache
```

---

### ESP32

```text
Instruction Cache

Data Cache
```

---

# Bare Metal MCUs

Often:

```text
No Cache
```

Examples:

```text
STM32F0

AVR

PIC16

8051
```

---

# 16. Real-Time Examples

---

## Example 1

Array Traversal:

```c
int arr[1000];

for(i=0;i<1000;i++)
{
    sum+=arr[i];
}
```

---

Because:

```text
arr[i]

arr[i+1]

arr[i+2]
```

are close,

cache:

```text
Loads Whole Block
```

---

Performance:

```text
Very Fast
```

---

## Example 2

Random Access

```c
arr[rand()%1000];
```

---

Accesses:

```text
Random Locations
```

---

Cache:

```text
Poor Hit Rate
```

---

Performance:

```text
Slower
```

---

# 17. Advantages

✅ Faster memory access

✅ Improved CPU performance

✅ Reduced RAM access

✅ Lower average latency

✅ Better instruction throughput

---

# 18. Disadvantages

❌ Expensive SRAM

❌ Complex hardware

❌ Cache coherency issues

❌ Unpredictable latency

❌ Power consumption

---

# 19. Major Project Usage

Cache architecture is critical in:

---

### ARM Cortex-A

```text
L1

L2

L3
```

---

### Intel CPUs

```text
L1

L2

L3
```

---

### AMD Ryzen

```text
Multi-Level Cache
```

---

### Raspberry Pi

```text
Instruction Cache

Data Cache
```

---

### STM32H7

```text
I-Cache

D-Cache
```

---

### Linux Kernel

Optimizes:

```text
Cache Locality

Memory Access

Scheduling
```

---

# 20. Best Practices

---

## Access Arrays Sequentially

Good:

```c
for(i=0;i<n;i++)
{
    sum+=arr[i];
}
```

---

Avoid:

```c
arr[random()];
```

---

## Use Cache-Friendly Data Structures

Prefer:

```text
Arrays
```

over:

```text
Linked Lists
```

for performance.

---

## Keep Working Set Small

Keep:

```text
Frequently Used Data

Inside Cache
```

---

## Minimize False Sharing

In multicore systems:

```text
Separate Frequently Updated Variables
```

---

# 21. Interview Questions

### Q1. What is Cache?

Small high-speed memory between CPU and RAM.

---

### Q2. Why is Cache used?

To reduce:

```text
Memory Access Time
```

and improve:

```text
CPU Performance
```

---

### Q3. Types of Cache?

```text
Instruction Cache

Data Cache

Unified Cache
```

---

### Q4. Levels of Cache?

```text
L1

L2

L3
```

---

### Q5. Difference between Write Through and Write Back?

| Write Through          | Write Back      |
| ---------------------- | --------------- |
| Update RAM Immediately | Update Later    |
| Simpler                | Faster          |
| More RAM Access        | Less RAM Access |

---

### Q6. What is Cache Hit?

Data found in cache.

---

### Q7. What is Cache Miss?

Data not found in cache and fetched from RAM.

---

### Q8. What is AMAT?

```text
AMAT

=

Hit Time

+

Miss Rate × Miss Penalty
```

---

# 22. Conclusion

**Cache Architecture** is designed to bridge the speed gap between the CPU and RAM by storing frequently used instructions and data in fast memory close to the processor.

---

## Complete Cache Hierarchy

```text
CPU Registers

↓

L1 Cache

↓

L2 Cache

↓

L3 Cache

↓

RAM

↓

SSD

↓

HDD
```

---

## Cache Summary

| Feature            | Description                          |
| ------------------ | ------------------------------------ |
| Purpose            | Reduce Memory Latency                |
| Fastest Cache      | L1                                   |
| Largest Cache      | L3                                   |
| Data Unit          | Cache Line                           |
| Mapping Types      | Direct, Associative, Set Associative |
| Write Policies     | Write Through, Write Back            |
| Common Coherency   | MESI                                 |
| Performance Metric | Hit Rate                             |

---

## Quick Interview Cheat Sheet

| Topic              | Answer                              |
| ------------------ | ----------------------------------- |
| Cache Purpose      | Reduce RAM Access                   |
| L1 Size            | 32KB - 128KB                        |
| L2 Size            | 256KB - 1MB                         |
| L3 Size            | 4MB - 32MB                          |
| Fastest Cache      | L1                                  |
| Write Back         | Update RAM Later                    |
| Write Through      | Update RAM Immediately              |
| Replacement Policy | LRU                                 |
| Coherency Protocol | MESI                                |
| AMAT               | Hit Time + Miss Rate × Miss Penalty |

---

## Learning Path

```text
Computer Architecture
        ↓
CPU Architecture
        ↓
Memory Hierarchy
        ↓
Cache Architecture
        ↓
Cache Mapping
        ↓
Write Policies
        ↓
Cache Coherency
        ↓
ARM Cortex-A
        ↓
Embedded Linux
        ↓
RTOS
        ↓
Performance Optimization
        ↓
Embedded Systems
```

**Cache Architecture is one of the most important concepts in Computer Architecture, Embedded Systems, Linux Kernel development, and Performance Optimization because it directly affects CPU speed, memory latency, and overall system performance.**
