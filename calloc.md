# calloc

---

#  Introduction

In C and C++, `calloc()` is a dynamic memory allocation function used to allocate memory for multiple elements and initialize all bytes to **zero**.

In embedded systems, `calloc()` is useful when you need:

* clean initialization of buffers
* predictable zeroed memory
* array-like dynamic structures
* communication buffers (UART/SPI/CAN)

However, like `malloc()`, it must be used carefully due to heap limitations.

---

#  Definition

`calloc()` stands for **Contiguous Allocation**. It allocates memory for an array of elements and initializes all bytes to zero.

---

#  Syntax

```c id="syntax1"
void* calloc(size_t num, size_t size);
```

---

#  Working Flow of calloc()

```text id="flow1"
Request num * size memory
        |
        v
Heap manager allocates block
        |
        v
Memory is initialized to zero
        |
        v
Pointer returned to program
        |
        v
Program uses memory
```

---

#  Simple Example

```c id="ex1"
#include <stdlib.h>

int main() {
    int *ptr = (int*)calloc(5, sizeof(int));

    // all values are initialized to 0

    free(ptr);

    return 0;
}
```

---

#  Memory Behavior Diagram

```text id="diag1"
Before calloc:
[ random garbage memory ]

After calloc:
[ 0 ][ 0 ][ 0 ][ 0 ][ 0 ]
```

---

#  Embedded System Example

## Sensor Data Buffer

```c id="ex2"
int *buffer = (int*)calloc(10, sizeof(int));

if(buffer == NULL) {
    // handle failure
}

// buffer initialized to zero
buffer[0] = 100;

free(buffer);
```

---

#  calloc Flow in Embedded System

```text id="flow2"
Task requests memory
        |
        v
Heap allocates block
        |
        v
All bytes set to zero
        |
        v
Pointer returned
        |
        v
Task uses clean memory
```

---

#  Real-Time Embedded Example

## UART RX Buffer

```c id="ex3"
char *rxBuffer = (char*)calloc(128, sizeof(char));

if(rxBuffer != NULL) {
    // safe zero-initialized buffer
}
```

---

#  Why calloc is useful in Embedded Systems

* avoids uninitialized data bugs
* safer for communication buffers
* simplifies initialization logic
* reduces manual memset usage

---

#  calloc vs malloc

| Feature               | calloc                | malloc         |
| --------------------- | --------------------- | -------------- |
| Memory initialization | Zero initialized      | Garbage values |
| Arguments             | 2 (num, size)         | 1 (size)       |
| Speed                 | Slower                | Faster         |
| Safety                | Higher                | Lower          |
| Embedded use          | Preferred for buffers | General use    |

---

#  Common Mistakes

---

## 1. Forgetting NULL check

```c id="m1"
ptr = calloc(10, sizeof(int));
*ptr = 5; //  crash if NULL
```

---

## 2. Memory leak

```c id="m2"
ptr = calloc(10, sizeof(int));
// no free → leak
```

---

## 3. Using after free

```c id="m3"
free(ptr);
ptr[0] = 1; //  invalid access
```

---

## 4. Overusing calloc unnecessarily

```text id="m4"
Using calloc when initialization is not needed → performance loss
```

---

#  Embedded Best Practices

---

## 1. Always check allocation

```c id="best1"
if(ptr == NULL) return;
```

---

## 2. Free memory properly

```c id="best2"
free(ptr);
ptr = NULL;
```

---

## 3. Use calloc for buffers needing zero initialization

---

## 4. Avoid frequent dynamic allocation in real-time systems

---

#  calloc Flow Diagram

```text id="flow3"
calloc call
      |
      v
Heap allocation
      |
      v
Memory set to zero
      |
      v
Pointer returned
      |
      v
Memory used safely
      |
      v
free() releases memory
```

---

#  Embedded Impact

Using calloc improves:

* data safety
* buffer reliability
* system predictability

But may reduce performance due to zero initialization overhead.

---
