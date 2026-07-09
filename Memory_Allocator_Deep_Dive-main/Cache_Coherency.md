# Cache Coherency

## Table of Contents

1. Introduction
2. Definition
3. Why Cache Coherency is Needed
4. Cache Coherency Problem
5. Cache Coherency vs Cache Consistency
6. Multi-Core Cache Architecture
7. Cache States
8. MESI Protocol
9. MOESI Protocol
10. Snooping Protocol
11. Directory-Based Protocol
12. False Sharing
13. Cache Coherency in Embedded Systems
14. DMA and Cache Coherency
15. Real-Time Examples
16. Advantages
17. Disadvantages
18. Major Project Usage
19. Best Practices
20. Interview Questions
21. Conclusion

---

# 1. Introduction

Modern processors have:

```text
Multiple CPU Cores

Core 0

Core 1

Core 2

Core 3
```

Each core has:

```text
L1 Cache

L2 Cache
```

---

Architecture:

```text
                RAM

                 ↑

        ----------------

        |              |

     Core0          Core1

      ↓               ↓

    L1 Cache       L1 Cache

      ↓               ↓

    L2 Cache       L2 Cache
```

---

Problem:

Suppose:

```c
int x = 10;
```

---

Core0:

```c
x = 100;
```

stored in:

```text
Core0 Cache
```

---

Core1:

```c
printf("%d",x);
```

may still read:

```text
10
```

instead of:

```text
100
```

---

This creates:

# Cache Coherency Problem

---

# 2. Definition

## Definition

**Cache Coherency** is a mechanism that ensures all processor cores observe the same value of shared data even when multiple caches contain copies of that data.

---

## Simple Definition

```text
Core0 Cache

x = 100

↓

Update Other Caches

↓

Core1 Reads

x = 100
```

---

Goal:

```text
All Cores

↓

Same Data View
```

---

# 3. Why Cache Coherency is Needed

Without coherency:

```text
Core0

x=100
```

---

Core1:

```text
Still Reads

x=10
```

---

Results:

❌ Incorrect program output

❌ Race conditions

❌ Data corruption

❌ Unpredictable behavior

---

Required in:

* Multi-core CPUs
* Embedded Linux
* SMP RTOS
* Servers
* Smartphones

---

# 4. Cache Coherency Problem

Suppose:

```c
int counter = 0;
```

---

Initially:

```text
RAM

counter=0
```

---

Core0 reads:

```text
counter=0
```

stored in:

```text
Cache0
```

---

Core1 reads:

```text
counter=0
```

stored in:

```text
Cache1
```

---

State:

```text
RAM

counter=0


Cache0

counter=0


Cache1

counter=0
```

---

Core0:

```c
counter++;
```

---

Now:

```text
Cache0

counter=1
```

---

But:

```text
Cache1

counter=0
```

---

Problem:

```text
Same Variable

Different Values
```

---

Need:

```text
Invalidate

or

Update

Other Caches
```

---

# 5. Cache Coherency vs Cache Consistency

| Cache Coherency         | Memory Consistency              |
| ----------------------- | ------------------------------- |
| Same data across caches | Order of memory operations      |
| Hardware mechanism      | Hardware + Software             |
| Multi-core issue        | Synchronization issue           |
| Example: MESI           | Example: Sequential Consistency |

---

Coherency:

```text
Same Value?
```

---

Consistency:

```text
Correct Order?
```

---

# 6. Multi-Core Cache Architecture

Example:

```text
                RAM

                 ↑

         Shared L3 Cache

         ↑         ↑

      Core0     Core1

       ↓          ↓

     L2          L2

       ↓          ↓

     L1          L1
```

---

Each core:

```text
Private L1

Private L2
```

---

Shared:

```text
L3

RAM
```

---

Problem:

Same memory:

```text
x
```

exists:

```text
RAM

L3

L2 Core0

L1 Core0

L2 Core1

L1 Core1
```

---

Need:

```text
Synchronization
```

---

# 7. Cache States

Cache line usually contains:

```text
Data

+

Tag

+

State
```

---

State examples:

```text
Modified

Exclusive

Shared

Invalid
```

---

These states form:

# MESI Protocol

---

# 8. MESI Protocol

MESI stands for:

```text
M

Modified


E

Exclusive


S

Shared


I

Invalid
```

---

## Modified (M)

Cache contains:

```text
Latest Data
```

---

RAM:

```text
Old Data
```

---

Example:

```text
Cache0

x=100
```

RAM:

```text
x=10
```

---

Only:

```text
Cache0

owns data
```

---

## Exclusive (E)

Data:

```text
Present

Only

One Cache
```

---

RAM:

```text
Same Data
```

---

Example:

```text
Cache0

x=50
```

RAM:

```text
x=50
```

---

## Shared (S)

Data exists:

```text
Cache0

Cache1

RAM
```

---

All contain:

```text
Same Data
```

---

Read-only sharing.

---

## Invalid (I)

Cache line:

```text
Not Valid
```

---

Must fetch:

```text
RAM

or

Other Cache
```

---

MESI State Diagram:

```text
              Read Miss

          +-----------+

          |

      Invalid

      /      \

 Read        Write

  |            |

 Shared <-- Exclusive

    \          /

      Modified
```

---

# MESI Example

Initially:

```text
RAM

x=10
```

---

Core0 reads:

```text
Cache0

x=10

State=Exclusive
```

---

Core1 reads:

```text
Cache0

State=Shared


Cache1

State=Shared
```

---

Core0 writes:

```text
x=100
```

---

Protocol:

```text
Invalidate

Cache1
```

---

Final:

```text
Cache0

x=100

Modified


Cache1

Invalid
```

---

Problem solved.

---

# 9. MOESI Protocol

Extension of MESI.

---

States:

```text
Modified

Owned

Exclusive

Shared

Invalid
```

---

Extra state:

### Owned (O)

Cache owns:

```text
Modified Data
```

but:

```text
Can Share

With Other Caches
```

---

Benefits:

✅ Less RAM traffic

✅ Faster sharing

---

Used in:

```text
AMD Ryzen

ARM Cortex-A
```

---

# 10. Snooping Protocol

Most common approach.

---

All caches:

```text
Monitor

Shared Bus
```

---

Example:

Core0:

```text
Write x=100
```

---

Broadcast:

```text
Invalidate x
```

---

Core1 hears:

```text
Invalidate x
```

---

Cache1:

```text
x

↓

Invalid
```

---

Advantages:

✅ Simple

✅ Fast

---

Disadvantages:

❌ Heavy bus traffic

❌ Doesn't scale well

---

Used in:

```text
Intel CPUs

ARM Cortex-A
```

---

# 11. Directory-Based Protocol

Large systems:

```text
32 cores

64 cores

128 cores
```

---

Broadcast becomes expensive.

---

Use:

```text
Directory
```

---

Directory stores:

```text
Who Has

Which Cache Line
```

---

Example:

```text
x

↓

Core0

Core5

Core7
```

---

Update:

```text
Notify Only

Core0

Core5

Core7
```

---

Advantages:

✅ Scalable

✅ Less Bus Traffic

---

Disadvantages:

❌ Extra Hardware

❌ More Complex

---

Used in:

```text
Servers

Supercomputers
```

---

# 12. False Sharing

Very important interview topic.

---

Suppose:

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

---

Core1:

```c
b++;
```

---

Even though:

```text
Different Variables
```

they may exist in:

```text
Same Cache Line
```

---

Result:

```text
Invalidate

Update

Invalidate

Update
```

continuously.

---

Performance:

```text
Very Poor
```

---

This is:

# False Sharing

---

Solution:

Add:

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
Different Cache Lines
```

---

Performance improves.

---

# 13. Cache Coherency in Embedded Systems

Important in:

---

### ARM Cortex-A

```text
L1

L2

L3

MESI/MOESI
```

---

### Raspberry Pi

```text
Multi-Core

Cache Coherency
```

---

### STM32H7

```text
I-Cache

D-Cache
```

---

DMA:

```text
Special Handling Required
```

---

### ESP32

```text
Dual Core

Shared Memory

Cache Coherency
```

---

# 14. DMA and Cache Coherency

Very important for Embedded Interviews.

---

CPU:

```text
Updates Cache
```

---

DMA:

```text
Reads RAM
```

---

Problem:

```text
Cache

x=100


RAM

x=10
```

---

DMA reads:

```text
10
```

Wrong.

---

Need:

```text
Cache Clean

or

Cache Flush
```

---

Before DMA:

```c
SCB_CleanDCache();
```

---

After DMA:

```c
SCB_InvalidateDCache();
```

---

Used heavily in:

```text
STM32H7

NXP iMX

ARM Cortex-A

Zynq
```

---

# 15. Real-Time Examples

---

## Example 1

Producer Consumer

Core0:

```c
buffer[0]=100;
flag=1;
```

---

Core1:

```c
while(flag==0);

printf("%d",buffer[0]);
```

---

Need:

```text
Memory Barrier

Cache Coherency
```

---

Otherwise:

```text
Old Data

May Be Read
```

---

## Example 2

DMA

CPU:

```c
txBuffer[0]=100;
```

---

DMA:

```text
Reads RAM
```

---

If cache not cleaned:

```text
DMA

Gets Old Data
```

---

Need:

```text
SCB_CleanDCache()
```

---

# 16. Advantages

✅ Consistent data across cores

✅ Enables multi-core processing

✅ Improves scalability

✅ Supports parallel applications

✅ Reduces software complexity

---

# 17. Disadvantages

❌ Extra hardware complexity

❌ Bus traffic overhead

❌ Power consumption

❌ False sharing

❌ Coherency latency

---

# 18. Major Project Usage

Cache coherency is used in:

---

### Intel CPUs

```text
MESI
```

---

### AMD Ryzen

```text
MOESI
```

---

### ARM Cortex-A

```text
ACE

MESI

MOESI
```

---

### Raspberry Pi

```text
Cache Coherent SMP
```

---

### Linux Kernel

Uses:

```text
Memory Barriers

Cache Flush

SMP Synchronization
```

---

### STM32H7

Uses:

```text
DCache Clean

DCache Invalidate
```

---

# 19. Best Practices

---

## Avoid False Sharing

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

## Use Atomic Variables

```c
std::atomic<int> counter;
```

---

## Use Memory Barriers

For:

```text
Shared Variables

DMA

SMP
```

---

## Clean Cache Before DMA

```c
SCB_CleanDCache();
```

---

## Invalidate After DMA

```c
SCB_InvalidateDCache();
```

---

# 20. Interview Questions

### Q1. What is Cache Coherency?

Ensures all CPU cores see the same value of shared data.

---

### Q2. Why is Cache Coherency required?

Because:

```text
Multiple Caches

Can Store

Same Variable
```

which may become inconsistent.

---

### Q3. What does MESI stand for?

```text
Modified

Exclusive

Shared

Invalid
```

---

### Q4. What is MOESI?

MESI plus:

```text
Owned State
```

---

### Q5. What is False Sharing?

Different variables:

```text
Same Cache Line
```

causing unnecessary invalidations.

---

### Q6. Difference between Coherency and Consistency?

| Coherency | Consistency            |
| --------- | ---------------------- |
| Same Data | Same Order             |
| Hardware  | Hardware + Software    |
| MESI      | Sequential Consistency |

---

### Q7. Why is DMA a coherency problem?

Because:

```text
CPU

Uses Cache


DMA

Uses RAM
```

Data may become inconsistent.

---

# 21. Conclusion

**Cache Coherency** ensures that all processor cores and devices observe a consistent view of shared memory despite having separate caches.

---

## Complete Coherency Flow

```text
Core0

↓

L1 Cache


Core1

↓

L1 Cache


↓

MESI / MOESI

↓

Shared L3

↓

RAM
```

---

## Cache Coherency Summary

| Feature             | Description                 |
| ------------------- | --------------------------- |
| Purpose             | Keep Shared Data Consistent |
| Common Protocol     | MESI                        |
| Advanced Protocol   | MOESI                       |
| Multi-Core Required | Yes                         |
| DMA Concern         | Yes                         |
| False Sharing       | Major Performance Issue     |
| Embedded Usage      | High                        |
| Linux Usage         | Very High                   |

---

## Quick Interview Cheat Sheet

| Topic            | Answer                               |
| ---------------- | ------------------------------------ |
| Purpose          | Consistent Shared Data               |
| MESI             | Modified, Exclusive, Shared, Invalid |
| MOESI            | MESI + Owned                         |
| False Sharing    | Same Cache Line                      |
| Coherency Method | Snooping                             |
| Large Systems    | Directory Protocol                   |
| DMA Fix          | Clean + Invalidate Cache             |
| ARM Cortex-A     | Cache Coherent                       |
| STM32H7          | Manual DCache Maintenance            |

---

## Learning Path

```text
Computer Architecture
        ↓
Memory Hierarchy
        ↓
Cache Architecture
        ↓
Cache Mapping
        ↓
Cache Coherency
        ↓
MESI Protocol
        ↓
MOESI Protocol
        ↓
DMA
        ↓
Memory Barriers
        ↓
ARM Cortex-A
        ↓
Embedded Linux
        ↓
SMP RTOS
        ↓
Performance Optimization
```

**Cache Coherency is one of the most important concepts for Multi-Core Processors, Embedded Linux, ARM Cortex-A systems, DMA-based embedded systems, Linux Kernel development, and high-performance computing because incorrect coherency handling can lead to subtle bugs, stale data, and severe performance issues.**
