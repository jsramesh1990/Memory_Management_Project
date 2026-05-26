# dynamic_memory_allocation

---

#  Introduction

Dynamic Memory Allocation (DMA) in C/C++ refers to **allocating memory at runtime from the heap** instead of compile time.

In embedded systems, DMA is used when:

* memory size is not known in advance
* buffers must grow/shrink dynamically
* communication or sensor data is variable
* flexible data structures are needed

However, it must be used carefully due to limited RAM and real-time constraints.

---

#  Definition

Dynamic Memory Allocation is the process of **requesting memory from the heap during program execution and releasing it when no longer needed**.

---

#  Memory Allocation Types

```text id="flow1"
Static Allocation → Compile time (stack/data)
Dynamic Allocation → Runtime (heap)
```

---

#  Memory Layout in Embedded System

```text id="mem1"
+-----------------------+
| Flash (Code)          |
|-----------------------|
| .text                 |
| .rodata               |
+-----------------------+
| RAM                   |
|-----------------------|
| .data                 |
| .bss                  |
| Heap  ← Dynamic memory
| Stack ← Function calls
+----------------------+
```

---

#  Dynamic Memory Functions in C

| Function  | Purpose                    |
| --------- | -------------------------- |
| malloc()  | Allocate memory            |
| calloc()  | Allocate + zero initialize |
| realloc() | Resize memory              |
| free()    | Release memory             |

---

#  Dynamic Memory Flow

```text id="flow2"
Program starts
      |
      v
Request memory (malloc/calloc)
      |
      v
Heap allocates block
      |
      v
Pointer returned
      |
      v
Memory used in program
      |
      v
free() releases memory
```

---

#  Simple Example

```c id="ex1"
#include <stdlib.h>

int main() {
    int *ptr = (int*)malloc(sizeof(int));

    if(ptr != NULL) {
        *ptr = 10;
    }

    free(ptr);

    return 0;
}
```

---

#  Embedded System Example

## Sensor Data Buffer

```c id="ex2"
#include <stdlib.h>

void readSensor() {
    int *data = (int*)malloc(100 * sizeof(int));

    if(data == NULL) return;

    // process data

    free(data);
}
```

---

#  Dynamic Allocation Flow Diagram

```text id="flow3"
Request memory
      |
      v
Heap manager checks availability
      |
      v
Memory allocated
      |
      v
Pointer returned
      |
      v
Data processing
      |
      v
Memory freed
```

---

#  Key Characteristics

* Happens at runtime
* Uses heap memory
* Requires manual management
* Flexible size allocation
* Slower than static memory
* Can fail (NULL return)

---

#  Problems in Embedded Systems

---

## 1. Memory Leak

```text id="p1"
allocated but not freed → RAM exhaustion
```

---

## 2. Fragmentation

```text id="p2"
heap becomes fragmented → allocation failure
```

---

## 3. Unpredictable Timing

```text id="p3"
not suitable for real-time deadlines
```

---

## 4. Heap Exhaustion

```text id="p4"
no memory left → system crash
```

---

#  Real-Time Embedded Example

## UART Buffer

```c id="ex3"
char *rxBuffer = (char*)calloc(128, sizeof(char));

if(rxBuffer != NULL) {
    // use buffer
    free(rxBuffer);
}
```

---

#  Advantages

| Advantage                   | Explanation             |
| --------------------------- | ----------------------- |
| Flexibility                 | Size decided at runtime |
| Efficient usage             | No wasted memory        |
| Supports complex structures | Linked lists, trees     |
| Scalable design             | Adaptive systems        |

---

#  Disadvantages

| Issue                | Explanation          |
| -------------------- | -------------------- |
| Memory leaks         | Improper free usage  |
| Fragmentation        | Poor heap management |
| Slower execution     | Heap overhead        |
| Unpredictable timing | Not real-time safe   |

---

#  Static vs Dynamic Memory

| Feature      | Static       | Dynamic |
| ------------ | ------------ | ------- |
| Allocation   | Compile time | Runtime |
| Speed        | Fast         | Slow    |
| Flexibility  | Low          | High    |
| Risk         | Low          | High    |
| Embedded use | Preferred    | Limited |

---

#  Best Practices in Embedded Systems

---

## 1. Prefer static memory

```c id="best1"
static int buffer[100];
```

---

## 2. Always check NULL

```c id="best2"
if(ptr == NULL) return;
```

---

## 3. Always free memory

```c id="best3"
free(ptr);
ptr = NULL;
```

---

## 4. Avoid frequent allocations

---

## 5. Use memory pools instead of heap (recommended)

---

#  Dynamic Memory Lifecycle

```text id="flow4"
Allocate memory
      |
      v
Use memory
      |
      v
Modify data
      |
      v
Free memory
      |
      v
Return to heap
```

---

