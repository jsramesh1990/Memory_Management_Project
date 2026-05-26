# realloc

---

#  Introduction

In C and C++, `realloc()` is used to **resize previously allocated heap memory**.

It is useful when:

* buffer size changes at runtime
* dynamic arrays need expansion/shrinking
* communication buffers grow (UART, CAN, SPI)
* data size is not fixed initially

However, in embedded systems, `realloc()` must be used carefully due to:

* fragmentation risk
* pointer invalidation
* unpredictable execution time

---

#  Definition

`realloc()` stands for **re-allocation of memory**, used to change the size of an existing memory block.

---

#  Syntax

```c id="syntax1"
void* realloc(void *ptr, size_t new_size);
```

---

#  Working Flow of realloc()

```text id="flow1"
Existing memory block
        |
        v
Request new size
        |
        v
Check if current block can expand
        |
        |---- Yes → expand in place
        |
        |---- No  → allocate new block
                     |
                     v
              Copy old data
                     |
                     v
              Free old block
        |
        v
Return new pointer
```

---

#  Simple Example

```c id="ex1"
#include <stdlib.h>

int main() {
    int *ptr = (int*)malloc(5 * sizeof(int));

    ptr = (int*)realloc(ptr, 10 * sizeof(int));

    free(ptr);

    return 0;
}
```

---

#  Memory Behavior Diagram

```text id="diag1"
Before realloc:
[ 5 blocks allocated ]

After realloc:
[ 10 blocks allocated (old data copied) ]
```

---

#  Embedded System Example

## Expanding UART Buffer

```c id="ex2"
char *buffer = (char*)malloc(64);

buffer = (char*)realloc(buffer, 128);

if(buffer == NULL) {
    // handle failure
}
```

---

#  Important Risk in realloc

If `realloc()` fails:

* it returns `NULL`
* original memory is NOT freed automatically (if assigned incorrectly)

---

## Safe usage pattern

```c id="safe1"
char *temp = realloc(ptr, new_size);

if(temp != NULL) {
    ptr = temp;
}
```

---

#  realloc Flow in Embedded System

```text id="flow2"
Task requests resize
        |
        v
Heap checks adjacent space
        |
        |---- Enough space → expand block
        |
        |---- Not enough → new allocation
                            copy data
                            free old block
        |
        v
Return updated pointer
```

---

#  Real-Time Embedded Example

## Sensor Data Expansion

```c id="ex3"
int *data = (int*)malloc(5 * sizeof(int));

// later more sensors added
data = (int*)realloc(data, 20 * sizeof(int));
```

---

#  Why realloc is dangerous in Embedded Systems

* unpredictable execution time
* may move memory block
* pointer invalidation risk
* heap fragmentation
* real-time constraint violation

---

#  realloc vs malloc

| Feature        | realloc       | malloc              |
| -------------- | ------------- | ------------------- |
| Purpose        | Resize memory | Allocate new memory |
| Preserves data | Yes           | No                  |
| Risk           | High          | Medium              |
| Speed          | Unpredictable | Predictable         |
| Embedded use   | Limited       | Controlled          |

---

#  Common Mistakes

---

## 1. Losing pointer on failure

```c id="m1"
ptr = realloc(ptr, size); //  risky
```

---

## 2. Memory leak

```c id="m2"
ptr = malloc(10);
ptr = realloc(ptr, 20); // old pointer lost if failure happens
```

---

## 3. Using old pointer after realloc

```c id="m3"
realloc(ptr, 20);
ptr[0] = 10; //  unsafe if pointer changed
```

---

## 4. Frequent realloc calls

```text id="m4"
causes fragmentation and performance issues
```

---

#  Embedded Best Practices

---

## 1. Always use temporary pointer

```c id="best1"
int *temp = realloc(ptr, new_size);
if(temp != NULL) {
    ptr = temp;
}
```

---

## 2. Avoid frequent resizing

---

## 3. Prefer memory pools in real-time systems

---

## 4. Pre-calculate required buffer size

---

#  realloc Flow Diagram

```text id="flow3"
Request resize
      |
      v
Check heap space
      |
      v
Expand OR allocate new block
      |
      v
Copy old data
      |
      v
Free old block
      |
      v
Return pointer
```

---

#  Embedded Impact

`realloc()` can:

* improve flexibility
* but harm real-time performance
* introduce fragmentation
* cause system instability if misused

---
