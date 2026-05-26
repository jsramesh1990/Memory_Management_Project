# free

---

#  Introduction

In C and C++, `free()` is used to **release dynamically allocated memory** back to the heap.

It is the counterpart of `malloc()`, `calloc()`, and `realloc()`.

In embedded systems, correct usage of `free()` is critical because improper memory release leads to:

* memory leaks
* heap exhaustion
* system instability
* real-time failures

---

#  Definition

`free()` is a standard library function that **deallocates memory previously allocated using heap functions**.

---

#  Syntax

```c id="syntax1"
void free(void *ptr);
```

---

#  Working Flow of free()

```text id="flow1"
Pointer passed to free()
        |
        v
Heap manager identifies memory block
        |
        v
Marks block as free
        |
        v
Returns memory to free list
        |
        v
Memory becomes reusable
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

#  Memory Flow Diagram

```text id="diag1"
Before free:
[ allocated memory block ]

After free:
[ free memory block available ]
```

---

#  Embedded System Example

## UART Buffer Release

```c id="ex2"
char *buffer = (char*)malloc(128);

if(buffer != NULL) {
    // use buffer
    free(buffer);
    buffer = NULL;
}
```

---

# ⚠ Why setting pointer to NULL is important

```c id="safe1"
free(ptr);
ptr = NULL;
```

### Benefits:

* prevents dangling pointer
* avoids accidental reuse
* safer debugging

---

#  free() Working Flow in Embedded System

```text id="flow2"
Task allocates memory
        |
        v
Uses memory
        |
        v
Calls free()
        |
        v
Memory returned to heap
        |
        v
Heap can reuse block
```

---

#  Real-Time Embedded Example

## Sensor Buffer Handling

```c id="ex3"
void processSensor() {
    int *data = (int*)malloc(100 * sizeof(int));

    if(data == NULL) return;

    // process data

    free(data);
    data = NULL;
}
```

---

#  What happens if free is NOT used?

```text id="flow3"
malloc called repeatedly
        |
        v
Memory never released
        |
        v
Heap gradually fills
        |
        v
Memory leak occurs
        |
        v
System crash
```

---

#  free vs malloc

| Feature   | malloc          | free           |
| --------- | --------------- | -------------- |
| Purpose   | Allocate memory | Release memory |
| Direction | Heap → Program  | Program → Heap |
| Risk      | NULL allocation | Double free    |
| Usage     | Memory creation | Memory cleanup |

---

# ⚠ Common Mistakes

---

## 1. Double free

```c id="m1"
free(ptr);
free(ptr); //  undefined behavior
```

---

## 2. Freeing NULL pointer (safe but redundant)

```c id="m2"
free(NULL); // safe but no effect
```

---

## 3. Using pointer after free

```c id="m3"
free(ptr);
*ptr = 10; //  dangling pointer
```

---

## 4. Not freeing memory

```c id="m4"
ptr = malloc(100);
// missing free → memory leak
```

---

#  Embedded Best Practices

---

## 1. Always set pointer to NULL after free

```c id="best1"
free(ptr);
ptr = NULL;
```

---

## 2. Free memory in reverse order of allocation

```text id="best2"
LIFO cleanup preferred
```

---

## 3. Avoid frequent malloc/free cycles

---

## 4. Ensure single ownership of memory

---

#  free() Flow Diagram

```text id="flow4"
Pointer received
      |
      v
Heap block identified
      |
      v
Block marked free
      |
      v
Added to free list
      |
      v
Available for reuse
```

---

#  Embedded System Impact

If `free()` is used incorrectly:

* system may crash
* heap corruption occurs
* interrupts may fail
* real-time deadlines missed

---


