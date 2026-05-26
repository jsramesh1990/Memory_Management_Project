# malloc

---

#  Introduction

In C and C++, `malloc()` is a standard library function used for **dynamic memory allocation** from the heap during runtime.

In embedded systems, `malloc()` is used when memory size is not known at compile time, but it must be used carefully due to limited RAM and real-time constraints.

---

#  Definition

`malloc()` stands for **Memory Allocation**. It allocates a block of memory on the heap and returns a pointer to the beginning of that block.

---

#  Syntax

```c id="syntax1"
void* malloc(size_t size);
```

---

#  Working Flow of malloc

```text id="flow1"
Program requests memory
        |
        v
Heap manager searches free block
        |
        v
Allocates requested size
        |
        v
Returns pointer to memory block
        |
        v
Program uses memory
```

---

#  Simple Example

```c id="ex1"
#include <stdlib.h>

int main() {
    int *ptr = (int*)malloc(sizeof(int));

    *ptr = 10;

    free(ptr);

    return 0;
}
```

---

#  Key Characteristics

* Allocates memory at runtime
* Returns `void*` pointer
* Memory is uninitialized (garbage values)
* Requires manual `free()`
* Allocated from heap
* Can fail and return `NULL`

---

#  malloc Memory Flow Diagram

```text id="diag1"
Heap Memory:

Before:
[ free memory blocks ]

malloc request
        ↓

After:
[ allocated block ][ free memory ]
```

---

#  Embedded System Example

## UART Buffer Allocation

```c id="ex2"
char *buffer = (char*)malloc(128);

if(buffer == NULL) {
    // handle allocation failure
}

buffer[0] = 'A';

free(buffer);
```

---

#  malloc Failure Handling

```c id="fail1"
if(ptr == NULL) {
    // memory allocation failed
}
```

---

#  Why malloc is risky in Embedded Systems

* Heap fragmentation
* Unpredictable allocation time
* Limited RAM availability
* Risk of memory leaks
* Real-time performance issues

---

#  malloc vs Static Memory

| Feature      | malloc  | Static Memory |
| ------------ | ------- | ------------- |
| Allocation   | Runtime | Compile time  |
| Speed        | Slow    | Fast          |
| Risk         | High    | Low           |
| Flexibility  | High    | Low           |
| Embedded use | Limited | Preferred     |

---

#  malloc Flow in Embedded System

```text id="flow2"
Task runs
      |
      v
malloc request
      |
      v
Heap checks free memory
      |
      v
Memory assigned or NULL returned
      |
      v
Task continues execution
```

---

#  Real-Time Embedded Example

## Sensor Data Buffer

```c id="ex3"
void readSensor() {
    int *data = (int*)malloc(50 * sizeof(int));

    if(data == NULL) return;

    // process sensor data

    free(data);
}
```

---

#  Common Mistakes

---

## 1. Memory leak

```c id="m1"
ptr = malloc(100);
// no free → leak
```

---

## 2. Using uninitialized memory

```c id="m2"
int *ptr = malloc(sizeof(int));
// ptr contains garbage
```

---

## 3. Forgetting NULL check

```c id="m3"
ptr = malloc(100);
*ptr = 10; // crash if NULL
```

---

## 4. Double free (related issue)

```c id="m4"
free(ptr);
free(ptr); // undefined behavior
```

---

#  malloc Flow Diagram

```text id="flow3"
malloc call
     |
     v
Heap allocation
     |
     v
Pointer returned
     |
     v
Memory used
     |
     v
free() called
     |
     v
Memory released
```

---

#  Embedded Best Practices

---

## 1. Always check NULL

```c id="safe1"
if(ptr == NULL) return;
```

---

## 2. Always free memory

```c id="safe2"
free(ptr);
ptr = NULL;
```

---

## 3. Prefer static memory in critical systems

```c id="safe3"
static int buffer[100];
```

---

## 4. Avoid frequent malloc/free cycles

---


