# Stack Overflow

## Table of Contents

1. [Introduction](#introduction)
2. [What is Stack Overflow?](#what-is-stack-overflow)
3. [Why Does Stack Overflow Occur?](#why-does-stack-overflow-occur)
4. [How the Stack Works](#how-the-stack-works)
5. [Stack Memory Layout](#stack-memory-layout)
6. [Common Causes of Stack Overflow](#common-causes-of-stack-overflow)
7. [Recursion-Based Stack Overflow](#recursion-based-stack-overflow)
8. [Large Local Variable Stack Overflow](#large-local-variable-stack-overflow)
9. [Function Call Stack Growth](#function-call-stack-growth)
10. [Stack Overflow vs Buffer Overflow](#stack-overflow-vs-buffer-overflow)
11. [What Happens When Code Hits It?](#what-happens-when-code-hits-it)
12. [Real-Time Examples](#real-time-examples)
13. [Stack Overflow in Embedded Systems](#stack-overflow-in-embedded-systems)
14. [Detection Methods](#detection-methods)
15. [Prevention Techniques](#prevention-techniques)
16. [Advantages and Disadvantages](#advantages-and-disadvantages)
17. [Interview Questions](#interview-questions)
18. [Quick Revision](#quick-revision)
19. [Conclusion](#conclusion)

---

# Introduction

Every program has a memory area called:

```text
Stack
```

The stack stores:

```text
Function Calls
Return Addresses
Local Variables
Function Parameters
Saved Registers
```

---

Example:

```c
void test()
{
    int a = 10;
}
```

Variable:

```c
a
```

is stored on the stack.

---

Problem:

If stack memory becomes full:

```text
Stack Overflow Occurs
```

---

# What is Stack Overflow?

## Definition

A Stack Overflow occurs when a program uses more stack memory than the allocated stack size.

---

Simple Definition:

> Stack Overflow happens when the call stack grows beyond its maximum limit.

---

Example:

```c
void func()
{
    func();
}
```

Function calls itself forever.

Eventually:

```text
Stack Full
```

---

# Why Does Stack Overflow Occur?

Common reasons:

```text
Infinite Recursion
Deep Recursion
Large Local Variables
Too Many Nested Function Calls
Small Stack Size
```

---

# How the Stack Works

Example:

```c
void A()
{
    B();
}

void B()
{
    C();
}
```

Memory:

```text
+------------+
| Function C |
+------------+
| Function B |
+------------+
| Function A |
+------------+
```

Each function call pushes a new stack frame.

---

# Stack Memory Layout

Typical process memory:

```text
+----------------------+
|        Stack         |
|          ↓           |
+----------------------+
|        Heap          |
|          ↑           |
+----------------------+
|        BSS           |
+----------------------+
|       Data           |
+----------------------+
|       Text           |
+----------------------+
```

---

Important:

```text
Stack grows downward
Heap grows upward
```

---

If they meet:

```text
Memory Corruption
```

---

# Common Causes of Stack Overflow

## 1. Infinite Recursion

Most common interview question.

Example:

```c
void recurse()
{
    recurse();
}
```

---

Execution:

```text
recurse()
    ↓
recurse()
    ↓
recurse()
    ↓
recurse()
```

Never stops.

---

Stack keeps growing until overflow.

---

## 2. Deep Recursion

Example:

```c
int factorial(int n)
{
    if(n == 0)
        return 1;

    return n *
           factorial(n-1);
}
```

---

For:

```c
factorial(100000)
```

stack may overflow.

---

## 3. Large Local Variables

Example:

```c
void process()
{
    char buffer[1000000];
}
```

Large array stored on stack.

---

Memory usage becomes huge.

---

## 4. Excessive Function Calls

Example:

```text
A()
 ↓
B()
 ↓
C()
 ↓
D()
 ↓
E()
```

Thousands of nested calls can exhaust stack memory.

---

# Recursion-Based Stack Overflow

Example:

```c
#include <stdio.h>

void recurse()
{
    printf("Hello\n");

    recurse();
}
```

---

Memory:

```text
Frame 1
Frame 2
Frame 3
Frame 4
Frame 5
...
```

---

Eventually:

```text
No More Stack Space
```

---

Program crashes.

---

# Large Local Variable Stack Overflow

Example:

```c
void task()
{
    int buffer[1000000];
}
```

---

Memory needed:

```text
1000000 × 4

= 4 MB
```

---

If stack size:

```text
1 MB
```

Overflow occurs immediately.

---

# Function Call Stack Growth

Example:

```c
void A(){B();}
void B(){C();}
void C(){D();}
```

---

Memory:

```text
+---------+
| D Frame |
+---------+
| C Frame |
+---------+
| B Frame |
+---------+
| A Frame |
+---------+
```

---

Every frame contains:

```text
Return Address
Parameters
Local Variables
Registers
```

---

# Stack Overflow vs Buffer Overflow

| Feature       | Stack Overflow      | Buffer Overflow       |
| ------------- | ------------------- | --------------------- |
| Cause         | Stack Exhaustion    | Writing Beyond Buffer |
| Memory Issue  | No More Stack Space | Memory Corruption     |
| Common Cause  | Infinite Recursion  | strcpy() Overflow     |
| Result        | Crash               | Crash / Corruption    |
| Security Risk | Moderate            | High                  |

---

# What Happens When Code Hits It?

### Example

```c
void recurse()
{
    recurse();
}
```

---

Step 1

Function called.

```text
Frame 1 Created
```

---

Step 2

Function calls itself.

```text
Frame 2 Created
```

---

Step 3

Again:

```text
Frame 3 Created
```

---

Step 4

Stack grows.

```text
Frame 1000
Frame 1001
Frame 1002
```

---

Step 5

Stack memory exhausted.

---

Step 6

CPU attempts another push.

---

Step 7

OS/RTOS detects invalid memory access.

---

Results:

```text
Segmentation Fault
Hard Fault
Bus Fault
Program Crash
Watchdog Reset
```

---

### Interview Answer

**What happens when code hits a stack overflow?**

Each function call creates a new stack frame containing local variables, return addresses, and saved registers. If function calls continue indefinitely or large amounts of stack memory are used, the stack eventually reaches its limit. Further stack growth accesses invalid memory, causing exceptions, crashes, segmentation faults, or hard faults depending on the platform.

---

# Real-Time Examples

## Example 1: Infinite Recursion

```c
void error()
{
    error();
}
```

Results:

```text
Stack Overflow
```

---

## Example 2: RTOS Task

```c
void Task1()
{
    uint8_t buffer[5000];
}
```

Task stack:

```text
2048 Bytes
```

Overflow occurs.

---

## Example 3: Automotive ECU

Diagnostic handler:

```c
void uds_handler()
{
    uint8_t large_buffer[10000];
}
```

May exceed task stack.

---

## Example 4: Embedded Linux

Recursive directory traversal:

```c
scan_directory()
```

Deep nesting can overflow stack.

---

# Stack Overflow in Embedded Systems

Very Important Interview Topic.

---

Typical RTOS Task:

```text
Task Stack = 1024 Bytes
```

or

```text
Task Stack = 2048 Bytes
```

---

Bad code:

```c
void Task()
{
    uint8_t rx_buffer[5000];
}
```

---

Memory required:

```text
5000 Bytes
```

Stack available:

```text
2048 Bytes
```

Overflow occurs.

---

Common symptoms:

```text
HardFault
Watchdog Reset
Random Crash
Data Corruption
```

---

# Detection Methods

## FreeRTOS

Enable:

```c
configCHECK_FOR_STACK_OVERFLOW
```

---

Functions:

```c
uxTaskGetStackHighWaterMark()
```

Checks remaining stack.

---

## Linux

Command:

```bash
ulimit -s
```

Displays stack size.

---

## Compiler Tools

Use:

```bash
-fstack-protector
```

---

## Static Analysis

Tools:

* Coverity
* Cppcheck
* Clang Static Analyzer

---

# Prevention Techniques

## Avoid Infinite Recursion

Bad:

```c
recurse();
```

---

Good:

```c
if(condition)
{
    recurse();
}
```

---

## Move Large Buffers to Heap

Bad:

```c
char buffer[100000];
```

---

Better:

```c
char *buffer =
malloc(100000);
```

(when appropriate)

---

## Increase Task Stack Size

Example:

```c
xTaskCreate(
    Task,
    "Task",
    4096,
    NULL,
    1,
    NULL);
```

---

## Monitor Stack Usage

Use:

```c
uxTaskGetStackHighWaterMark()
```

---

# Advantages and Disadvantages

## Advantages

Stack overflow itself has **no advantages**.

Understanding it helps:

✔ Prevent Crashes

✔ Design Reliable Systems

✔ Optimize Memory Usage

---

## Disadvantages

✔ Program Crash

✔ Hard Fault

✔ Watchdog Reset

✔ Data Corruption

✔ Unpredictable Behavior

---

# Interview Questions

## What is Stack Overflow?

A stack overflow occurs when a program consumes more stack memory than the allocated stack size.

---

## What Causes Stack Overflow?

```text
Infinite Recursion
Deep Recursion
Large Local Variables
Small Stack Size
```

---

## What Happens When Code Hits It?

The stack grows beyond its allocated memory limit. Subsequent stack operations access invalid memory, causing faults or crashes.

---

## Why Are Embedded Systems More Vulnerable?

Because task stacks are usually very small and memory resources are limited.

---

## Difference Between Stack Overflow and Buffer Overflow?

Stack overflow occurs due to stack exhaustion, while buffer overflow occurs when data is written beyond buffer boundaries.

---

## How Can You Detect Stack Overflow in FreeRTOS?

Using:

```c
configCHECK_FOR_STACK_OVERFLOW

uxTaskGetStackHighWaterMark()
```

---

# Most Asked Interview Question

## Explain Stack Overflow with an Example.

Consider:

```c
void recurse()
{
    recurse();
}
```

Every function call creates a new stack frame. Since there is no termination condition, the recursion continues indefinitely. The stack keeps growing until all stack memory is exhausted. When the CPU attempts to create another stack frame, it accesses invalid memory, causing a crash, segmentation fault, hard fault, or watchdog reset depending on the system.

---

# Interview Answer (2-Minute Version)

A stack overflow occurs when a program exceeds the available stack memory. The stack stores function call information such as return addresses, parameters, and local variables. Common causes include infinite recursion, deep recursion, large local arrays, and insufficient task stack sizes. When the stack becomes full, further function calls or stack operations access invalid memory, resulting in crashes, segmentation faults, hard faults, or watchdog resets. In embedded systems, stack overflow is especially important because RTOS tasks often have limited stack sizes. Proper stack sizing, monitoring, and avoiding excessive recursion help prevent stack overflow issues.

---

# Quick Revision

```text
Stack Overflow

Definition:
Stack Memory Exhausted

Causes:
✔ Infinite Recursion
✔ Deep Recursion
✔ Large Local Variables
✔ Small Stack Size

Memory:

Function Call
      ↓
Stack Frame
      ↓
Stack Full
      ↓
Crash

Embedded Systems:
✔ RTOS Tasks
✔ ISR Functions
✔ Large Buffers

Detection:
✔ FreeRTOS Stack Check
✔ High Water Mark
✔ Static Analysis

Symptoms:
✔ HardFault
✔ Crash
✔ Watchdog Reset

Most Important:
Stack Overflow =
No More Stack Space Available
```

---

# Conclusion

Stack overflow is a critical runtime error that occurs when a program exceeds its available stack memory. It is commonly caused by infinite recursion, deep call chains, or large local variables. In embedded systems and RTOS-based applications, stack overflow is a frequent source of hard faults and system crashes due to limited stack sizes. Understanding stack behavior, stack frame growth, and monitoring techniques is essential for Embedded Systems, Firmware Development, Automotive Software, Linux Programming, and technical interviews.
