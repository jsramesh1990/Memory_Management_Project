# Double Free

## Table of Contents

1. What is Double Free?
2. Why Does It Occur?
3. Heap Memory Refresher
4. How Double Free Happens
5. Memory Lifecycle
6. Simple Example
7. What Happens When Code Hits It?
8. Real-Time Examples
9. Double Free vs Use After Free
10. Double Free vs Memory Leak
11. Consequences of Double Free
12. Detection Methods
13. Prevention Techniques
14. C++ Smart Pointer Solutions
15. Embedded Systems Perspective
16. Interview Questions
17. Quick Revision
18. Interview Answer (2 Minutes)

---

# What is Double Free?

## Definition

A Double Free occurs when a program attempts to free the same memory block more than once.

---

Example:

```c
int *ptr = malloc(sizeof(int));

free(ptr);

free(ptr);   // Double Free
```

---

Problem:

```text
Memory Already Released
But Program Tries To Release It Again
```

---

## Simple Definition

> Double Free happens when `free()` (or `delete`) is called multiple times on the same memory block.

---

# Why Does It Occur?

Common reasons:

```text
Multiple Owners Of Same Pointer
Poor Memory Management
Error Handling Bugs
Complex Control Flow
Dangling Pointers
```

---

Example:

```c
char *buf = malloc(100);

free(buf);

/* Forgot it was already freed */

free(buf);
```

---

# Heap Memory Refresher

Allocate:

```c
int *ptr =
malloc(sizeof(int));
```

Memory:

```text
Heap

+--------+
| Object |
+--------+
```

---

Free:

```c
free(ptr);
```

Memory:

```text
Heap

+--------+
| FREE   |
+--------+
```

Returned to allocator.

---

Problem:

```c
free(ptr);
```

again.

Allocator receives the same block twice.

---

# How Double Free Happens

### Step 1

Allocate memory.

```c
int *ptr =
malloc(sizeof(int));
```

---

### Step 2

Free memory.

```c
free(ptr);
```

Memory returned to heap.

---

### Step 3

Pointer still contains address.

```text
ptr
 ↓
0x1000
```

---

### Step 4

Free again.

```c
free(ptr);
```

Double Free occurs.

---

# Memory Lifecycle

## Correct

```text
Allocate
   ↓
Use
   ↓
Free
   ↓
Stop Using Pointer
```

---

## Incorrect

```text
Allocate
   ↓
Use
   ↓
Free
   ↓
Free Again ❌
```

---

# Simple Example

```c
#include <stdlib.h>

int main()
{
    int *ptr =
        malloc(sizeof(int));

    free(ptr);

    free(ptr);   // Double Free
}
```

---

Result:

```text
Undefined Behavior
```

Possible outcomes:

```text
Crash
Abort
Heap Corruption
No Immediate Error
```

---

# What Happens When Code Hits It?

Example:

```c
char *buf =
malloc(100);

free(buf);

free(buf);
```

---

### Step 1

Allocator creates heap block.

```text
Heap Block Allocated
```

---

### Step 2

`free(buf)` called.

```text
Block Returned To Free List
```

---

### Step 3

Allocator marks block as available.

---

### Step 4

Second `free(buf)` occurs.

Allocator thinks:

```text
This Block Is Being Freed Again
```

---

### Step 5

Heap structures become inconsistent.

Possible issues:

```text
Corrupted Free List
Corrupted Metadata
Invalid Heap State
```

---

### Step 6

Future allocations may fail.

```c
malloc()
free()
```

can crash.

---

## Interview Answer

**What happens when code hits a double free?**

The memory allocator receives a request to free a memory block that has already been released. This can corrupt internal heap management structures such as free lists and metadata. Modern allocators often detect the error and terminate the program, but if not detected, it may lead to heap corruption, crashes, undefined behavior, or security vulnerabilities.

---

# Real-Time Examples

## Example 1: Multiple Function Ownership

```c
void func1(char *ptr)
{
    free(ptr);
}

void func2(char *ptr)
{
    free(ptr);
}
```

---

Usage:

```c
char *buf = malloc(100);

func1(buf);
func2(buf);
```

Double Free.

---

## Example 2: Error Handling

```c
char *buf =
malloc(100);

if(error)
{
    free(buf);
}

free(buf);
```

Double Free.

---

## Example 3: Linked List

```c
Node *node =
malloc(sizeof(Node));

free(node);

free(node);
```

Double Free.

---

## Example 4: Automotive ECU

Diagnostic buffer:

```c
uint8_t *diag_buf =
malloc(512);
```

Freed in:

```text
Communication Module
Diagnostic Module
```

Both attempt cleanup.

Double Free.

---

# Double Free vs Use After Free

| Double Free                   | Use After Free           |
| ----------------------------- | ------------------------ |
| Free memory twice             | Access memory after free |
| `free(ptr)` twice             | `*ptr = 10` after free   |
| Heap corruption               | Memory corruption        |
| Allocator structures affected | User data affected       |

---

Example:

### Double Free

```c
free(ptr);
free(ptr);
```

---

### Use After Free

```c
free(ptr);

ptr[0] = 'A';
```

---

# Double Free vs Memory Leak

| Double Free     | Memory Leak         |
| --------------- | ------------------- |
| Too many frees  | No free             |
| Heap corruption | Memory waste        |
| Crash possible  | Resource exhaustion |
| Dangerous       | Inefficient         |

---

Example:

### Memory Leak

```c
ptr = malloc(100);

ptr = NULL;
```

Memory lost.

---

### Double Free

```c
free(ptr);
free(ptr);
```

Heap corruption.

---

# Consequences of Double Free

## Heap Corruption

Allocator metadata damaged.

---

## Program Crash

Possible:

```text
Abort
Segmentation Fault
Hard Fault
Bus Fault
```

---

## Unpredictable Behavior

Future allocations fail.

---

## Security Vulnerabilities

Attackers may exploit:

```text
Heap Metadata Corruption
Free List Manipulation
Memory Reuse Attacks
```

---

# Detection Methods

## Address Sanitizer

Compile:

```bash
-fsanitize=address
```

Detects:

```text
Double Free
Use After Free
Heap Overflow
```

---

## Valgrind

```bash
valgrind ./app
```

Reports:

```text
Invalid Free
Double Free
```

---

## Static Analysis

Tools:

* Coverity
* Cppcheck
* Clang Static Analyzer

---

# Prevention Techniques

## Set Pointer To NULL

Bad:

```c
free(ptr);
free(ptr);
```

---

Good:

```c
free(ptr);

ptr = NULL;
```

---

Safe:

```c
free(ptr);

ptr = NULL;

free(ptr);
```

In C:

```text
free(NULL)
```

does nothing.

---

## Single Ownership

One module owns memory.

```text
Allocate
Use
Free
```

One owner only.

---

## Clear Cleanup Logic

Avoid:

```text
Multiple Cleanup Paths
```

that may free the same memory twice.

---

## Check Pointer Before Free

```c
if(ptr)
{
    free(ptr);
    ptr = NULL;
}
```

---

# C++ Smart Pointer Solutions

## unique_ptr

```cpp
std::unique_ptr<int> ptr =
    std::make_unique<int>(10);
```

Automatically freed once.

Double free impossible.

---

## shared_ptr

Reference counting.

```cpp
std::shared_ptr<MyObj> obj;
```

Freed when count reaches zero.

---

## weak_ptr

Used with shared ownership.

Avoids ownership confusion.

---

# Embedded Systems Perspective

Very important interview topic.

---

Common scenario:

```c
uint8_t *buffer =
malloc(256);
```

Module A:

```c
free(buffer);
```

Module B:

```c
free(buffer);
```

Double Free.

---

Results:

```text
Heap Corruption
Hard Fault
Watchdog Reset
Random Crash
```

---

RTOS systems:

```text
Limited Heap
Custom Allocators
```

are especially vulnerable.

---

# Interview Questions

## What is Double Free?

Double Free occurs when the same memory block is released more than once.

---

## Why is Double Free Dangerous?

Because it corrupts allocator metadata and heap structures.

---

## What Happens When Code Hits It?

The allocator attempts to free an already freed block, leading to heap corruption or runtime errors.

---

## How Can You Prevent Double Free?

```text
Set Pointer To NULL
Single Ownership
Smart Pointers
Careful Cleanup Logic
```

---

## Difference Between Double Free and Use After Free?

Double Free releases memory twice, while Use After Free accesses memory after it has been released.

---

## Why Does `ptr = NULL` Help?

Because:

```c
free(NULL);
```

is safe and does nothing.

---

# Most Asked Interview Question

## Explain Double Free with an Example.

Consider:

```c
char *buf =
malloc(100);

free(buf);

free(buf);
```

The first `free()` correctly returns the memory block to the heap allocator. The second `free()` attempts to release the same block again. Since the allocator already considers the block free, internal heap structures may become corrupted. Modern allocators often detect this and terminate the program, but otherwise it can lead to crashes, undefined behavior, or security vulnerabilities.

---

# Interview Answer (2 Minutes)

Double Free is a memory-management bug that occurs when the same dynamically allocated memory block is freed more than once. After a successful `free()`, the memory is returned to the heap allocator and should no longer be released again. Calling `free()` a second time on the same pointer can corrupt heap metadata, damage free lists, and cause crashes or security vulnerabilities. Common causes include multiple ownership of pointers, error-handling bugs, and poor resource-management design. Prevention techniques include setting pointers to NULL after freeing, enforcing clear ownership rules, and using C++ smart pointers such as `unique_ptr` and `shared_ptr`.

---

# Quick Revision

```text
Double Free

Definition:
Freeing Same Memory Twice

Example:

free(ptr);
free(ptr);

Problem:
Heap Metadata Corruption

Causes:
✔ Multiple Owners
✔ Cleanup Bugs
✔ Dangling Pointers
✔ Error Handling

Effects:
✔ Crash
✔ Heap Corruption
✔ Hard Fault
✔ Security Issues

Detection:
✔ AddressSanitizer
✔ Valgrind
✔ Coverity

Prevention:
✔ ptr = NULL
✔ Single Ownership
✔ Smart Pointers

Most Important:

free(ptr)
     ↓
Memory Released
     ↓
free(ptr) Again
     ↓
Double Free
```

---

# Conclusion

Double Free is a serious heap-management bug where the same memory block is released multiple times. It can corrupt allocator metadata, destabilize heap structures, and lead to crashes, hard faults, and security vulnerabilities. Understanding Double Free is essential for Embedded Systems, RTOS development, Linux programming, Automotive Software, Firmware Development, and C/C++ technical interviews because it is one of the most common memory-management errors encountered in real-world systems.
