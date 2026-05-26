# chunks

---

#  Introduction

In embedded systems, a **chunk** refers to a small, fixed or variable-sized block of memory used to store data.

Chunks are commonly used in:

* memory pools
* heap allocation systems
* packet buffering (UART/SPI/CAN)
* audio/video streaming
* RTOS message queues
* DMA buffers

---

#  Definition

A chunk is a **contiguous block of memory allocated for storing a unit of data or multiple related data elements**.

---

#  Key Idea

Instead of handling large continuous memory randomly, systems divide memory into **manageable chunks** for efficiency and safety.

---

#  Chunk Working Flow

```text id="flow1"
Request data storage
        |
        v
Memory divided into chunks
        |
        v
Allocate free chunk
        |
        v
Store data in chunk
        |
        v
Release chunk when done
```

---

#  Chunk Memory Diagram

```text id="diag1"
+---------+---------+---------+---------+
| Chunk 1 | Chunk 2 | Chunk 3 | Chunk 4 |
|  DATA   |  FREE   |  DATA   |  FREE   |
+---------+---------+---------+---------+
```

---

#  Types of Chunks

---

# 1. Fixed-Size Chunks

All chunks have equal size.

## Example Use Case:

* memory pools
* RTOS buffers

```text id="t1"
[128B][128B][128B][128B]
```

---

## Advantage:

* no fragmentation
* fast allocation

---

# 2. Variable-Size Chunks

Chunks differ in size.

## Example Use Case:

* file systems
* dynamic packet handling

```text id="t2"
[64B][256B][128B][512B]
```

---

## Disadvantage:

* fragmentation risk
* complex management

---

#  Chunk Allocation Flow

```text id="flow2"
System request
      |
      v
Search free chunk
      |
      v
Assign chunk
      |
      v
Mark as used
      |
      v
Return pointer
```

---

#  Embedded System Example

## UART Buffer Using Chunks

```c id="ex1"
#define CHUNK_SIZE 64
#define TOTAL_CHUNKS 4

char memoryPool[CHUNK_SIZE * TOTAL_CHUNKS];

int used[TOTAL_CHUNKS] = {0};
```

---

## Allocate Chunk

```c id="ex2"
char* allocateChunk() {
    for(int i = 0; i < TOTAL_CHUNKS; i++) {
        if(used[i] == 0) {
            used[i] = 1;
            return &memoryPool[i * CHUNK_SIZE];
        }
    }
    return NULL;
}
```

---

## Free Chunk

```c id="ex3"
void freeChunk(char *ptr) {
    int index = (ptr - memoryPool) / CHUNK_SIZE;
    used[index] = 0;
}
```

---

#  Flow in Embedded System

```text id="flow3"
Incoming data (UART/SPI)
        |
        v
Assign free chunk
        |
        v
Store packet
        |
        v
Process data
        |
        v
Release chunk
```

---

#  Chunk vs Continuous Memory

| Feature       | Chunk Memory       | Continuous Memory  |
| ------------- | ------------------ | ------------------ |
| Allocation    | Fixed blocks       | Full dynamic block |
| Fragmentation | Low (fixed chunks) | High risk          |
| Efficiency    | High               | Medium             |
| Embedded use  | Very common        | Less preferred     |

---

#  Why Chunks are Important in Embedded Systems

* predictable memory usage
* fast allocation/deallocation
* avoids heap fragmentation
* ideal for real-time systems
* simplifies buffer management

---

#  Real-Time Embedded Example

## CAN Message Buffer

```c id="ex4"
typedef struct {
    int id;
    char data[8];
} CAN_Message;

CAN_Message *chunk = allocateChunk();
```

---

#  Chunk Lifecycle

```text id="flow4"
Chunk created
     |
     v
Assigned to task
     |
     v
Data processed
     |
     v
Chunk released
     |
     v
Returned to pool
```

---

#  Chunk Memory Layout

```text id="mem1"
Memory Pool
+------------------------+
| Chunk 1 (free)       |
| Chunk 2 (used)       |
| Chunk 3 (used)       |
| Chunk 4 (free)       |
+------------------------+
```

---

# ⚠ Common Problems

---

## 1. Chunk Leak

```text id="m1"
Chunk allocated but not freed
→ memory exhaustion
```

---

## 2. Double Free

```text id="m2"
Same chunk freed twice → corruption
```

---

## 3. Invalid Chunk Access

```text id="m3"
Access after release → undefined behavior
```

---

## 4. Fragmentation (variable chunks)

```text id="m4"
Memory split into unusable pieces
```

---

#  Best Practices

---

## 1. Use fixed-size chunks in embedded systems

```text id="safe1"
prevents fragmentation
```

---

## 2. Track chunk usage

```c id="safe2"
use bitmap or array
```

---

## 3. Always free chunks

```c id="safe3"
return to pool after use
```

---

## 4. Avoid large dynamic chunks

```text id="safe4"
reduces memory risk
```

---

