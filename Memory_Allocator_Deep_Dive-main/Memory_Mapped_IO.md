# Memory-Mapped I/O (MMIO) in Embedded Systems

## Table of Contents

1. [Introduction](#introduction)
2. [What is Memory-Mapped I/O?](#what-is-memory-mapped-io)
3. [Why do we use MMIO?](#why-do-we-use-mmio)
4. [How MMIO works](#how-mmio-works)
5. [Memory Map concept](#memory-map-concept)
6. [Example in C](#example-in-c)
7. [volatile keyword importance](#volatile-keyword-importance)
8. [MMIO vs Port-Mapped I/O](#mmio-vs-port-mapped-io)
9. [Use cases in Embedded Systems](#use-cases-in-embedded-systems)
10. [Advantages and Disadvantages](#advantages-and-disadvantages)
11. [Interview Questions](#interview-questions)
12. [Conclusion](#conclusion)

---

# Introduction

In embedded systems, hardware devices like GPIO, UART, SPI, and timers need to be controlled by software.

To interact with hardware efficiently, we use **Memory-Mapped I/O (MMIO)**.

---

# What is Memory-Mapped I/O?

### Definition

> Memory-Mapped I/O is a technique where hardware device registers are mapped into the same address space as system memory, allowing the CPU to access hardware using normal memory operations.

---

## Simple Meaning

👉 Hardware registers behave like memory locations.

---

# Why do we use MMIO?

✔ Faster hardware access
✔ Simple programming model
✔ No special I/O instructions required
✔ Used in microcontrollers and SoCs
✔ Direct register control

---

# How MMIO works

Instead of special I/O instructions, CPU uses:

```text id="mm1"
Load / Store instructions
```

---

## Flow

```text id="mm2"
CPU → Address Bus → Hardware Register
```

---

## Example

```text id="mm3"
0x40021000 → GPIO Register
```

---

# Memory Map concept

In embedded systems:

```text id="mm4"
0x00000000 → Flash
0x20000000 → RAM
0x40000000 → Peripheral Registers
```

---

## Example Peripheral Region

```text id="mm5"
GPIO → 0x40020000
UART → 0x40011000
TIMER → 0x40012C00
```

---

# Example in C

```c id="mm6"
#define GPIO_ODR (*(volatile unsigned int*)0x40020014)

int main()
{
    GPIO_ODR = 1;   // Turn ON pin
    GPIO_ODR = 0;   // Turn OFF pin
}
```

---

# volatile keyword importance

## Why volatile?

Because hardware registers can change **without CPU knowledge**.

---

## Without volatile (problem)

```c id="mm7"
GPIO_ODR = 1;  // might be optimized away
```

---

## With volatile (correct)

```c id="mm8"
volatile unsigned int *reg;
```

---

## Meaning

```text id="mm9"
volatile = do not optimize this memory access
```

---

# MMIO vs Port-Mapped I/O

| Feature       | MMIO                    | Port-Mapped I/O          |
| ------------- | ----------------------- | ------------------------ |
| Address space | Shared with memory      | Separate                 |
| Instructions  | Normal load/store       | Special I/O instructions |
| Usage         | ARM, STM32, modern MCUs | x86 legacy systems       |
| Complexity    | Simple                  | Complex                  |

---

# Use cases in Embedded Systems

## 1. GPIO Control

```c id="mm10"
LED ON/OFF via register
```

---

## 2. UART Communication

```c id="mm11"
Read/write UART data register
```

---

## 3. Timers

```c id="mm12"
Configure timer registers
```

---

## 4. ADC/DAC

```c id="mm13"
Read sensor values from registers
```

---

## 5. Automotive ECUs

* Engine control
* ABS systems
* Airbag systems

---

# Advantages

✔ Fast hardware access
✔ Simple programming model
✔ No special instructions needed
✔ Efficient for embedded systems
✔ Standard in modern processors

---

# Disadvantages

✘ Risk of incorrect register access
✘ Hardware dependent
✘ Requires careful documentation
✘ Debugging can be difficult
✘ Mistakes can crash system

---

# Interview Questions

## What is Memory-Mapped I/O?

Memory-Mapped I/O is a method where hardware registers are mapped into memory space and accessed using normal memory operations.

---

## Why do we use MMIO?

To simplify hardware access and allow direct control of peripherals using memory addresses.

---

## Why is volatile used in MMIO?

To prevent compiler optimization of hardware register access.

---

## Difference between MMIO and Port-Mapped I/O?

MMIO uses normal memory space, while Port-Mapped I/O uses separate I/O instructions.

---

## Where is MMIO used?

In microcontrollers, embedded systems, drivers, and hardware register programming.

---

# Most Asked Interview Question

## Explain Memory-Mapped I/O with example.

Memory-Mapped I/O is a technique where hardware registers are mapped into the same address space as system memory. This allows the CPU to access hardware devices using normal memory read/write operations. For example, in embedded systems, GPIO registers are accessed using pointers to fixed memory addresses like `*(volatile int*)0x40020014`. It is widely used in microcontrollers like STM32 and ARM-based systems for controlling peripherals such as GPIO, UART, and timers.

---

# Quick Revision

```text id="mm14"
MMIO = Hardware as Memory

Key Points:
- Registers mapped to memory
- Access using pointers
- Uses volatile keyword

Used in:
- GPIO
- UART
- Timers
- ADC
```

---

# Conclusion

Memory-Mapped I/O is a fundamental concept in embedded systems that allows direct interaction with hardware using memory addresses. It simplifies hardware programming and is widely used in microcontrollers, drivers, and real-time systems. Understanding MMIO is essential for embedded firmware development and system-level programming interviews.
