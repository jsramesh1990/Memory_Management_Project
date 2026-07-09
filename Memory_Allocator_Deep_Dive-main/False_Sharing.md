# False Sharing

## Table of Contents

1. Introduction
2. Definition
3. Why False Sharing Happens
4. Cache Line Basics
5. False Sharing vs True Sharing
6. How False Sharing Occurs
7. Step-by-Step Example
8. Performance Impact
9. Detecting False Sharing
10. Avoiding False Sharing
11. False Sharing in Embedded Systems
12. Real-Time Examples
13. Advantages and Disadvantages
14. Major Project Usage
15. Best Practices
16. Interview Questions
17. Conclusion

---

# 1. Introduction

Modern CPUs are:

```text
Multi-Core

Core0

Core1

Core2

Core3
```

Each core has:

```text
L1 Cache

L2 Cache
```

and stores data in units called:

```text
Cache Lines
```

Typically:

```text
64 Bytes
```

---

Suppose:

```c
struct Data
{
    int a;
    int b;
};
```

Memory layout:

```text
Address

0x1000 -> a

0x1004 -> b
```

Both variables fit inside:

```text
Same Cache Line
```

---

Core0:

```c
a++;
```

Core1:

```c
b++;
```

---

Although:

```text
Different Variables
```

the CPU repeatedly:

```text
Invalidate Cache

Reload Cache

Invalidate Again
```

This is called:

# False Sharing

---

# 2. Definition

## Definition

**False Sharing** occurs when multiple CPU cores modify different variables that happen to reside in the same cache line, causing unnecessary cache invalidations and performance degradation.

---

## Simple Definition

```text
Core0

updates a

↓

Invalidate Cache Line


Core1

updates b

↓

Invalidate Cache Line


Repeat...
```

---

Even though:

```text
a ≠ b
```

they share:

```text
Same Cache Line
```

---

# 3. Why False Sharing Happens

CPU caches do NOT track:

```text
Individual Variables
```

They track:

```text
Cache Lines
```

Example:

Cache line size:

```text
64 Bytes
```

---

Memory:

```text
Address

0x1000

↓

a


0x1004

↓

b


0x1008

↓

c


0x100C

↓

d
```

All belong to:

```text
Same Cache Line
```

---

When:

```text
a changes
```

CPU assumes:

```text
Entire Cache Line Changed
```

and invalidates:

```text
b

c

d

also
```

---

# 4. Cache Line Basics

Typical cache line:

```text
64 Bytes
```

---

Example:

```text
Cache Line

---------------------------------

a

b

c

d

e

f

g

h

---------------------------------

64 Bytes
```

---

CPU transfers:

```text
Whole Cache Line
```

not:

```text
Single Variable
```

---

Important:

```text
Cache Coherency

works at

Cache Line Granularity
```

---

# 5. False Sharing vs True Sharing

| False Sharing             | True Sharing            |
| ------------------------- | ----------------------- |
| Different Variables       | Same Variable           |
| Same Cache Line           | Same Variable           |
| Unnecessary Invalidations | Necessary Invalidations |
| Performance Problem       | Correct Synchronization |
| Avoidable                 | Required                |

---

## True Sharing Example

```c
int counter;
```

---

Core0:

```c
counter++;
```

---

Core1:

```c
counter++;
```

---

Both access:

```text
Same Variable
```

This is:

```text
True Sharing
```

---

## False Sharing Example

```c
struct Data
{
    int a;

    int b;
};
```

---

Core0:

```c
a++;
```

Core1:

```c
b++;
```

---

Different variables.

But:

```text
Same Cache Line
```

This is:

```text
False Sharing
```

---

# 6. How False Sharing Occurs

Suppose:

```c
struct Data
{
    int a;
    int b;
};
```

---

Memory:

```text
0x1000

a

0x1004

b
```

---

Core0:

```text
Reads

Cache Line

[a,b]
```

---

Core1:

```text
Reads

Cache Line

[a,b]
```

---

State:

```text
Core0 Cache

[a,b]


Core1 Cache

[a,b]
```

---

Core0:

```c
a++;
```

MESI:

```text
Core1 Cache

↓

Invalid
```

---

Core1:

```c
b++;
```

MESI:

```text
Core0 Cache

↓

Invalid
```

---

Repeat:

```text
Invalidate

Reload

Invalidate

Reload

...
```

---

Performance:

```text
Terrible
```

---

# 7. Step-by-Step Example

### Initial

```text
RAM

a=0

b=0
```

---

Core0:

```c
a++;
```

---

Cache0:

```text
a=1

b=0

Modified
```

---

Cache1:

```text
Invalid
```

---

Core1:

```c
b++;
```

---

Must:

```text
Fetch Cache Line

[a,b]

Update b

Invalidate Core0
```

---

Core0:

```c
a++;
```

---

Again:

```text
Fetch Cache Line

Invalidate Core1
```

---

Repeated:

```text
Thousands

or

Millions

of Times
```

---

# 8. Performance Impact

False sharing causes:

---

## Excessive Cache Invalidations

```text
Invalidate

↓

Reload

↓

Invalidate

↓

Reload
```

---

## Increased Memory Traffic

CPU spends time:

```text
Moving Cache Lines

Instead of

Executing Code
```

---

## CPU Stall Cycles

CPU:

```text
Waiting

For Cache Synchronization
```

---

## Poor Scalability

More cores:

```text
More Invalidations
```

---

Performance:

```text
4 cores

↓

May become

Slower than

1 core
```

---

# 9. Detecting False Sharing

Common symptoms:

---

### CPU Usage High

But:

```text
Low Throughput
```

---

### Adding Threads

```text
More Threads

↓

Less Performance
```

---

### High Cache Misses

Tools show:

```text
L1 Misses

L2 Misses

Cache Invalidations
```

---

## Detection Tools

### Linux Perf

```bash
perf stat

perf record
```

---

### Intel VTune

Shows:

```text
False Sharing

Cache Line Contention
```

---

### AMD uProf

Detects:

```text
Cache Contention
```

---

# 10. Avoiding False Sharing

## Method 1

### Add Padding

Bad:

```c
struct Data
{
    int a;

    int b;
};
```

---

Good:

```c
struct Data
{
    int a;

    char pad[60];

    int b;
};
```

---

Now:

```text
a

and

b
```

are in:

```text
Different Cache Lines
```

---

## Method 2

Align Variables

```c
struct Data
{
    alignas(64) int a;

    alignas(64) int b;
};
```

---

Each variable:

```text
Separate Cache Line
```

---

## Method 3

Thread Local Storage

Bad:

```c
globalCounter++;
```

---

Good:

```c
threadLocalCounter++;
```

---

Each thread:

```text
Own Memory
```

---

## Method 4

Per-Core Buffers

Example:

```text
Core0

buffer0


Core1

buffer1


Core2

buffer2
```

---

No sharing:

```text
No False Sharing
```

---

# 11. False Sharing in Embedded Systems

Very important in:

---

### ARM Cortex-A

```text
Multi-Core

L1

L2

MESI
```

---

### Raspberry Pi

```text
4 Cores

Cache Coherency
```

---

### NXP i.MX

```text
Shared DDR

Cache Coherent
```

---

### STM32H7

Single core.

Usually:

```text
No False Sharing
```

---

### ESP32

Dual Core.

Possible:

```text
False Sharing
```

especially:

```text
Shared Buffers
```

---

# 12. Real-Time Examples

---

## Example 1

Bad Counter Design

```c
struct Counter
{
    int core0;

    int core1;
};
```

---

Core0:

```c
counter.core0++;
```

---

Core1:

```c
counter.core1++;
```

---

Problem:

```text
Same Cache Line
```

---

Performance:

```text
Poor
```

---

## Better

```c
struct Counter
{
    alignas(64)

    int core0;


    alignas(64)

    int core1;
};
```

---

Performance:

```text
Much Faster
```

---

## Example 2

Packet Processing

Bad:

```text
Shared Statistics

↓

False Sharing

↓

Slow Networking
```

---

Good:

```text
Per Core Statistics

↓

Combine Later
```

---

Used in:

```text
DPDK

Linux Kernel

Network Drivers
```

---

# 13. Advantages and Disadvantages

### Advantages

Actually:

```text
False Sharing

Has

NO Advantages
```

---

The only "advantage":

```text
Variables

Close Together

Less Memory
```

but performance:

```text
Much Worse
```

---

### Disadvantages

❌ Frequent invalidations

❌ Cache line bouncing

❌ More memory traffic

❌ Poor scalability

❌ Lower throughput

❌ Higher latency

❌ Wasted CPU cycles

---

# 14. Major Project Usage

False sharing is a major optimization topic in:

---

### Linux Kernel

Avoid:

```text
Shared Counters

Shared Statistics
```

---

### DPDK

Uses:

```text
Per Core Buffers

Cache Line Alignment
```

---

### Intel TBB

Optimizes:

```text
Task Schedulers

Work Stealing
```

---

### Java JVM

Uses:

```text
@Contended
```

to avoid:

```text
False Sharing
```

---

### Redis

Uses:

```text
Per Thread Data
```

---

### Embedded Linux

Uses:

```text
Per CPU Variables
```

---

# 15. Best Practices

✅ Align shared structures to:

```text
64 Bytes
```

---

✅ Use:

```c
alignas(64)
```

---

✅ Use:

```text
Per Thread Variables
```

---

✅ Use:

```text
Per CPU Buffers
```

---

✅ Avoid:

```text
Multiple Counters

Inside Same Struct
```

---

✅ Profile using:

```text
perf

VTune

uProf
```

---

# 16. Interview Questions

### Q1. What is False Sharing?

When multiple cores modify different variables located in the same cache line causing unnecessary cache invalidations.

---

### Q2. Why does False Sharing happen?

Because:

```text
Cache Coherency

works on

Cache Lines

not

Variables
```

---

### Q3. Is False Sharing a correctness issue?

```text
No
```

It is:

```text
Performance Problem
```

---

### Q4. Difference between False Sharing and True Sharing?

| False Sharing       | True Sharing            |
| ------------------- | ----------------------- |
| Different Variables | Same Variable           |
| Same Cache Line     | Same Variable           |
| Performance Problem | Synchronization Problem |
| Avoidable           | Necessary               |

---

### Q5. How to avoid False Sharing?

* Padding
* `alignas(64)`
* Thread Local Storage
* Per Core Buffers
* Separate Cache Lines

---

### Q6. Which cache protocol is involved?

```text
MESI

MOESI
```

because cache invalidations occur through coherency protocols.

---

# 17. Conclusion

**False Sharing** is a performance problem that occurs when multiple CPU cores update different variables stored in the same cache line, causing excessive cache invalidations and cache line bouncing.

---

## False Sharing Workflow

```text
Core0

updates a

↓

Invalidate Cache Line


Core1

updates b

↓

Invalidate Cache Line


Core0

Reload

↓

Update


Core1

Reload

↓

Update


Repeat...
```

---

## Solution

```text
Separate Variables

↓

Separate Cache Lines

↓

No Invalidations

↓

Better Performance
```

---

## False Sharing Summary

| Feature            | Description                |
| ------------------ | -------------------------- |
| Problem Type       | Performance                |
| Correctness Issue  | No                         |
| Cause              | Same Cache Line            |
| Cache Granularity  | Usually 64 Bytes           |
| Detection Tools    | perf, VTune, uProf         |
| Solution           | Padding, Alignment         |
| Protocol Involved  | MESI / MOESI               |
| Embedded Relevance | High in Multi-Core Systems |

---

## Quick Interview Cheat Sheet

| Topic            | Answer                                 |
| ---------------- | -------------------------------------- |
| False Sharing    | Different variables in same cache line |
| Main Cause       | Cache-line granularity                 |
| Cache Line Size  | Typically 64 Bytes                     |
| Problem Type     | Performance                            |
| Detection        | perf, VTune                            |
| Avoidance        | Padding                                |
| C++ Keyword      | `alignas(64)`                          |
| Protocol         | MESI                                   |
| Embedded Example | ESP32 Dual Core                        |
| Linux Example    | Per CPU Variables                      |

---

## Learning Path

```text
Computer Architecture
        ↓
Memory Hierarchy
        ↓
Cache Architecture
        ↓
Cache Coherency
        ↓
MESI Protocol
        ↓
False Sharing
        ↓
Memory Barriers
        ↓
SMP Systems
        ↓
ARM Cortex-A
        ↓
Embedded Linux
        ↓
Linux Kernel
        ↓
Performance Optimization
```

**False Sharing is a favorite interview topic for Embedded Linux, Linux Kernel, Multi-Core ARM, High-Performance Computing, and System Programming roles because it explains why adding more CPU cores does not always improve performance and how cache-aware programming can dramatically increase scalability.**
