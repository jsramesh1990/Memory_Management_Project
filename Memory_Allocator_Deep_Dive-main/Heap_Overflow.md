# Heap Overflow

## Table of Contents

1. What is Heap Overflow?
2. Heap Memory Refresher
3. Why Heap Overflow Occurs
4. Heap Overflow vs Stack Overflow
5. Heap Overflow vs Buffer Overflow
6. Heap Memory Layout
7. How Heap Overflow Happens
8. Example 1: Simple Heap Overflow
9. Example 2: Overwriting Adjacent Heap Block
10. What Happens When Code Hits It?
11. Heap Metadata Corruption
12. Real-Time Embedded Examples
13. Consequences of Heap Overflow
14. Detection Methods
15. Prevention Techniques
16. Interview Questions
17. Quick Revision
18. Interview Answer (2 Minutes)

---

# What is Heap Overflow?

## Definition

A Heap Overflow occurs when a program writes beyond the boundaries of dynamically allocated memory on the heap.

Example:

```c
char *ptr = malloc(8);

strcpy(ptr, "HELLO_WORLD");
```

Allocated:

```text
8 Bytes
```

Data copied:

```text
11 Bytes + NULL
```

Overflow occurs.

---

## Simple Definition

> Heap overflow happens when data written to a heap-allocated buffer exceeds the allocated size and corrupts nearby heap memory.

---

# Heap Memory Refresher

Heap memory is used for:

```text
Dynamic Allocation
Runtime Memory Management
Variable Size Objects
```

Functions:

```c
malloc()
calloc()
realloc()
free()
```

Example:

```c
int *ptr =
malloc(sizeof(int));
```

Memory comes from:

```text
Heap Region
```

---

# Why Heap Overflow Occurs

Common causes:

```text
Wrong Buffer Size
strcpy()
strcat()
memcpy() Size Errors
Invalid Indexing
Length Calculation Bugs
```

Example:

```c
char *buf =
malloc(10);

buf[20] = 'A';
```

Overflow.

---

# Heap Overflow vs Stack Overflow

| Heap Overflow            | Stack Overflow                 |
| ------------------------ | ------------------------------ |
| Write beyond heap buffer | Stack memory exhausted         |
| Memory corruption        | Stack exhaustion               |
| Dynamic memory involved  | Function calls/local variables |
| Corrupts heap blocks     | Crashes due to full stack      |

---

# Heap Overflow vs Buffer Overflow

Many interviewers ask this.

### Buffer Overflow

General term.

```text
Writing Beyond Buffer
```

Can happen on:

```text
Stack
Heap
Global Memory
```

---

### Heap Overflow

Specific type of buffer overflow.

Occurs only on:

```text
Heap Memory
```

---

# Heap Memory Layout

Example:

```c
char *A = malloc(8);
char *B = malloc(8);
```

Memory:

```text
Heap

+--------+
| BlockA |
+--------+

+--------+
| BlockB |
+--------+
```

---

If A overflows:

```text
Heap

+--------+
| BlockA |
+--------+
     ↓
     ↓ Overflow

+--------+
| BlockB |
+--------+
```

Block B gets corrupted.

---

# How Heap Overflow Happens

Example:

```c
char *buf =
malloc(5);
```

Memory:

```text
Address

1000
1001
1002
1003
1004
```

---

Write:

```c
strcpy(buf,
       "HELLOWORLD");
```

Actual writes:

```text
1000 H
1001 E
1002 L
1003 L
1004 O

1005 W
1006 O
1007 R
1008 L
1009 D
```

---

Addresses:

```text
1005+
```

belong to another memory region.

Overflow occurs.

---

# Example 1: Simple Heap Overflow

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char *ptr =
        malloc(8);

    strcpy(ptr,
           "HELLO_WORLD");

    free(ptr);
}
```

Problem:

```text
8 Bytes Allocated

11+ Bytes Written
```

Result:

```text
Undefined Behavior
```

---

# Example 2: Overwriting Adjacent Heap Block

```c
char *A =
malloc(8);

char *B =
malloc(8);
```

Memory:

```text
+--------+
| A      |
+--------+

+--------+
| B      |
+--------+
```

---

Overflow:

```c
strcpy(A,
       "ABCDEFGHIJKLMNOP");
```

---

Memory:

```text
+--------+
| AAAAAA |
+--------+
| BBBBBB |
+--------+
```

B becomes corrupted.

---

# What Happens When Code Hits It?

Example:

```c
char *buf =
malloc(8);

strcpy(buf,
       "AUTOSAR_STACK");
```

---

Step 1

Heap manager allocates:

```text
8 Bytes
```

---

Step 2

CPU starts copying.

```text
A
U
T
O
S
A
R
...
```

---

Step 3

First 8 bytes fit.

---

Step 4

Additional bytes continue writing.

CPU does NOT know buffer size.

---

Step 5

Adjacent heap memory overwritten.

Possible targets:

```text
Heap Metadata
Other Variables
Pointers
Objects
```

---

Step 6

Program continues.

Corruption may remain hidden.

---

Step 7

Later:

```c
free(buf);
```

Heap manager reads corrupted metadata.

---

Results:

```text
Crash
Segmentation Fault
Abort
Memory Corruption
```

---

## Interview Answer

**What happens when code hits a heap overflow?**

The CPU writes data beyond the allocated heap buffer because it does not automatically enforce memory boundaries. Adjacent heap blocks or allocator metadata become corrupted. The corruption may not cause an immediate failure, but later heap operations such as `malloc()` or `free()` may detect invalid metadata and crash the application.

---

# Heap Metadata Corruption

Very important interview topic.

Heap allocators store metadata:

```text
Block Size
Allocation Status
Pointers
Free List Information
```

Memory:

```text
+----------+
| Metadata |
+----------+
| UserData |
+----------+
```

---

Overflow:

```text
+----------+
|CORRUPTED |
+----------+
| UserData |
+----------+
```

Later:

```c
free(ptr);
```

Allocator reads invalid metadata.

Crash occurs.

---

# Real-Time Embedded Examples

## Example 1: UART Buffer

```c
char *rx =
malloc(64);
```

Received:

```text
100 Bytes
```

Overflow.

---

## Example 2: CAN Message Processing

```c
uint8_t *frame =
malloc(8);
```

Copied:

```c
memcpy(frame,
       source,
       16);
```

Overflow.

---

## Example 3: Ethernet Packet

```c
uint8_t *packet =
malloc(1500);
```

Packet size validation missing.

Overflow occurs.

---

## Example 4: Automotive ECU

Diagnostic message:

```text
UDS Request
```

larger than expected.

Buffer overflow corrupts heap structures.

---

# Consequences of Heap Overflow

## Memory Corruption

```text
Variables Modified
Pointers Corrupted
Objects Damaged
```

---

## Crashes

```text
Segmentation Fault
Abort
Bus Fault
Hard Fault
```

---

## Security Vulnerabilities

Attackers may exploit:

```text
Heap Metadata Corruption
Function Pointer Overwrite
Object Corruption
```

---

## Random Behavior

Most dangerous.

Program appears fine.

Fails later.

---

# Detection Methods

## Address Sanitizer

Compile:

```bash
-fsanitize=address
```

Detects:

```text
Heap Overflow
Use After Free
Double Free
```

---

## Valgrind (Linux)

```bash
valgrind ./app
```

Finds:

```text
Invalid Writes
Memory Corruption
```

---

## Static Analysis

Tools:

* Coverity
* Cppcheck
* Clang Static Analyzer

---

## MPU/MMU

Embedded systems may detect invalid access using:

```text
MPU
MMU
```

---

# Prevention Techniques

## Validate Lengths

Bad:

```c
strcpy(dest, src);
```

Good:

```c
strncpy(dest,
        src,
        size);
```

---

## Check memcpy()

Bad:

```c
memcpy(dest,
       src,
       100);
```

Good:

```c
if(length <= size)
{
    memcpy(dest,
           src,
           length);
}
```

---

## Allocate Correct Size

Bad:

```c
malloc(8);
```

Good:

```c
malloc(strlen(str)+1);
```

---

## Use Safer APIs

```c
snprintf()
strncpy()
memmove()
```

---

# Interview Questions

## What is Heap Overflow?

Heap overflow occurs when a program writes beyond the size of dynamically allocated memory.

---

## Why Does Heap Overflow Occur?

Because the program writes more data than the allocated heap buffer can hold.

---

## What Happens When Code Hits It?

The CPU continues writing beyond the heap block boundaries, corrupting adjacent heap memory and allocator metadata.

---

## Why is Heap Overflow Dangerous?

It can corrupt:

```text
Heap Metadata
Objects
Pointers
Free Lists
```

leading to crashes and security vulnerabilities.

---

## Difference Between Heap Overflow and Stack Overflow?

| Heap Overflow             | Stack Overflow       |
| ------------------------- | -------------------- |
| Buffer Boundary Violation | Stack Exhaustion     |
| Dynamic Memory            | Function Call Memory |
| Memory Corruption         | No Stack Space Left  |

---

## Which Tools Detect Heap Overflow?

```text
AddressSanitizer
Valgrind
Coverity
Cppcheck
```

---

# Most Asked Interview Question

## Explain Heap Overflow with an Example.

Consider:

```c
char *buf =
malloc(8);

strcpy(buf,
       "HELLO_WORLD");
```

Only 8 bytes are allocated, but more than 8 bytes are copied. The CPU writes beyond the allocated heap block and corrupts adjacent heap memory. The corruption may affect nearby objects or allocator metadata. The application may crash immediately or later during heap operations such as `malloc()` or `free()`.

---

# Interview Answer (2 Minutes)

Heap overflow is a memory corruption error that occurs when data written to a dynamically allocated heap buffer exceeds the allocated size. Since the CPU does not automatically enforce heap boundaries, writing beyond the buffer corrupts adjacent heap memory or allocator metadata. Heap overflow can lead to crashes, segmentation faults, hard faults, unpredictable behavior, and security vulnerabilities. Common causes include incorrect buffer sizing, unsafe string functions, and invalid memory copy operations. In embedded systems, heap overflows often occur in communication buffers such as UART, CAN, Ethernet, and diagnostic message handling. Prevention techniques include bounds checking, safe APIs, static analysis, and runtime tools such as AddressSanitizer.

---

# Quick Revision

```text
Heap Overflow

Definition:
Writing Beyond Heap Buffer

Occurs In:
malloc()
calloc()
realloc()

Example:

char *buf =
malloc(8);

strcpy(buf,
       "HELLO_WORLD");

Result:
Heap Corruption

Affected:
✔ Heap Metadata
✔ Objects
✔ Pointers

Symptoms:
✔ Crash
✔ Hard Fault
✔ Segmentation Fault
✔ Random Behavior

Detection:
✔ AddressSanitizer
✔ Valgrind
✔ Coverity

Prevention:
✔ Bounds Checking
✔ strncpy()
✔ snprintf()
✔ Correct Allocation Size

Most Important:

Heap Overflow =
Writing More Data
Than Heap Memory Allocated
```

---

# Conclusion

Heap overflow is a specific type of buffer overflow that affects dynamically allocated memory. It occurs when a program writes beyond the allocated heap buffer, leading to memory corruption, allocator metadata damage, crashes, and potential security vulnerabilities. Understanding heap overflow is essential for Embedded Systems, Linux Programming, Firmware Development, RTOS Applications, Automotive Software, Cybersecurity, and technical interviews focused on memory management and software reliability.
