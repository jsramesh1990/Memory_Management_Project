# Buffer Overflow

## Table of Contents

1. [Introduction](#introduction)
2. [What is Buffer Overflow?](#what-is-buffer-overflow)
3. [Why Does Buffer Overflow Occur?](#why-does-buffer-overflow-occur)
4. [How Memory is Organized](#how-memory-is-organized)
5. [Types of Buffer Overflow](#types-of-buffer-overflow)
6. [Stack Buffer Overflow](#stack-buffer-overflow)
7. [Heap Buffer Overflow](#heap-buffer-overflow)
8. [Global Buffer Overflow](#global-buffer-overflow)
9. [How Buffer Overflow Works](#how-buffer-overflow-works)
10. [Memory Corruption Example](#memory-corruption-example)
11. [What Happens When Code Hits It?](#what-happens-when-code-hits-it)
12. [Real-Time Examples](#real-time-examples)
13. [Buffer Overflow in Embedded Systems](#buffer-overflow-in-embedded-systems)
14. [How to Prevent Buffer Overflow](#how-to-prevent-buffer-overflow)
15. [Compiler Protection Mechanisms](#compiler-protection-mechanisms)
16. [Advantages and Disadvantages](#advantages-and-disadvantages)
17. [Interview Questions](#interview-questions)
18. [Quick Revision](#quick-revision)
19. [Conclusion](#conclusion)

---

# Introduction

A buffer is simply a memory area used to store data.

Example:

```c
char name[10];
```

Memory:

```text
+----+----+----+----+----+
|    |    |    |    |    |
+----+----+----+----+----+
```

The buffer can hold:

```text
10 Characters
```

---

Problem:

If we try to store more than 10 characters:

```text
Memory Outside Buffer Gets Modified
```

This condition is called:

```text
Buffer Overflow
```

---

# What is Buffer Overflow?

## Definition

A Buffer Overflow occurs when a program writes more data into a buffer than the buffer can hold.

---

Example:

```c
char name[5];

strcpy(name,"AUTOMOTIVE");
```

Buffer size:

```text
5 Bytes
```

Data size:

```text
10 Bytes+
```

Overflow occurs.

---

# Why Does Buffer Overflow Occur?

Common reasons:

```text
No Boundary Checking
Unsafe Functions
Invalid Index Access
Wrong Length Calculations
Programming Errors
```

---

Example:

```c
char buffer[10];

buffer[20] = 'A';
```

Writing outside buffer.

---

# How Memory is Organized

Typical Program Memory:

```text
+----------------------+
|        Stack         |
+----------------------+
|        Heap          |
+----------------------+
|        BSS           |
+----------------------+
|       Data           |
+----------------------+
|       Text           |
+----------------------+
```

Buffer overflow can affect:

```text
Stack
Heap
Data
BSS
```

---

# Types of Buffer Overflow

| Type                   | Location          |
| ---------------------- | ----------------- |
| Stack Buffer Overflow  | Stack             |
| Heap Buffer Overflow   | Heap              |
| Global Buffer Overflow | Data/BSS          |
| Integer-Based Overflow | Index Calculation |
| Format String Overflow | String Processing |

---

# Stack Buffer Overflow

Most common interview topic.

Example:

```c
#include <string.h>

void test()
{
    char buffer[8];

    strcpy(buffer,
           "HELLO_WORLD");
}
```

---

Buffer:

```text
8 Bytes
```

Data:

```text
11 Bytes
```

Overflow occurs.

---

Memory:

```text
Before

+------------+
| buffer     |
+------------+
| Return Addr|
+------------+
```

---

After overflow:

```text
+------------+
| Overflow   |
+------------+
| Corrupted  |
+------------+
```

Return address may be overwritten.

---

# Heap Buffer Overflow

Example:

```c
char *ptr =
malloc(8);

strcpy(ptr,
       "HELLO_WORLD");
```

---

Allocated:

```text
8 Bytes
```

Copied:

```text
11 Bytes
```

Overflow affects nearby heap memory.

---

# Global Buffer Overflow

Example:

```c
char buffer[5];

int flag = 1;
```

Memory:

```text
buffer
flag
```

---

Overflow:

```c
strcpy(buffer,
       "ABCDEFGHIJK");
```

may overwrite:

```text
flag
```

---

# How Buffer Overflow Works

Example:

```c
char data[5];
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
strcpy(data,
       "HELLOWORLD");
```

---

Actual Write:

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
1005-1009
```

belong to something else.

Memory corruption occurs.

---

# Memory Corruption Example

```c
#include <stdio.h>
#include <string.h>

int main()
{
    char buffer[5];
    int flag = 100;

    strcpy(buffer,
           "HELLOWORLD");

    printf("%d", flag);
}
```

Possible output:

```text
Random Value
```

because:

```text
flag Corrupted
```

---

# What Happens When Code Hits It?

### Example

```c
char name[5];

strcpy(name,
       "AUTOSAR");
```

---

Step 1

Buffer allocated:

```text
5 Bytes
```

---

Step 2

Function attempts copy:

```text
7 Characters
```

---

Step 3

CPU writes:

```text
A
U
T
O
S
A
R
```

---

Step 4

First 5 bytes fit.

---

Step 5

Remaining bytes overwrite adjacent memory.

---

Step 6

Possible results:

```text
Variable Corruption
Stack Corruption
Program Crash
Undefined Behavior
```

---

### Interview Answer

**What happens when code hits a buffer overflow?**

The CPU does not automatically know the size of a buffer. It continues writing data to consecutive memory locations. If more data is written than the buffer can hold, adjacent memory gets overwritten. This can corrupt variables, stack frames, return addresses, heap metadata, or other critical program data, leading to crashes, unpredictable behavior, or security vulnerabilities.

---

# Real-Time Examples

## Example 1: UART Reception

```c
char rx_buffer[64];
```

Received:

```text
100 Bytes
```

without length checking.

Overflow occurs.

---

## Example 2: CAN Message Processing

```c
uint8_t can_data[8];
```

Incorrect copy:

```c
memcpy(can_data,
       source,
       16);
```

Overflow.

---

## Example 3: GPS Data

```c
char gps_string[32];
```

Large NMEA message arrives.

Buffer overflow occurs.

---

## Example 4: Automotive ECU

Diagnostic packet larger than expected:

```text
UDS Message
```

overwrites memory if validation is missing.

---

# Buffer Overflow in Embedded Systems

Very Important Interview Topic.

---

Common causes:

```text
UART Drivers
SPI Drivers
I2C Drivers
CAN Drivers
DMA Buffers
```

---

Example:

```c
uint8_t uart_rx[128];
```

DMA receives:

```text
200 Bytes
```

Memory corruption occurs.

---

Consequences:

```text
System Crash
Watchdog Reset
Unexpected Behavior
Hard Fault
```

---

# How to Prevent Buffer Overflow

## Use Length Checking

Bad:

```c
strcpy(dest, src);
```

---

Good:

```c
strncpy(dest,
        src,
        sizeof(dest)-1);
```

---

## Validate Array Indices

Bad:

```c
buffer[20] = 1;
```

---

Good:

```c
if(index < BUFFER_SIZE)
{
    buffer[index] = 1;
}
```

---

## Use Safe Functions

Prefer:

```c
snprintf()
memcpy()
with size validation
```

---

## Use Static Analysis

Tools:

* Coverity
* Cppcheck
* Clang Static Analyzer

---

# Compiler Protection Mechanisms

## Stack Canaries

Compiler inserts:

```text
Secret Value
```

between buffer and return address.

---

Example:

```bash
-fstack-protector
```

---

## Address Sanitizer

Compile:

```bash
-fsanitize=address
```

Detects:

```text
Buffer Overflow
Use After Free
Memory Errors
```

---

## MPU/MMU Protection

Embedded systems may use:

```text
Memory Protection Unit (MPU)
Memory Management Unit (MMU)
```

to detect illegal memory access.

---

# Advantages and Disadvantages

## Advantages

Buffer overflow itself has **no advantages**.

Understanding it helps developers:

✔ Write Safer Code

✔ Improve Security

✔ Prevent Crashes

✔ Build Reliable Embedded Systems

---

## Disadvantages

✔ Memory Corruption

✔ System Crashes

✔ Hard Faults

✔ Security Vulnerabilities

✔ Unpredictable Behavior

✔ Data Loss

---

# Interview Questions

## What is a Buffer Overflow?

A buffer overflow occurs when a program writes more data into a buffer than the buffer can hold.

---

## Why Does Buffer Overflow Happen?

Because the CPU does not automatically check buffer boundaries, and the program writes beyond allocated memory.

---

## What is Stack Buffer Overflow?

A stack buffer overflow occurs when data exceeds the size of a stack-allocated array and overwrites nearby stack memory.

---

## What Happens When Code Hits It?

The CPU continues writing beyond the allocated buffer, corrupting adjacent memory and causing undefined behavior.

---

## Why is Buffer Overflow Dangerous?

It can overwrite:

```text
Variables
Pointers
Return Addresses
Heap Metadata
Control Data
```

leading to crashes or security issues.

---

## How Can We Prevent Buffer Overflow?

```text
Bounds Checking
Safe APIs
Input Validation
Static Analysis
Compiler Protection
```

---

## Which Functions Are Commonly Associated with Buffer Overflows?

Unsafe examples:

```c
strcpy()
strcat()
gets()
sprintf()
```

Safer alternatives:

```c
strncpy()
strncat()
fgets()
snprintf()
```

---

# Most Asked Interview Question

## Explain Buffer Overflow with an Example.

Consider:

```c
char buffer[5];

strcpy(buffer,
       "HELLOWORLD");
```

The buffer can hold only 5 characters, but 10 characters are copied. The CPU writes beyond the allocated memory because it does not know the buffer size. This overwrites adjacent memory locations, causing memory corruption, crashes, or security vulnerabilities. Buffer overflows are common causes of embedded system faults and software security issues.

---

# Interview Answer (2-Minute Version)

A buffer overflow occurs when more data is written into a memory buffer than it can hold. Since the CPU does not automatically enforce buffer boundaries, writing beyond the buffer overwrites adjacent memory. This can corrupt variables, stack frames, heap structures, or return addresses, leading to crashes, undefined behavior, and security vulnerabilities. In embedded systems, buffer overflows commonly occur in UART, CAN, SPI, and DMA buffers when input lengths are not validated. Buffer overflows can be prevented using bounds checking, safe string functions, compiler protections, and static analysis tools.

---

# Quick Revision

```text
Buffer Overflow

Definition:
Writing Beyond Buffer Limits

Example:

char buffer[5];

strcpy(buffer,
       "HELLOWORLD");

Result:
Memory Corruption

Types:
✔ Stack Overflow
✔ Heap Overflow
✔ Global Overflow

Causes:
✔ No Bounds Check
✔ Unsafe Functions
✔ Wrong Index

Effects:
✔ Crash
✔ Hard Fault
✔ Data Corruption
✔ Security Issues

Prevention:
✔ strncpy()
✔ snprintf()
✔ Bounds Checking
✔ Static Analysis
✔ Stack Canary

Embedded Systems:
UART
CAN
SPI
DMA

Most Important:
Buffer Overflow =
Writing More Data
Than Buffer Can Hold
```

---

# Conclusion

Buffer overflow is one of the most important software reliability and security concepts in C/C++ programming. It occurs when data exceeds the capacity of a memory buffer, causing memory corruption and unpredictable behavior. Understanding buffer overflows is essential for Embedded Systems, Automotive ECUs, Firmware Development, RTOS programming, Linux systems, Cybersecurity, and technical interviews, where memory safety is a critical topic.
