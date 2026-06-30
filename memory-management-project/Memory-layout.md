# Memory Layout in C - Complete Guide with Diagrams

## Table of Contents

1. [Introduction](#1-introduction)
2. [Definition](#2-definition)
3. [Why Memory Layout is Important](#3-why-memory-layout-is-important)
4. [Embedded System Memory Architecture](#4-embedded-system-memory-architecture)
5. [Memory Sections Overview](#5-memory-sections-overview)
6. [Text Section (.text)](#6-text-section-text)
7. [Read-Only Data (.rodata)](#7-read-only-data-rodata)
8. [Data Section (.data)](#8-data-section-data)
9. [BSS Section (.bss)](#9-bss-section-bss)
10. [Heap](#10-heap)
11. [Stack](#11-stack)
12. [Complete Memory Map Diagram](#12-complete-memory-map-diagram)
13. [Heap vs Stack Growth Diagram](#13-heap-vs-stack-growth-diagram)
14. [Embedded System Full Execution Flow](#14-embedded-system-full-execution-flow)
15. [Startup Code Flow Diagram](#15-startup-code-flow-diagram)
16. [Variable Storage Diagram](#16-variable-storage-diagram)
17. [Real-Time Embedded Example](#17-real-time-embedded-example)
18. [Comparison Table](#18-comparison-table)
19. [Common Issues with Diagrams](#19-common-issues-with-diagrams)
20. [Real-Time Examples](#20-real-time-examples)
21. [Major Project Usage](#21-major-project-usage)
22. [Best Practices](#22-best-practices)
23. [Quick Revision](#23-quick-revision)
24. [Interview Questions](#24-interview-questions)
25. [Conclusion](#25-conclusion)

---

## 1. Introduction

### What is Memory Layout?

In embedded systems, **memory layout** defines how a program is organized in physical memory (Flash, RAM, registers). Every embedded application runs on a microcontroller where memory is limited and strictly structured.

### Why It Matters

Understanding memory layout is essential for:
- Firmware development
- Debugging crashes
- Optimizing RAM usage
- Writing drivers
- RTOS development
- Linker script understanding

### Simple Definition

```
Memory Layout = How a program is organized in memory
```

---

## 2. Definition

### Formal Definition

Memory layout is the **arrangement of program code, variables, stack, heap, and hardware registers in memory** during execution.

### Simple Definition

```
Memory Layout = Organization of program in memory
```

### Key Points

```
✓ Defines where code is stored
✓ Defines where data is stored
✓ Defines how stack and heap grow
✓ Critical for embedded systems
✓ Controlled by linker script
```

### Types of Memory

```
FLASH/ROM: Non-volatile, read-only
RAM:       Volatile, read-write
Registers: Hardware control
```

---

## 3. Why Memory Layout is Important

### 1. Understand Program Behavior

```
Know where code and data reside
Understand memory usage
Debug memory issues
```

### 2. Optimize Memory Usage

```
Reduce Flash usage
Minimize RAM usage
Improve performance
```

### 3. Debug Crashes

```
Stack overflow → crash
Heap corruption → crash
NULL pointer → crash
```

### 4. Write Linker Scripts

```
Define memory regions
Place sections in memory
Control memory layout
```

### 5. Develop RTOS

```
Task stacks
Heap management
Memory allocation
```

### 6. Embedded Systems

```
Limited memory
Flash and RAM constraints
Real-time requirements
```

---

## 4. Embedded System Memory Architecture

### Complete Memory Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                   FLASH / ROM (Non-Volatile)                │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                  .text (Code)                      │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Interrupt Vector Table                     │  │   │   │
│  │  │  │  Reset Handler                              │  │   │   │
│  │  │  │  Startup Code                               │  │   │   │
│  │  │  │  All Functions (main(), ISRs, etc.)        │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                .rodata (Read-Only)                │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Constants                                  │  │   │   │
│  │  │  │  String Literals                            │  │   │   │
│  │  │  │  const Variables                            │  │   │   │
│  │  │  │  Initial Values for .data                  │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     RAM (Volatile)                           │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                  .data                            │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Initialized Global Variables               │  │   │   │
│  │  │  │  Initialized Static Variables               │  │   │   │
│  │  │  │  Copied from Flash at startup               │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                   .bss                             │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Uninitialized Global Variables            │  │   │   │
│  │  │  │  Uninitialized Static Variables            │  │   │   │
│  │  │  │  Zero-initialized at startup               │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                    HEAP                            │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Dynamic Memory (malloc/free)              │  │   │   │
│  │  │  │  Grows upward →                            │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │                   STACK                            │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Local Variables                            │  │   │   │
│  │  │  │  Function Parameters                        │  │   │   │
│  │  │  │  Return Addresses                           │  │   │   │
│  │  │  │  Saved Registers                            │  │   │   │
│  │  │  │  Grows downward ←                           │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │               PERIPHERAL REGISTERS                          │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  GPIO Registers                                    │   │   │
│  │  │  UART Registers                                    │   │   │
│  │  │  SPI Registers                                     │   │   │
│  │  │  Timer Registers                                   │   │   │
│  │  │  ADC Registers                                     │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Memory Types Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                         MEMORY TYPES                               │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────┐  ┌─────────────────────────┐  │   │
│  │  │       FLASH             │  │         RAM             │  │   │
│  │  │    (Non-Volatile)       │  │      (Volatile)         │  │   │
│  │  ├─────────────────────────┤  ├─────────────────────────┤  │   │
│  │  │  ✓ Holds code           │  │  ✓ Holds variables      │  │   │
│  │  │  ✓ Holds constants      │  │  ✓ Holds stack          │  │   │
│  │  │  ✓ Read-only            │  │  ✓ Read-write           │  │   │
│  │  │  ✓ Retains data         │  │  ✓ Lost on power-off    │  │   │
│  │  │  ✓ Program lifetime     │  │  ✓ Runtime lifetime     │  │   │
│  │  └─────────────────────────┘  └─────────────────────────┘  │   │
│  │                                                             │   │
│  │                    ┌─────────────────────────┐              │   │
│  │                    │   PERIPHERAL REGISTERS  │              │   │
│  │                    ├─────────────────────────┤              │   │
│  │                    │  ✓ Hardware control     │              │   │
│  │                    │  ✓ Memory-mapped I/O    │              │   │
│  │                    │  ✓ Read-write           │              │   │
│  │                    │  ✓ Device-specific      │              │   │
│  │                    └─────────────────────────┘              │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 5. Memory Sections Overview

### Section Summary Table with Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    MEMORY SECTIONS OVERVIEW                        │
│                                                                     │
│  ┌──────────────┬──────────────┬────────────────┬────────────────┐ │
│  │   SECTION    │   LOCATION   │    CONTENT     │   LIFETIME     │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   .text      │   Flash      │    Code        │   Program      │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   .rodata    │   Flash      │    Constants   │   Program      │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   .data      │   RAM        │    Init Globals│   Program      │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   .bss       │   RAM        │    Zero Globals│   Program      │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   Heap       │   RAM        │    Dynamic     │   Runtime      │ │
│  ├──────────────┼──────────────┼────────────────┼────────────────┤ │
│  │   Stack      │   RAM        │    Local vars  │   Function     │ │
│  └──────────────┴──────────────┴────────────────┴────────────────┘ │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Flow of Memory Sections Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     MEMORY SECTION FLOW                            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    SOURCE CODE (.c)                         │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  int global_var = 10;                               │ │   │
│  │  │  int uninit_var;                                    │ │   │
│  │  │  const int MAX = 100;                               │ │   │
│  │  │  void main() { int local = 5; }                    │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    COMPILER                                 │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .text   → Code (main)                             │ │   │
│  │  │  .rodata → MAX = 100                              │ │   │
│  │  │  .data   → global_var = 10                        │ │   │
│  │  │  .bss    → uninit_var                             │ │   │
│  │  │  Stack   → local                                   │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    LINKER                                   │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  Places sections in memory using linker script      │ │   │
│  │  │  .text → Flash                                     │ │   │
│  │  │  .rodata → Flash                                  │ │   │
│  │  │  .data → RAM                                      │ │   │
│  │  │  .bss → RAM                                      │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                EXECUTABLE (.elf / .hex)                     │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  Ready for flashing to microcontroller              │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 6. Text Section (.text)

### Definition

The **.text** section stores executable program instructions (machine code).

### Characteristics

```
✓ Read-only
✓ Stored in Flash/ROM
✓ Contains all functions
✓ Non-volatile
✓ Shared (code can be shared)
```

### .text Section Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                      .text SECTION (FLASH)                         │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000000     │  Interrupt Vector Table                 │   │
│  │  0x08000004     │  Reset Handler Address                  │   │
│  │  0x08000008     │  NMI Handler Address                    │   │
│  │  0x0800000C     │  HardFault Handler Address              │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000100     │  Startup Code                           │   │
│  │  0x08000104     │  - Set stack pointer                   │   │
│  │  0x08000108     │  - Copy .data to RAM                   │   │
│  │  0x0800010C     │  - Zero .bss                           │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000200     │  main() Code                           │   │
│  │  0x08000204     │  - Function prologue                   │   │
│  │  0x08000208     │  - Application logic                  │   │
│  │  0x0800020C     │  - Function epilogue                  │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000300     │  function1() Code                      │   │
│  │  0x08000304     │  - Code body                          │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000400     │  ISR Handlers                         │   │
│  │  0x08000404     │  - UART ISR                           │   │
│  │  0x08000408     │  - Timer ISR                          │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            │  More functions...                     │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: .text is stored in Flash, read-only, non-volatile          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What Goes in .text

```c
void main() {           // Code goes in .text
    int a = 10;
}

void function1() {      // Code goes in .text
    // Function body
}

int add(int x, int y) { // Code goes in .text
    return x + y;
}
```

### Viewing .text Size

```bash
# Check section sizes
arm-none-eabi-size firmware.elf

# Output:
# text    data     bss     dec     hex filename
# 10240   512      256     11008   2b00 firmware.elf
```

---

## 7. Read-Only Data (.rodata)

### Definition

The **.rodata** section stores constant values like strings and const variables.

### Characteristics

```
✓ Stored in Flash
✓ Read-only
✓ Cannot be modified
✓ Non-volatile
✓ Saves RAM
```

### .rodata Section Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                   .rodata SECTION (FLASH)                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08001000     │  const int MAX = 100                    │   │
│  │  0x08001004     │  const float PI = 3.14159              │   │
│  │  0x08001008     │  "Hello World" string                  │   │
│  │  0x08001013     │  "Hello World" (continued)            │   │
│  │  0x08001014     │  const char* msg = "Error"            │   │
│  │  0x0800101A     │  "Error" string                      │   │
│  │  0x0800101E     │  Initial value for global_var = 10   │   │
│  │  0x08001022     │  Initial value for static_var = 20   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            │  More constants...                      │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: .rodata is stored in Flash, read-only, saves RAM           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What Goes in .rodata

```c
const int x = 100;           // Goes in .rodata
const float pi = 3.14159;    // Goes in .rodata
const char* msg = "Hello";   // String goes in .rodata
printf("Hello");             // String in .rodata

// Also:
static const int max_val = 255;  // Goes in .rodata
```

### Benefits of .rodata

```
✓ Saves RAM (stored in Flash)
✓ Protection from modification
✓ Safe for constants
```

---

## 8. Data Section (.data)

### Definition

The **.data** section stores initialized global and static variables.

### Characteristics

```
✓ Stored in RAM (volatile)
✓ Contains initial values
✓ Initialized at startup
✓ Program lifetime
```

### .data Section Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    .data SECTION (RAM)                              │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000000     │  int global_var = 10                    │   │
│  │  0x20000004     │  int count = 0                         │   │
│  │  0x20000008     │  static int static_var = 20            │   │
│  │  0x2000000C     │  char name[10] = "John"               │   │
│  │  0x20000016     │  int array[5] = {1,2,3,4,5}          │   │
│  │  0x2000002A     │  struct config settings = {...}      │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            │  More initialized variables...          │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: .data is stored in RAM, read-write, initialized at startup  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What Goes in .data

```c
int a = 10;              // Goes in .data
int b = 20;              // Goes in .data
static int c = 30;       // Goes in .data
char name[10] = "John";  // Goes in .data

// Also:
int array[5] = {1,2,3,4,5};  // Goes in .data
```

### Initialization Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 .data INITIALIZATION FLOW                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              FLASH (Initial Values)                         │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .rodata Section                                    │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  int a = 10 (initial value)                    │ │ │   │
│  │  │  │  int b = 20 (initial value)                    │ │ │   │
│  │  │  │  int c = 30 (initial value)                    │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              STARTUP CODE (Copies .data)                   │   │
│  │                                                             │   │
│  │  // Copy .data from Flash to RAM                           │   │
│  │  LDR R0, =_sdata                                           │   │
│  │  LDR R1, =_edata                                           │   │
│  │  LDR R2, =_sidata                                          │   │
│  │  copy_loop:                                                │   │
│  │      CMP R0, R1                                            │   │
│  │      BEQ copy_done                                        │   │
│  │      LDR R3, [R2], #4                                     │   │
│  │      STR R3, [R0], #4                                     │   │
│  │      B copy_loop                                          │   │
│  │  copy_done:                                               │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              RAM (Runtime Values)                           │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .data Section                                      │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  int a = 10  (runtime)                        │ │ │   │
│  │  │  │  int b = 20  (runtime)                        │ │ │   │
│  │  │  │  int c = 30  (runtime)                        │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 9. BSS Section (.bss)

### Definition

The **.bss** section stores uninitialized global and static variables (initialized to 0).

### Characteristics

```
✓ Stored in RAM
✓ Automatically zero-initialized
✓ Does not consume Flash space
✓ Program lifetime
```

### .bss Section Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    .bss SECTION (RAM)                              │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000100     │  int uninit_global = 0                 │   │
│  │  0x20000104     │  static int static_uninit = 0          │   │
│  │  0x20000108     │  char buffer[100] = all zeros        │   │
│  │  0x2000016C     │  float readings[10] = all zeros     │   │
│  │  0x20000194     │  struct data info = all zeros       │   │
│  │  0x200001A0     │  uint8_t rx_buffer[256] = zeros    │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            │  More uninitialized variables...        │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: .bss is stored in RAM, zero-initialized at startup          │
│  No Flash space used for .bss!                                    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What Goes in .bss

```c
int x;                   // Goes in .bss
int y;                   // Goes in .bss
static int z;            // Goes in .bss
char buffer[100];        // Goes in .bss

// Also:
float readings[10];      // Goes in .bss
struct data info;        // Goes in .bss
```

### .bss Zero-Initialization Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 .bss ZERO-INITIALIZATION FLOW                      │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              STARTUP CODE (Zero .bss)                      │   │
│  │                                                             │   │
│  │  // Clear .bss section                                     │   │
│  │  LDR R0, =_sbss                                            │   │
│  │  LDR R1, =_ebss                                            │   │
│  │  MOV R2, #0                                                │   │
│  │  clear_loop:                                               │   │
│  │      CMP R0, R1                                            │   │
│  │      BEQ clear_done                                       │   │
│  │      STR R2, [R0], #4                                     │   │
│  │      B clear_loop                                         │   │
│  │  clear_done:                                              │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              RAM (Zero-Initialized Values)                  │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .bss Section                                      │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  int x = 0  (zero)                            │ │ │   │
│  │  │  │  int y = 0  (zero)                            │ │ │   │
│  │  │  │  buffer[100] = all zeros                     │ │ │   │
│  │  │  │  readings[10] = all zeros                    │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. Heap

### Definition

The **heap** is used for dynamic memory allocation (`malloc`, `free`).

### Characteristics

```
✓ Stored in RAM
✓ Grows upward (toward higher addresses)
✓ Programmer-managed
✓ Dynamic allocation
✓ Risk of fragmentation
✓ Risk of memory leaks
```

### Heap Memory Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                      HEAP MEMORY LAYOUT                            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000200     │  Start of Heap                          │   │
│  │  0x20000204     │  ┌─────────────────────────────────────┐ │   │
│  │  0x20000208     │  │  malloc(10) → 10 bytes            │ │   │
│  │  0x20000212     │  │  ptr1 = 0x20000204               │ │   │
│  │  0x20000216     │  ├─────────────────────────────────────┤ │   │
│  │  0x2000021A     │  │  malloc(20) → 20 bytes            │ │   │
│  │  0x2000022E     │  │  ptr2 = 0x20000218               │ │   │
│  │  0x20000232     │  ├─────────────────────────────────────┤ │   │
│  │  0x20000236     │  │  free(ptr1) → memory freed       │ │   │
│  │  0x2000023A     │  │  (may cause fragmentation)        │ │   │
│  │  0x2000023E     │  ├─────────────────────────────────────┤ │   │
│  │  0x20000242     │  │  malloc(15) → 15 bytes            │ │   │
│  │  0x20000251     │  │  ptr3 = 0x20000240               │ │   │
│  │  0x20000255     │  └─────────────────────────────────────┘ │   │
│  │  ...            │  More heap allocations...              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: Heap grows upward, managed by malloc/free                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What Goes in Heap

```c
int *ptr = (int*)malloc(10 * sizeof(int));  // Allocates heap
char *str = (char*)malloc(50);              // Allocates heap
struct data *info = malloc(sizeof(struct data));  // Allocates heap
```

### Heap Operations Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    HEAP OPERATIONS                                 │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           malloc(size)                             │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  1. Find free block of size bytes          │  │   │   │
│  │  │  │  2. Mark as used                           │  │   │   │
│  │  │  │  3. Return pointer to allocated memory    │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           free(ptr)                               │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  1. Mark memory as free                    │  │   │   │
│  │  │  │  2. Merge with adjacent free blocks        │  │   │   │
│  │  │  │  3. Return to heap pool                    │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           realloc(ptr, new_size)                  │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  1. Allocate new block of new_size         │  │   │   │
│  │  │  │  2. Copy data from old block               │  │   │   │
│  │  │  │  3. Free old block                         │  │   │   │
│  │  │  │  4. Return pointer to new block            │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Embedded Systems Note

Heap is often avoided in real-time systems because:
- Unpredictable timing
- Fragmentation
- Memory leaks
- Hard real-time constraints

---

## 11. Stack

### Definition

The **stack** is used for function calls, local variables, and return addresses.

### Characteristics

```
✓ Stored in RAM
✓ LIFO (Last In First Out) structure
✓ Automatically managed
✓ Fast access
✓ Limited size
✓ Grows downward
```

### Stack Memory Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     STACK MEMORY LAYOUT                            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS         CONTENT                                   │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x2000FFFC     │  Top of Stack (High Address)            │   │
│  │  0x2000FFF8     │  ┌─────────────────────────────────────┐ │   │
│  │  0x2000FFF4     │  │  main() Stack Frame               │ │   │
│  │  0x2000FFF0     │  │  ─────────────────────────────    │ │   │
│  │  0x2000FFEC     │  │  Return Address                   │ │   │
│  │  0x2000FFE8     │  │  Saved Registers                 │ │   │
│  │  0x2000FFE4     │  │  local_var = 5                   │ │   │
│  │  0x2000FFE0     │  ├─────────────────────────────────────┤ │   │
│  │  0x2000FFDC     │  │  function1() Stack Frame          │ │   │
│  │  0x2000FFD8     │  │  ─────────────────────────────    │ │   │
│  │  0x2000FFD4     │  │  Return Address                   │ │   │
│  │  0x2000FFD0     │  │  Saved Registers                 │ │   │
│  │  0x2000FFCC     │  │  local = 10                      │ │   │
│  │  0x2000FFC8     │  ├─────────────────────────────────────┤ │   │
│  │  0x2000FFC4     │  │  function2() Stack Frame          │ │   │
│  │  0x2000FFC0     │  │  ─────────────────────────────    │ │   │
│  │  0x2000FFBC     │  │  Return Address                   │ │   │
│  │  0x2000FFB8     │  │  Saved Registers                 │ │   │
│  │  0x2000FFB4     │  │  x = 20                          │ │   │
│  │  0x2000FFB0     │  │  y = 30                          │ │   │
│  │  0x2000FFAC     │  └─────────────────────────────────────┘ │   │
│  │  0x2000FFA8     │  Bottom of Stack (Low Address)         │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Note: Stack grows downward from high to low addresses             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Stack Frame Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                      STACK FRAME                                   │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           High Address                             │   │   │
│  │  │  ┌───────────────────────────────────────────────┐ │   │   │
│  │  │  │  Previous Stack Frame                       │ │   │   │
│  │  │  ├───────────────────────────────────────────────┤ │   │   │
│  │  │  │  Return Address                            │ │   │   │
│  │  │  ├───────────────────────────────────────────────┤ │   │   │
│  │  │  │  Saved Registers (R0-R12, LR)              │ │   │   │
│  │  │  ├───────────────────────────────────────────────┤ │   │   │
│  │  │  │  Local Variables                           │ │   │   │
│  │  │  │  ───────────────────────────────────────── │ │   │   │
│  │  │  │  int local1 = 10                         │ │   │   │
│  │  │  │  int local2 = 20                         │ │   │   │
│  │  │  │  char buf[10]                            │ │   │   │
│  │  │  ├───────────────────────────────────────────────┤ │   │   │
│  │  │  │  Temporary Variables                        │ │   │   │
│  │  │  ├───────────────────────────────────────────────┤ │   │   │
│  │  │  │  Stack Pointer (SP)                         │ │   │   │
│  │  │  └───────────────────────────────────────────────┘ │   │   │
│  │  │           Low Address                             │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Stack Operations Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    STACK OPERATIONS                                │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           PUSH Operation                           │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  SP = SP - 4                              │  │   │   │
│  │  │  │  [SP] = Value                              │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           POP Operation                           │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  Value = [SP]                             │  │   │   │
│  │  │  │  SP = SP + 4                              │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           Function Call (CALL)                     │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  1. Push Return Address                    │  │   │   │
│  │  │  │  2. Jump to Function                      │  │   │   │
│  │  │  │  3. Create Stack Frame                    │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │           Function Return (RET)                    │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  1. Pop Return Address                     │  │   │   │
│  │  │  │  2. Restore Saved Registers               │  │   │   │
│  │  │  │  3. Jump to Return Address                 │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 12. Complete Memory Map Diagram

### Full Memory Map

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    COMPLETE MEMORY MAP                             │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  ADDRESS        | SECTION    | CONTENT                     │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000000     |            |                             │   │
│  │  │              |            |  Interrupt Vector Table    │   │
│  │  0x08000004     |  .text     |  Reset Handler              │   │
│  │  0x08000008     |            |  NMI Handler               │   │
│  │  0x0800000C     |            |  HardFault Handler         │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000100     |  .text     |  Startup Code               │   │
│  │  0x08000104     |            |  - Set SP                  │   │
│  │  0x08000108     |            |  - Copy .data             │   │
│  │  0x0800010C     |            |  - Zero .bss              │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08000200     |  .text     |  main() Code               │   │
│  │  0x08000204     |            |  function1() Code          │   │
│  │  0x08000208     |            |  function2() Code          │   │
│  │  ...            |            |  More functions...         │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x08001000     |  .rodata   |  const int MAX = 100      │   │
│  │  0x08001004     |            |  const float PI = 3.14    │   │
│  │  0x08001008     |            |  "Hello World" string     │   │
│  │  0x08001010     |            |  Initial values for .data │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            |            |                             │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000000     |  .data     |  int global_var = 10      │   │
│  │  0x20000004     |            |  int count = 0            │   │
│  │  0x20000008     |            |  static int s_var = 20    │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000100     |  .bss      |  int uninit_global = 0    │   │
│  │  0x20000104     |            |  char buffer[100] = 0     │   │
│  │  0x20000168     |            |  static int s_uninit = 0  │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x20000200     |  HEAP      |  Dynamic Memory            │   │
│  │  │              |  (grows    |  malloc/free              │   │
│  │  │              |   up)      |  ──────────────────────── │   │
│  │  │              |            |  ptr1 (10 bytes)          │   │
│  │  │              |            |  ptr2 (20 bytes)          │   │
│  │  │              |            |  ptr3 (15 bytes)          │   │
│  │  ...            |            |  ...                      │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  ...            |  STACK     |  ──────────────────────── │   │
│  │  │              |  (grows    |  Function Calls           │   │
│  │  │              |   down)    |  Local Variables         │   │
│  │  0x2000FF00     |            |  Return Addresses        │   │
│  │  0x2000F000     |            |  Saved Registers         │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │  0x40000000     |  PERIPH    |  GPIO Registers            │   │
│  │  0x40001000     |  REGISTERS |  UART Registers            │   │
│  │  0x40002000     |            |  SPI Registers             │   │
│  │  0x40003000     |            |  Timer Registers           │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 13. Heap vs Stack Growth Diagram

### Memory Growth Direction

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                   HEAP vs STACK GROWTH                             │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    HIGH ADDRESS                             │   │
│  │                                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │                     STACK                            │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐  │ │   │
│  │  │  │  Stack Frame 3  (function3)                   │  │ │   │
│  │  │  ├─────────────────────────────────────────────────┤  │ │   │
│  │  │  │  Stack Frame 2  (function2)                   │  │ │   │
│  │  │  ├─────────────────────────────────────────────────┤  │ │   │
│  │  │  │  Stack Frame 1  (function1)                   │  │ │   │
│  │  │  ├─────────────────────────────────────────────────┤  │ │   │
│  │  │  │  Stack Frame 0  (main)                        │  │ │   │
│  │  │  └─────────────────────────────────────────────────┘  │ │   │
│  │  │                   ↓ Grows Down                       │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │              FREE MEMORY                             │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐  │ │   │
│  │  │  │  Available for heap or stack growth           │  │ │   │
│  │  │  └─────────────────────────────────────────────────┘  │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │              HEAP (Grows Up)                        │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐  │ │   │
│  │  │  │  malloc(15) → 15 bytes                        │  │ │   │
│  │  │  ├─────────────────────────────────────────────────┤  │ │   │
│  │  │  │  malloc(20) → 20 bytes                        │  │ │   │
│  │  │  ├─────────────────────────────────────────────────┤  │ │   │
│  │  │  │  malloc(10) → 10 bytes                        │  │ │   │
│  │  │  └─────────────────────────────────────────────────┘  │ │   │
│  │  │                   ↑ Grows Up                         │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .bss Section                                       │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .data Section                                      │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │                    LOW ADDRESS                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Important: Stack and Heap grow toward each other!                │
│  If they meet → Stack overflow or Heap corruption                │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Stack vs Heap Comparison Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                  STACK vs HEAP COMPARISON                         │
│                                                                     │
│  ┌────────────────────────────────┬────────────────────────────────┐│
│  │           STACK               │           HEAP                 ││
│  ├────────────────────────────────┼────────────────────────────────┤│
│  │  LIFO (Last In First Out)     │  Free Allocation              ││
│  │  Automatic Management         │  Manual Management            ││
│  │  Very Fast Access             │  Slower Access                ││
│  │  Fixed Size                   │  Dynamic Size                 ││
│  │  No Fragmentation             │  Can Fragment                 ││
│  │  Limited Size (few KB)        │  Larger Size                  ││
│  │  Grows Downward               │  Grows Upward                 ││
│  │  Local Variables              │  malloc/free                  ││
│  │  Function Parameters          │  Dynamic Arrays               ││
│  │  Return Addresses             │  Complex Data Structures      ││
│  └────────────────────────────────┴────────────────────────────────┘│
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     KEY DIFFERENCES                        │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │                                                             │   │
│  │  Memory Management:                                        │   │
│  │  Stack:  Automatic (push/pop on function call/return)     │   │
│  │  Heap:   Manual (malloc/free)                             │   │
│  │                                                             │   │
│  │  Speed:                                                    │   │
│  │  Stack:  Very Fast (just pointer increment/decrement)     │   │
│  │  Heap:   Slower (requires searching for free blocks)      │   │
│  │                                                             │   │
│  │  Lifetime:                                                 │   │
│  │  Stack:  Function lifetime (exits when function returns)  │   │
│  │  Heap:   Programmer controlled (exists until free)        │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 14. Embedded System Full Execution Flow

### Complete Boot Sequence

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 EMBEDDED SYSTEM EXECUTION FLOW                     │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    POWER ON / RESET                        │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  CPU starts executing from reset address             │ │   │
│  │  │  Program counter set to 0x00000000                  │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                1. VECTOR TABLE FETCH                       │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  Read reset handler address from vector table        │ │   │
│  │  │  Vector Table at 0x08000000                          │ │   │
│  │  │  Reset Handler at 0x08000100                        │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                2. STARTUP CODE (Assembly)                  │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  a. Set Stack Pointer (SP)                         │ │   │
│  │  │     LDR SP, =_estack                              │ │   │
│  │  │  b. Copy .data from Flash to RAM                   │ │   │
│  │  │     - Load initial values from Flash              │ │   │
│  │  │     - Store to RAM location                       │ │   │
│  │  │  c. Zero .bss section                             │ │   │
│  │  │     - Fill with zeros                             │ │   │
│  │  │  d. Initialize C runtime                          │ │   │
│  │  │     - Set up static constructors (C++)           │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                3. CALL main()                              │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  BL main                                            │ │   │
│  │  │  Branch to main function                           │ │   │
│  │  │  Control transfers to C code                      │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └──────────────────────────────┬──────────────────────────────┘   │
│                                 ↓                                  │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                4. APPLICATION RUNS                        │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  • Execute main() code                             │ │   │
│  │  │  • Use .data and .bss variables                    │ │   │
│  │  │  • Allocate heap memory (if used)                  │ │   │
│  │  │  • Function calls use stack                       │ │   │
│  │  │  • ISRs handle interrupts                          │ │   │
│  │  │  • while(1) loop runs forever                     │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Execution Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    EXECUTION FLOW DIAGRAM                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │     POWER ON     │                  │   │
│  │                      └─────────┬─────────┘                  │   │
│  │                                ↓                            │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │  VECTOR TABLE    │                  │   │
│  │                      │  (0x08000000)   │                  │   │
│  │                      └─────────┬─────────┘                  │   │
│  │                                ↓                            │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │   RESET HANDLER  │                  │   │
│  │                      │  (0x08000100)   │                  │   │
│  │                      └─────────┬─────────┘                  │   │
│  │                                ↓                            │   │
│  │                ┌───────────────┴───────────────┐            │   │
│  │                ↓                               ↓            │   │
│  │      ┌─────────────────┐             ┌─────────────────┐   │   │
│  │      │   Copy .data   │             │   Zero .bss    │   │   │
│  │      │   Flash → RAM  │             │   to 0         │   │   │
│  │      └─────────────────┘             └─────────────────┘   │   │
│  │                ↓                               ↓            │   │
│  │                └───────────────┬───────────────┘            │   │
│  │                                ↓                            │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │   CALL main()    │                  │   │
│  │                      └─────────┬─────────┘                  │   │
│  │                                ↓                            │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │   main() Runs   │                  │   │
│  │                      │   Application   │                  │   │
│  │                      └─────────┬─────────┘                  │   │
│  │                                ↓                            │   │
│  │                      ┌───────────────────┐                  │   │
│  │                      │   while(1)      │                  │   │
│  │                      │   Main Loop     │                  │   │
│  │                      └───────────────────┘                  │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 15. Startup Code Flow Diagram

### Detailed Startup Code

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                  STARTUP CODE FLOW DIAGRAM                        │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  1. Set Stack Pointer                               │   │   │
│  │  │  ────────────────────────────────────────────────── │   │   │
│  │  │  LDR R0, =_estack                                  │   │   │
│  │  │  MOV SP, R0                                        │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                        ↓                                  │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  2. Copy .data from Flash to RAM                   │   │   │
│  │  │  ────────────────────────────────────────────────── │   │   │
│  │  │  LDR R0, =_sdata   ; Source in Flash              │   │   │
│  │  │  LDR R1, =_edata   ; End of .data                │   │   │
│  │  │  LDR R2, =_sidata  ; Initial values              │   │   │
│  │  │                                                   │   │   │
│  │  │  copy_data:                                       │   │   │
│  │  │      CMP R0, R1                                   │   │   │
│  │  │      BEQ copy_done                               │   │   │
│  │  │      LDR R3, [R2], #4                            │   │   │
│  │  │      STR R3, [R0], #4                            │   │   │
│  │  │      B copy_data                                 │   │   │
│  │  │  copy_done:                                      │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                        ↓                                  │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  3. Zero .bss Section                             │   │   │
│  │  │  ────────────────────────────────────────────────── │   │   │
│  │  │  LDR R0, =_sbss                                   │   │   │
│  │  │  LDR R1, =_ebss                                   │   │   │
│  │  │  MOV R2, #0                                       │   │   │
│  │  │                                                   │   │   │
│  │  │  zero_bss:                                        │   │   │
│  │  │      CMP R0, R1                                   │   │   │
│  │  │      BEQ zero_done                               │   │   │
│  │  │      STR R2, [R0], #4                            │   │   │
│  │  │      B zero_bss                                  │   │   │
│  │  │  zero_done:                                      │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                        ↓                                  │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  4. Call main()                                   │   │   │
│  │  │  ────────────────────────────────────────────────── │   │   │
│  │  │  BL main                                          │   │   │
│  │  │                                                   │   │   │
│  │  │  5. Infinite Loop (if main returns)              │   │   │
│  │  │  B .                                              │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 16. Variable Storage Diagram

### Variable Storage Classes

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 VARIABLE STORAGE DIAGRAM                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  GLOBAL VARIABLES                                  │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │                                                    │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  int global_init = 10;  → .data           │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  int global_uninit;         → .bss          │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  const int MAX = 100;      → .rodata       │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  STATIC VARIABLES                                 │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │                                                    │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  static int s_init = 20;    → .data        │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  static int s_uninit;        → .bss        │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  LOCAL VARIABLES                                  │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │                                                    │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  void func() {                             │  │   │   │
│  │  │  │      int local = 5;     → STACK            │  │   │   │
│  │  │  │      static int s_var;  → .bss            │  │   │   │
│  │  │  │  }                                        │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  DYNAMIC VARIABLES                                 │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │                                                    │   │   │
│  │  │  ┌─────────────────────────────────────────────┐  │   │   │
│  │  │  │  int *ptr = malloc(10 * sizeof(int));     │  │   │   │
│  │  │  │              → HEAP                        │  │   │   │
│  │  │  └─────────────────────────────────────────────┘  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Variable Lifetime Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                  VARIABLE LIFETIME DIAGRAM                         │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  SECTION    │ LIFETIME     │ SCOPE                 │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │  .data      │ Program     │ Global/File            │   │   │
│  │  │  .bss       │ Program     │ Global/File            │   │   │
│  │  │  .rodata    │ Program     │ Global/File            │   │   │
│  │  │  Stack      │ Function    │ Function               │   │   │
│  │  │  Heap       │ Runtime     │ Programmer controlled  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  VARIABLE TYPE   │ SECTION  │ LIFETIME              │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │  Global init     │ .data    │ Program               │   │   │
│  │  │  Global uninit   │ .bss     │ Program               │   │   │
│  │  │  const global    │ .rodata  │ Program               │   │   │
│  │  │  static init     │ .data    │ Program               │   │   │
│  │  │  static uninit   │ .bss     │ Program               │   │   │
│  │  │  local           │ Stack    │ Function              │   │   │
│  │  │  const local     │ Stack    │ Function              │   │   │
│  │  │  malloc()        │ Heap     │ Until free()          │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 17. Real-Time Embedded Example

### Complete Program with Memory Layout

```c
#include <stdio.h>
#include <stdlib.h>

// .bss (uninitialized global)
int global_uninit;

// .data (initialized global)
int global_init = 100;

// .rodata (constant)
const int const_val = 50;

// .text (function code)
void function1() {
    // Stack (local variables)
    int local = 10;
    static int static_local = 20;  // .data
}

// .text (function code)
int main() {
    // Stack (local variables)
    int local_var = 5;
    
    // Heap (dynamic memory)
    int *ptr = (int*)malloc(10 * sizeof(int));
    if(ptr != NULL) {
        ptr[0] = 100;
        free(ptr);
    }
    
    function1();
    
    return 0;
}
```

### Memory Layout of Example

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│              MEMORY LAYOUT OF EXAMPLE PROGRAM                      │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │  SECTION    │ VARIABLE        │ LOCATION  │ VALUE   │   │   │
│  │  ├─────────────────────────────────────────────────────┤   │   │
│  │  │  .text     │ main()          │ Flash     │ Code    │   │   │
│  │  │  .text     │ function1()     │ Flash     │ Code    │   │   │
│  │  │  .rodata   │ const_val       │ Flash     │ 50      │   │   │
│  │  │  .data     │ global_init     │ RAM       │ 100     │   │   │
│  │  │  .data     │ static_local    │ RAM       │ 20      │   │   │
│  │  │  .bss      │ global_uninit   │ RAM       │ 0       │   │   │
│  │  │  Stack     │ local_var       │ RAM       │ 5       │   │   │
│  │  │  Stack     │ local           │ RAM       │ 10      │   │   │
│  │  │  Heap      │ ptr allocation  │ RAM       │ Dynamic │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Memory Map Visualization

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                  MEMORY MAP VISUALIZATION                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  Flash:                                                    │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .text                                               │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  main() code                                   │ │ │   │
│  │  │  │  function1() code                              │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  ├───────────────────────────────────────────────────────┤ │   │
│  │  │  .rodata                                            │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  const_val = 50                                │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  RAM:                                                      │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  .data                                               │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  global_init = 100                             │ │ │   │
│  │  │  │  static_local = 20                             │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  ├───────────────────────────────────────────────────────┤ │   │
│  │  │  .bss                                                │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  global_uninit = 0                             │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  ├───────────────────────────────────────────────────────┤ │   │
│  │  │  Heap (grows up)                                     │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  ptr allocation (10 bytes)                     │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  ├───────────────────────────────────────────────────────┤ │   │
│  │  │  Stack (grows down)                                  │ │   │
│  │  │  ┌─────────────────────────────────────────────────┐ │ │   │
│  │  │  │  main() frame: local_var = 5                  │ │ │   │
│  │  │  │  function1() frame: local = 10               │ │ │   │
│  │  │  └─────────────────────────────────────────────────┘ │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 18. Comparison Table

### Memory Sections Comparison

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 MEMORY SECTIONS COMPARISON                         │
│                                                                     │
│  ┌──────────────┬──────────────┬──────────────┬──────────────────┐ │
│  │   SECTION    │   MEMORY    │   LIFETIME   │   MANAGED BY     │ │
│  ├──────────────┼──────────────┼──────────────┼──────────────────┤ │
│  │   .text      │   Flash     │   Program    │   Compiler       │ │
│  │   .rodata    │   Flash     │   Program    │   Compiler       │ │
│  │   .data      │   RAM       │   Program    │   Startup Code   │ │
│  │   .bss       │   RAM       │   Program    │   Startup Code   │ │
│  │   Heap       │   RAM       │   Runtime    │   Programmer     │ │
│  │   Stack      │   RAM       │   Function   │   Compiler       │ │
│  └──────────────┴──────────────┴──────────────┴──────────────────┘ │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │  SECTION    │ VOLATILE    │ READ/WRITE  │ CONTENT            ││
│  ├──────────────┼──────────────┼──────────────┼──────────────────┤│
│  │  .text      │   No        │   Read-only │   Code            ││
│  │  .rodata    │   No        │   Read-only │   Constants       ││
│  │  .data      │   Yes       │   Read-Write│   Init Globals    ││
│  │  .bss       │   Yes       │   Read-Write│   Zero Globals    ││
│  │  Heap       │   Yes       │   Read-Write│   Dynamic         ││
│  │  Stack      │   Yes       │   Read-Write│   Locals          ││
│  └──────────────┴──────────────┴──────────────┴──────────────────┘│
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Variable Storage Classes Table

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│               VARIABLE STORAGE CLASSES TABLE                       │
│                                                                     │
│  ┌────────────────┬──────────────┬──────────────┬────────────────┐ │
│  │   VARIABLE     │   SECTION    │   LIFETIME   │   SCOPE        │ │
│  ├────────────────┼──────────────┼──────────────┼────────────────┤ │
│  │ Global init    │   .data      │   Program    │   All files    │ │
│  │ Global uninit  │   .bss       │   Program    │   All files    │ │
│  │ Static init    │   .data      │   Program    │   File/Func    │ │
│  │ Static uninit  │   .bss       │   Program    │   File/Func    │ │
│  │ Local          │   Stack      │   Function   │   Function     │ │
│  │ const global   │   .rodata    │   Program    │   All files    │ │
│  │ const local    │   Stack      │   Function   │   Function     │ │
│  │ malloc()       │   Heap       │   Runtime    │   Global       │ │
│  └────────────────┴──────────────┴──────────────┴────────────────┘ │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 19. Common Issues with Diagrams

### 1. Stack Overflow Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                    STACK OVERFLOW DIAGRAM                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  Normal Operation:                                         │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  High Address │  Stack (grows down)                │ │   │
│  │  │               │  ─────────────────────────────     │ │   │
│  │  │               │  Stack Frame 3                    │ │   │
│  │  │               │  Stack Frame 2                    │ │   │
│  │  │               │  Stack Frame 1                    │ │   │
│  │  │  Low Address  │  Heap (grows up)                 │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  Stack Overflow:                                           │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  High Address │  Stack (grows down)                │ │   │
│  │  │               │  ─────────────────────────────     │ │   │
│  │  │               │  Stack Frame 3                    │ │   │
│  │  │               │  Stack Frame 2                    │ │   │
│  │  │               │  Stack Frame 1                    │ │   │
│  │  │               │  Stack Frame 0                    │ │   │
│  │  │               │  ❌ STACK COLLISION! ❌           │ │   │
│  │  │  Low Address  │  Heap                            │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  Causes:                                                   │   │
│  │  • Deep recursion                                         │   │
│  │  • Large local arrays                                     │   │
│  │  • Too many function calls                               │   │
│  │  • Stack size too small                                  │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 2. Heap Fragmentation Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│               HEAP FRAGMENTATION DIAGRAM                          │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  Initial Heap:                                             │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  [FREE MEMORY]                                      │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  After Allocations:                                        │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  [A: 10]  [B: 20]  [C: 15]  [D: 25]               │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  After Free(B):                                            │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  [A: 10]  [FREE]  [C: 15]  [D: 25]                 │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  After Free(D):                                            │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  [A: 10]  [FREE]  [C: 15]  [FREE]                  │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  │  Fragmentation:                                            │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │  [A: 10]  [FREE:5]  [C:15]  [FREE:8]              │ │   │
│  │  │           (too small to use)                        │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 20. Real-Time Examples

### Example 1: STM32 Memory Layout

```c
// STM32F4 Memory Map
// Flash: 0x08000000 - 0x080FFFFF (1MB)
// RAM:   0x20000000 - 0x2001FFFF (128KB)

int global_var = 10;        // .data in RAM
int uninit_var;             // .bss in RAM
const int MAX = 100;        // .rodata in Flash

void function1() {
    int local = 5;          // Stack in RAM
    static int static_var;  // .bss in RAM
}
```

### Example 2: Memory Usage Check

```bash
# Check memory usage
arm-none-eabi-size firmware.elf

# Output:
# text    data     bss     dec     hex filename
# 10240   512      256     11008   2b00 firmware.elf

# Breakdown:
# text:  10240 bytes (Flash)
# data:  512 bytes  (RAM)
# bss:   256 bytes  (RAM)
# Total RAM: 768 bytes
# Total Flash: 10240 bytes
```

### Example 3: Linker Script Snippet

```ld
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    .text :
    {
        *(.text)
        *(.text.*)
    } > FLASH

    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    } > FLASH

    .data :
    {
        *(.data)
        *(.data.*)
    } > RAM AT> FLASH

    .bss :
    {
        *(.bss)
        *(.bss.*)
    } > RAM
}
```

---

## 21. Major Project Usage

### Operating Systems

```
Linux Kernel:
- .text:  Boot code, kernel code
- .data:  Kernel data structures
- .bss:   Zero-initialized data
- Stack:  Process stacks
- Heap:   Dynamic allocation
```

### Embedded RTOS

```
FreeRTOS:
- .text:  RTOS code
- .data:  RTOS data
- .bss:   Zero-initialized data
- Stack:  Task stacks
- Heap:   Dynamic allocation (optional)
```

### Firmware

```
STM32 Firmware:
- .text:  Application code
- .rodata: Constants, strings
- .data:  Global variables
- .bss:   Zero-initialized variables
- Stack:  Main stack
- Heap:   malloc (if used)
```

---

## 22. Best Practices

### 1. Minimize RAM Usage

```
✓ Use const for constants (.rodata)
✓ Reduce global variables
✓ Use static where possible
✓ Avoid large local arrays
✓ Use malloc sparingly
```

### 2. Optimize Stack Usage

```
✓ Minimize local variables
✓ Avoid deep recursion
✓ Reduce large local arrays
✓ Set appropriate stack size
✓ Monitor stack usage
```

### 3. Avoid Heap in RTOS

```
✓ Static allocation
✓ Pool allocator
✓ Fixed-size buffers
✓ Avoid fragmentation
✓ No malloc in ISR
```

### 4. Use const Correctly

```c
// Good - saves RAM
const int MAX = 100;      // .rodata
const char* msg = "Hello"; // .rodata

// Good for arrays
const uint8_t data[100] = { ... };  // .rodata
```

### 5. Linker Script Customization

```
✓ Define memory regions
✓ Place sections appropriately
✓ Set stack size
✓ Set heap size
✓ Generate map file
```

---

## 23. Quick Revision

### Memory Sections Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                 MEMORY LAYOUT QUICK REFERENCE                      │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  FLASH:                                                    │   │
│  │  .text     → Code (functions, main())                     │   │
│  │  .rodata   → Constants, strings                           │   │
│  │                                                             │   │
│  │  RAM:                                                      │   │
│  │  .data     → Initialized globals/static variables        │   │
│  │  .bss      → Zero-initialized globals/static variables   │   │
│  │  Heap      → Dynamic memory (malloc/free)               │   │
│  │  Stack     → Local variables, function calls            │   │
│  │                                                             │   │
│  │  Startup Flow:                                             │   │
│  │  1. Copy .data from Flash to RAM                          │   │
│  │  2. Zero .bss                                            │   │
│  │  3. Initialize stack                                     │   │
│  │  4. Call main()                                          │   │
│  │                                                             │   │
│  │  Common Issues:                                            │   │
│  │  • Stack overflow                                         │   │
│  │  • Heap fragmentation                                     │   │
│  │  • Memory corruption                                      │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Variable Storage Quick Reference

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│              VARIABLE STORAGE QUICK REFERENCE                      │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │  Variable Type          →  Section                         │   │
│  │  ─────────────────────────────────────────────────────────  │   │
│  │  Global init            →  .data                          │   │
│  │  Global uninit          →  .bss                           │   │
│  │  Static init            →  .data                          │   │
│  │  Static uninit          →  .bss                           │   │
│  │  Local                  →  Stack                          │   │
│  │  const global           →  .rodata                        │   │
│  │  const local            →  Stack                          │   │
│  │  malloc()               →  Heap                           │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 24. Interview Questions

### Basic Questions

#### Q1: What is memory layout?
**Answer:** Memory layout defines how program code and data are organized in memory during execution.

#### Q2: What sections are in C memory layout?
**Answer:** .text, .rodata, .data, .bss, heap, stack.

#### Q3: What is stored in .text section?
**Answer:** Executable code (functions, main()).

#### Q4: What is stored in .data section?
**Answer:** Initialized global and static variables.

### Intermediate Questions

#### Q5: Difference between .data and .bss?
**Answer:** .data has initial values; .bss is zero-initialized.

#### Q6: Why .bss doesn't consume Flash space?
**Answer:** Only size is stored, not initial values (all zeros).

#### Q7: What is stored in stack?
**Answer:** Local variables, function parameters, return addresses.

#### Q8: What is stored in heap?
**Answer:** Dynamically allocated memory (malloc/free).

### Advanced Questions

#### Q9: How does startup code initialize memory?
**Answer:** Copies .data from Flash to RAM, zeros .bss.

#### Q10: What happens when stack and heap collide?
**Answer:** Memory corruption, hard fault, program crash.

#### Q11: Why avoid heap in embedded systems?
**Answer:** Fragmentation, unpredictable timing, memory leaks.

#### Q12: How to check memory usage?
**Answer:** Use size tool, map files, linker output.

---

## 25. Conclusion

### Summary

Memory layout is a fundamental concept in C programming, especially for embedded systems. Understanding how code and data are organized in memory is essential for:

```
✓ Writing efficient firmware
✓ Debugging crashes
✓ Optimizing resource usage
✓ Developing RTOS
✓ Writing linker scripts
```

### Key Takeaways

```
✓ .text and .rodata in Flash
✓ .data, .bss, heap, stack in RAM
✓ .data copied at startup
✓ .bss zeroed at startup
✓ Stack and heap grow toward each other
✓ Embedded: avoid heap
```

### Memory Usage Tips

```
✓ Use const for read-only data
✓ Minimize global variables
✓ Avoid recursion
✓ Use static allocation
✓ Monitor stack usage
```

### Learning Path

```
C Programming
    ↓
Variables and Storage
    ↓
Memory Layout
    ↓
Linker Scripts
    ↓
Startup Code
    ↓
Embedded Systems
    ↓
RTOS
```

### Final Thought

Memory layout is critical for embedded systems programming. Understanding where code, data, stack, and heap reside helps you write more efficient, reliable, and robust firmware. Always consider memory constraints when designing embedded applications.

---

*End of Document*
