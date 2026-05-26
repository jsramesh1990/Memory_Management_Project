# memory-pool

---

#  Introduction

In embedded systems, a **memory pool** is a pre-allocated block of memory that is divided into fixed-size chunks and managed internally.

Instead of using `malloc()` and `free()`, memory pools provide:

* predictable allocation time
* no fragmentation
* better real-time performance
* safer memory handling

Memory pools are widely used in **RTOS, drivers, networking stacks, and safety-critical firmware**.

---

#  Definition

A memory pool is a **fixed-size memory allocator** where a large block of memory is split into smaller blocks for repeated use.

---

#  Working Flow of Memory Pool

```text id="flow1"
System startup
      |
      v
Allocate large memory block
      |
      v
Divide into fixed-size chunks
      |
      v
Maintain free list
      |
      v
Request → give free block
      |
      v
Release → return block to pool
```

---

#  Memory Pool Diagram

```text id="diag1"
+----------------------------+
| Block 1 (free)            |
| Block 2 (used)            |
| Block 3 (free)            |
| Block 4 (used)            |
| Block 5 (free)            |
+----------------------------+
        ↑
   Free list pointer
```

---

#  Simple Memory Pool Implementation

```c id="ex1"
#include <stdio.h>

#define POOL_SIZE 5

typedef struct {
    int data;
    int used;
} Block;

Block pool[POOL_SIZE];
```

---

## Initialize Pool

```c id="ex2"
void initPool() {
    for(int i = 0; i < POOL_SIZE; i++) {
        pool[i].used = 0;
    }
}
```

---

## Allocate Block

```c id="ex3"
Block* allocate() {
    for(int i = 0; i < POOL_SIZE; i++) {
        if(pool[i].used == 0) {
            pool[i].used = 1;
            return &pool[i];
        }
    }
    return NULL;
}
```

---

## Free Block

```c id="ex4"
void freeBlock(Block *b) {
    b->used = 0;
}
```

---

#  Memory Pool Flow

```text id="flow2"
Request memory
      |
      v
Check free list
      |
      v
Return fixed block
      |
      v
Use memory
      |
      v
Return to pool (free list updated)
```

---

#  Embedded System Use Cases

Memory pools are used in:

* RTOS task management
* UART/SPI buffers
* network packet handling
* sensor data buffers
* CAN communication systems
* real-time audio processing

---

#  Why Memory Pool is Better than malloc

---

## 1. No Fragmentation

```text id="f1"
malloc → fragmented memory
pool   → fixed blocks
```

---

## 2. Predictable Timing

```text id="f2"
malloc → unpredictable delay
pool   → constant time allocation
```

---

## 3. Deterministic Behavior

```text id="f3"
Important for real-time systems
```

---

## 4. Safer Memory Usage

```text id="f4"
No random heap corruption
```

---

# 📊 malloc vs Memory Pool

| Feature        | malloc/free | Memory Pool |
| -------------- | ----------- | ----------- |
| Speed          | Slow        | Fast        |
| Fragmentation  | Yes         | No          |
| Deterministic  | No          | Yes         |
| Safety         | Low         | High        |
| Embedded usage | Limited     | Preferred   |

---

#  Real-Time Embedded Example

## UART Packet Buffer

```c id="ex5"
Block *packet = allocate();

if(packet != NULL) {
    packet->data = 100;
}
```

---

## Free after use

```c id="ex6"
freeBlock(packet);
```

---

#  Memory Pool Flow in RTOS

```text id="flow3"
Task requests memory
        |
        v
Memory pool manager checks free list
        |
        v
Block assigned instantly
        |
        v
Task processes data
        |
        v
Block returned to pool
```

---

#  Memory Pool Structure

```text id="struct1"
+----------------------+
| Pool Header         |
| Free List Pointer   |
| Block Array         |
| Status Table        |
+----------------------+
```

---

#  Limitations of Memory Pool

---

## 1. Fixed Size Limitation

```text id="l1"
All blocks same size → waste if data is small
```

---

## 2. Pre-allocation Cost

```text id="l2"
Memory reserved at startup
```

---

## 3. Manual Management

```text id="l3"
Developer must manage allocation/freeing
```

---

#  When to Use Memory Pool

* real-time systems
* embedded drivers
* RTOS applications
* communication stacks
* safety-critical systems

---

#  When NOT to Use

* general desktop applications
* highly dynamic memory sizes
* non-real-time systems

---

#  Common Mistakes

---

## 1. Forgetting to free block

```c id="m1"
leads to pool exhaustion
```

---

## 2. Double free

```c id="m2"
same block freed twice → corruption
```

---

## 3. Using invalid block

```c id="m3"
access after release → undefined behavior
```

---


