# memory-leaks

---

#  Introduction

In C and C++, a **memory leak** happens when dynamically allocated memory is not properly released, causing unused memory to remain occupied.

In embedded systems, memory leaks are **critical issues** because RAM is very limited and usually non-expandable.

Even a small leak can:

* crash the system
* cause unpredictable behavior
* degrade performance over time
* lead to reboot or watchdog reset

---

#  Definition

A memory leak occurs when:

* memory is allocated using `malloc/new`
* but is not freed using `free/delete`
* and is no longer accessible by the program

---

#  Syntax Example

## Allocation

```c id="m1"
int *ptr = (int*)malloc(sizeof(int));
```

## Missing Free (Leak)

```c id="m2"
// no free(ptr); → memory leak
```

---

#  Memory Leak Working Flow

```text id="flow1"
malloc/new request
        |
        v
Memory allocated in heap
        |
        v
Pointer lost / overwritten
        |
        v
No reference to memory
        |
        v
Memory remains occupied forever (LEAK)
```

---

#  Memory Leak Diagram

```text id="diag1"
Heap Memory:

Before:
+-----------+
| Block A   |  ← ptr
+-----------+

After loss:
+-----------+
| Block A   |  ← no pointer (leaked)
+-----------+
```

---

#  Simple Example (Leak)

```c id="ex1"
#include <stdlib.h>

int main() {
    while(1) {
        int *ptr = (int*)malloc(sizeof(int));

        // ❌ no free(ptr)
    }
}
```

---

#  What Happens Here?

* memory is allocated repeatedly
* never released
* heap slowly fills
* system eventually crashes

---

#  Embedded System Impact

```text id="flow2"
Continuous operation
       |
       v
Repeated allocation
       |
       v
RAM gradually fills
       |
       v
System slowdown
       |
       v
Watchdog reset / crash
```

---

#  Real-Time Embedded Example

---

## UART Buffer Leak

```c id="ex2"
void receiveData() {
    char *buffer = (char*)malloc(128);

    // processing data

    // ❌ forgot free(buffer)
}
```

---

## Timer-Based Leak

```c id="ex3"
void timerTask() {
    int *data = (int*)malloc(10 * sizeof(int));

    // no free → repeated call leaks memory
}
```

---

#  Types of Memory Leaks

---

# 1. Persistent Leak

Memory is never freed.

```c id="t1"
ptr = malloc(100);
// no free ever
```

---

# 2. Lost Pointer Leak

Pointer overwritten before freeing.

```c id="t2"
ptr = malloc(100);
ptr = malloc(200); // old memory lost
```

---

# 3. Scope-Based Leak

Memory allocated in function but not freed.

```c id="t3"
void func() {
    int *p = malloc(50);
} // leak
```

---

#  Memory Leak Flow in Embedded System

```text id="flow3"
Task runs repeatedly
        |
        v
malloc called each cycle
        |
        v
No free executed
        |
        v
Heap grows continuously
        |
        v
System instability
```

---

#  Heap Usage Over Time

```text id="graph1"
Time →
RAM usage ↑

|        /
|       /
|      /
|     /
|____/____________
   normal → leak → crash
```

---

#  How to Prevent Memory Leaks

---

## 1. Always free memory

```c id="s1"
free(ptr);
ptr = NULL;
```

---

## 2. Match malloc/free pairs

```c id="s2"
ptr = malloc(100);
// later
free(ptr);
```

---

## 3. Avoid dynamic memory in embedded systems

```text id="s3"
Preferred: static memory allocation
```

---

## 4. Use static buffers instead

```c id="s4"
static char buffer[128];
```

---

## 5. Use RAII (C++)

```cpp id="s5"
std::vector<int> v;
```

---

# ⚠ Common Mistakes

---

## 1. Forgetting free()

```c id="m1"
ptr = malloc(100);
// missing free → leak
```

---

## 2. Overwriting pointer

```c id="m2"
ptr = malloc(100);
ptr = malloc(200); // leak
```

---

## 3. Returning malloc without handling

```c id="m3"
return malloc(100); // caller forgets to free
```

---


# ⚖ Memory Leak vs Dangling Pointer

| Feature       | Memory Leak    | Dangling Pointer         |
| ------------- | -------------- | ------------------------ |
| Memory status | Lost           | Freed but referenced     |
| Access        | Not possible   | Possible but invalid     |
| Risk          | RAM exhaustion | Crash/undefined behavior |

---
