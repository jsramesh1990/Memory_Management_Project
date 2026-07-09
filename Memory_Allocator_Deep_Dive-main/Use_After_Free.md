# Use After Free (UAF)

## Table of Contents

1. What is Use After Free?
2. Why Does It Occur?
3. Heap Memory Refresher
4. How Use After Free Happens
5. Memory Lifecycle
6. Simple Example
7. What Happens When Code Hits It?
8. Real-Time Examples
9. Use After Free vs Memory Leak
10. Use After Free vs Dangling Pointer
11. Consequences of Use After Free
12. Detection Methods
13. Prevention Techniques
14. C++ Smart Pointer Solutions
15. Embedded Systems Perspective
16. Interview Questions
17. Quick Revision
18. Interview Answer (2 Minutes)

---

# What is Use After Free?

## Definition

Use After Free (UAF) occurs when a program continues to access memory after that memory has already been released using `free()` (C) or `delete` (C++).

---

Example:

```c
int *ptr = malloc(sizeof(int));

free(ptr);

*ptr = 100;    // UAF
```

Problem:

```text
Memory Already Released
But Still Being Used
```

---

## Simple Definition

> Use After Free happens when a pointer accesses memory that has already been deallocated.

---

# Why Does It Occur?

Common causes:

```text
Pointer Not Set To NULL
Multiple References To Same Memory
Complex Ownership Logic
Multithreading Issues
Incorrect Resource Management
```

---

Example:

```c
char *buf = malloc(100);

free(buf);

/* Forgot that memory is freed */

strcpy(buf, "HELLO");
```

UAF occurs.

---

# Heap Memory Refresher

Allocation:

```c
char *ptr = malloc(100);
```

Memory:

```text
Heap

+------------+
| 100 Bytes  |
+------------+
```

---

Free:

```c
free(ptr);
```

Memory becomes available to allocator.

---

Important:

```text
Pointer Variable Still Exists
Memory Does Not Belong To You Anymore
```

---

# How Use After Free Happens

### Step 1

Allocate memory.

```c
int *ptr =
malloc(sizeof(int));
```

Memory:

```text
ptr
 ↓
+------+
| 100  |
+------+
```

---

### Step 2

Free memory.

```c
free(ptr);
```

Memory:

```text
ptr
 ↓
+------+
| FREE |
+------+
```

---

### Step 3

Use pointer again.

```c
*ptr = 200;
```

UAF occurs.

---

# Memory Lifecycle

Correct:

```text
Allocate
   ↓
Use
   ↓
Free
   ↓
Stop Using
```

---

Incorrect:

```text
Allocate
   ↓
Use
   ↓
Free
   ↓
Use Again ❌
```

---

# Simple Example

```c
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *ptr =
        malloc(sizeof(int));

    *ptr = 10;

    free(ptr);

    printf("%d\n", *ptr);  // UAF
}
```

---

Problem:

```text
Memory Already Released
```

Result:

```text
Undefined Behavior
```

Possible outputs:

```text
10
Random Value
Crash
Segmentation Fault
```

---

# What Happens When Code Hits It?

Example:

```c
char *buf =
malloc(20);

free(buf);

buf[0] = 'A';
```

---

### Step 1

Heap block allocated.

```text
Heap Block Created
```

---

### Step 2

Memory freed.

```text
Block Returned To Heap
```

---

### Step 3

Allocator may reuse block.

Example:

```c
char *new_buf =
malloc(20);
```

Same memory may be assigned elsewhere.

---

### Step 4

Old pointer writes:

```c
buf[0] = 'A';
```

---

Now:

```text
New Object Corrupted
```

---

### Interview Answer

**What happens when code hits a Use After Free bug?**

The program accesses memory that has already been returned to the allocator. The allocator may have reused that memory for another object. Any read or write operation through the stale pointer can corrupt unrelated data, cause crashes, produce incorrect results, or create security vulnerabilities. The behavior is undefined and unpredictable.

---

# Real-Time Examples

## Example 1: UART Buffer

```c
char *rx =
malloc(128);

free(rx);

/* Accidentally reused */

rx[0] = 0;
```

UAF.

---

## Example 2: Linked List Node

```c
Node *node =
malloc(sizeof(Node));

free(node);

node->next = NULL;
```

Accessing freed node.

---

## Example 3: Automotive ECU

Diagnostic session object:

```c
Session *session =
create_session();

destroy_session(session);
```

Later:

```c
session->state = ACTIVE;
```

UAF.

---

## Example 4: Multithreading

Thread A:

```c
free(shared_obj);
```

Thread B:

```c
shared_obj->value = 5;
```

Classic UAF race condition.

---

# Use After Free vs Memory Leak

| Use After Free    | Memory Leak      |
| ----------------- | ---------------- |
| Memory Freed      | Memory Not Freed |
| Access After Free | Lost Pointer     |
| Memory Corruption | Memory Waste     |
| Dangerous         | Resource Loss    |

---

Example:

### UAF

```c
free(ptr);
*ptr = 10;
```

---

### Memory Leak

```c
ptr = malloc(100);

ptr = NULL;
```

Allocated memory lost.

---

# Use After Free vs Dangling Pointer

## Dangling Pointer

A pointer that points to invalid memory.

Example:

```c
free(ptr);
```

Now:

```text
ptr → Invalid Memory
```

Pointer is dangling.

---

## Use After Free

Occurs when that dangling pointer is actually used.

Example:

```c
free(ptr);

*ptr = 100;
```

UAF.

---

Interview Definition:

```text
Every UAF uses a dangling pointer.
Not every dangling pointer causes UAF.
```

---

# Consequences of Use After Free

## Data Corruption

```text
Overwrite Another Object
```

---

## Random Behavior

```text
Wrong Results
```

---

## Crashes

```text
Segmentation Fault
Bus Fault
Hard Fault
Abort
```

---

## Security Vulnerabilities

Attackers may exploit:

```text
Function Pointers
Virtual Tables
Heap Structures
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
Use After Free
Heap Overflow
Double Free
```

---

## Valgrind

```bash
valgrind ./app
```

Detects invalid memory access.

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
```

Good:

```c
free(ptr);

ptr = NULL;
```

---

Benefit:

```c
ptr[0] = 'A';
```

becomes:

```text
NULL Pointer Error
```

which is easier to detect.

---

## Ownership Rules

Clearly define:

```text
Who Allocates?
Who Frees?
Who Owns Memory?
```

---

## Avoid Shared Raw Pointers

Bad:

```c
obj1 = ptr;
obj2 = ptr;
```

Both may free memory.

---

## Use Smart Pointers (C++)

Use:

```cpp
std::unique_ptr
std::shared_ptr
std::weak_ptr
```

---

# C++ Smart Pointer Solutions

## unique_ptr

```cpp
std::unique_ptr<int> ptr =
    std::make_unique<int>(10);
```

Automatic cleanup.

No accidental UAF.

---

## shared_ptr

Reference-counted ownership.

```cpp
std::shared_ptr<MyObj> obj;
```

---

## weak_ptr

Prevents accessing destroyed objects.

```cpp
std::weak_ptr<MyObj> weak;
```

Can check validity before use.

---

# Embedded Systems Perspective

Very important interview topic.

---

Typical scenario:

```c
uint8_t *buffer =
malloc(256);

free(buffer);
```

ISR or task later uses:

```c
buffer[0] = 1;
```

Results:

```text
Memory Corruption
Hard Fault
Random Crash
```

---

Automotive Example:

```text
UDS Session Object
CAN Buffer
Network Packet Buffer
```

Freed too early.

Later accessed.

---

# Interview Questions

## What is Use After Free?

Use After Free occurs when a program accesses memory after it has been deallocated.

---

## Why is Use After Free Dangerous?

Because the freed memory may already be reused by another object, causing corruption or crashes.

---

## What Happens When Code Hits It?

The program accesses invalid memory. The memory may belong to another object or be unmapped, leading to undefined behavior.

---

## What is a Dangling Pointer?

A pointer that references memory that is no longer valid.

---

## How Can You Prevent Use After Free?

```text
Set Pointer To NULL
Clear Ownership Rules
Use Smart Pointers
Avoid Shared Raw Pointers
```

---

## Which Tools Detect UAF?

```text
AddressSanitizer
Valgrind
Coverity
Cppcheck
```

---

# Most Asked Interview Question

## Explain Use After Free with an Example.

Consider:

```c
int *ptr =
malloc(sizeof(int));

free(ptr);

*ptr = 100;
```

After `free()`, the memory is returned to the heap allocator and no longer belongs to the program. The pointer still contains the old address, becoming a dangling pointer. Accessing the memory through this pointer is called Use After Free. The memory may already have been reused by another object, leading to corruption, crashes, or unpredictable behavior.

---

# Interview Answer (2 Minutes)

Use After Free is a memory management bug where a program accesses memory after it has been deallocated. It typically occurs when a pointer continues to be used after a call to `free()` or `delete`. Although the pointer still holds the old address, the memory no longer belongs to the program and may be reused by the allocator. Accessing it can cause data corruption, crashes, segmentation faults, hard faults, or security vulnerabilities. Common prevention techniques include setting pointers to NULL after freeing, defining clear ownership rules, avoiding shared raw pointers, and using C++ smart pointers such as `unique_ptr`, `shared_ptr`, and `weak_ptr`.

---

# Quick Revision

```text
Use After Free (UAF)

Definition:
Using Memory After free()

Example:

free(ptr);

*ptr = 10;

Problem:
Memory No Longer Valid

Causes:
✔ Dangling Pointer
✔ Ownership Bugs
✔ Multithreading Issues
✔ Incorrect Resource Handling

Effects:
✔ Data Corruption
✔ Crash
✔ Segmentation Fault
✔ Hard Fault
✔ Security Issues

Detection:
✔ AddressSanitizer
✔ Valgrind
✔ Coverity

Prevention:
✔ ptr = NULL
✔ Smart Pointers
✔ Ownership Rules

Most Important:

free(ptr)
     ↓
Pointer Still Exists
     ↓
Using It Again
     ↓
Use After Free
```

---

# Conclusion

Use After Free is one of the most dangerous memory-management bugs in C and C++. It occurs when memory is accessed after it has been released, often through a dangling pointer. Because freed memory may be reused for other objects, UAF bugs can cause corruption, crashes, hard faults, and serious security vulnerabilities. Understanding UAF is essential for Embedded Systems, Firmware Development, Linux Programming, Automotive Software, RTOS applications, and C/C++ technical interviews.
