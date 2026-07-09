
# Linear Buffer

# Table of Contents

1. Introduction
2. What is a Linear Buffer?
3. Why Do We Need a Linear Buffer?
4. Characteristics
5. Memory Layout
6. Operations on Linear Buffer
7. Implementation in C
8. Linear Buffer vs Circular Buffer
9. Applications
10. Usage in Embedded Systems
11. Usage in Device Drivers
12. Advantages
13. Disadvantages
14. Best Practices
15. Interview Questions
16. Summary

---

# 1. Introduction

A Buffer is a temporary memory area used to store data while it is being transferred from one place to another.

A **Linear Buffer** stores data in a continuous block of memory and accesses it sequentially from beginning to end.

It is one of the simplest and most commonly used data structures in:

* Embedded Systems
* Device Drivers
* UART Communication
* DMA Transfers
* File Systems
* Networking
* Audio/Video Processing

---

# 2. What is a Linear Buffer?

A Linear Buffer is a fixed-size contiguous memory block where:

* Data is written from left to right.
* Data is read from left to right.
* Once the buffer becomes full, no more data can be added unless:

  * Data is removed.
  * Buffer is reset.
  * Buffer is reallocated.

---

## Example

```text
Index:

0   1   2   3   4   5   6   7

+---+---+---+---+---+---+---+---+
| A | B | C | D | E |   |   |   |
+---+---+---+---+---+---+---+---+

Write Pointer = 5

Read Pointer = 0
```

---

# 3. Why Do We Need a Linear Buffer?

Suppose data arrives from UART:

```text
A B C D E F G
```

CPU may not process it immediately.

A buffer stores incoming data temporarily.

Without buffer:

```text
UART -> CPU Busy

Incoming Data Lost
```

With buffer:

```text
UART

↓

Linear Buffer

↓

CPU
```

Data is preserved until processed.

---

# 4. Characteristics

A Linear Buffer has:

### Continuous Memory

```text
buffer[0]

buffer[1]

buffer[2]

buffer[3]
```

Stored sequentially in RAM.

---

### Fixed Size

Example:

```c
char buffer[256];
```

Maximum:

```text
256 bytes
```

---

### Sequential Access

Read:

```text
0 → 1 → 2 → 3
```

Write:

```text
0 → 1 → 2 → 3
```

---

### Simple Management

Only:

* Read pointer
* Write pointer
* Size

need to be maintained.

---

# 5. Memory Layout

Example:

```c
char buffer[8];
```

Memory:

```text
Address

1000 -> buffer[0]

1001 -> buffer[1]

1002 -> buffer[2]

1003 -> buffer[3]

1004 -> buffer[4]

1005 -> buffer[5]

1006 -> buffer[6]

1007 -> buffer[7]
```

Continuous memory.

---

# 6. Operations on Linear Buffer

Basic operations:

---

## Write

```c
buffer[write++] = data;
```

Example:

```text
Before

+---+---+---+---+
| A | B |   |   |
+---+---+---+---+

write=2


Insert C


+---+---+---+---+
| A | B | C |   |
+---+---+---+---+

write=3
```

---

## Read

```c
data = buffer[read++];
```

Example:

```text
Read A


+---+---+---+---+
| A | B | C | D |
+---+---+---+---+

read=1
```

---

## Reset

```c
read = 0;

write = 0;
```

Buffer becomes empty.

---

## Check Full

```c
if(write == size)
{
    buffer full;
}
```

---

## Check Empty

```c
if(read == write)
{
    buffer empty;
}
```

---

# 7. Implementation in C

Structure:

```c
typedef struct
{
    uint8_t *buffer;

    uint32_t size;

    uint32_t read;

    uint32_t write;

}LinearBuffer;
```

---

## Initialize

```c
void init(LinearBuffer *lb,
          uint8_t *buf,
          uint32_t size)
{

    lb->buffer = buf;

    lb->size = size;

    lb->read = 0;

    lb->write = 0;

}
```

---

## Write Function

```c
int write_buffer(LinearBuffer *lb,
                 uint8_t data)
{

    if(lb->write >= lb->size)

        return -1;


    lb->buffer[lb->write++] = data;

    return 0;

}
```

---

## Read Function

```c
int read_buffer(LinearBuffer *lb,
                uint8_t *data)
{

    if(lb->read >= lb->write)

        return -1;


    *data = lb->buffer[lb->read++];

    return 0;

}
```

---

# 8. Linear Buffer vs Circular Buffer

| Feature           | Linear Buffer       | Circular Buffer |
| ----------------- | ------------------- | --------------- |
| Memory            | Continuous          | Continuous      |
| Reuse Space       | No                  | Yes             |
| Overflow          | Easy                | Less likely     |
| Complexity        | Simple              | Moderate        |
| Write Pointer     | Moves one direction | Wraps around    |
| Read Pointer      | Moves one direction | Wraps around    |
| Memory Efficiency | Lower               | Higher          |

---

Example:

Linear Buffer:

```text
+---+---+---+---+

A B C D

FULL

Cannot insert more

+---+---+---+---+
```

---

Circular Buffer:

```text
Write pointer wraps

+---+---+---+---+

A B C D

↓

Reuse A location

+---+---+---+---+
```

---

# 9. Applications

Linear Buffers are used in:

---

## UART Reception

Example:

```text
UART

↓

Linear Buffer

↓

Application
```

---

## SPI Transfers

Stores:

* Incoming bytes
* Outgoing bytes

---

## I2C Communication

Temporary data storage.

---

## Audio Processing

Stores:

* Audio samples
* PCM data

---

## Video Processing

Stores:

* Frames
* Pixel data

---

## Networking

Stores:

* Packets
* Frames
* Ethernet data

---

## File Systems

Stores:

* File chunks
* Sector data

---

# 10. Usage in Embedded Systems

Common in:

### UART Driver

```c
UART_RX_ISR()

{

buffer[write++] = UART_DR;

}
```

Stores received bytes.

---

### ADC Data

```c
adc_buffer[index++] = ADC_RESULT;
```

Stores samples.

---

### Sensor Data

```c
temperature_buffer[index++]
```

Stores readings.

---

### LCD Frame Buffer

Stores:

```text
Pixel Data

Color Information

Characters
```

---

# 11. Usage in Device Drivers

Linear Buffers are heavily used in:

---

## USB Drivers

Store:

* Endpoint data
* Transfer descriptors

---

## Ethernet Drivers

Store:

* RX packets
* TX packets

---

## UART Drivers

Store:

* Incoming bytes
* Outgoing bytes

---

## DMA Drivers

DMA transfers data to:

```text
DMA

↓

Linear Buffer

↓

CPU Processes
```

---

# 12. Advantages

## Simple Design

Easy to implement.

---

## Fast Access

O(1)

for:

* Read
* Write

---

## Continuous Memory

Cache friendly.

Better performance.

---

## Easy Debugging

Simple:

```text
read

write

size
```

only.

---

## Suitable for Small Systems

Works well in:

* Microcontrollers
* Small RTOS
* Bare Metal Systems

---

# 13. Disadvantages

## Cannot Reuse Space

Example:

```text
+---+---+---+---+

A B C D

Read A,B

Space exists

But cannot reuse

+---+---+---+---+
```

Memory wasted.

---

## Buffer Overflow

```c
write > size
```

causes:

* Memory corruption
* Crash
* Undefined behavior

---

## Frequent Reset Required

Need:

```c
read=0;

write=0;
```

after processing.

---

## Not Suitable for Continuous Streams

Examples:

* Audio streaming
* Ethernet packets
* High speed UART

Circular buffer is preferred.

---

# 14. Best Practices

✔ Check for overflow.

```c
if(write >= size)
```

---

✔ Check for underflow.

```c
if(read >= write)
```

---

✔ Use DMA for large transfers.

---

✔ Protect shared buffer:

```c
mutex

or

spinlock
```

in multithreading.

---

✔ Use Circular Buffer when:

* Continuous stream exists
* Data never stops
* Producer and consumer run simultaneously

---

# 15. Interview Questions

### Q1. What is Linear Buffer?

A fixed-size contiguous memory area where data is written and read sequentially.

---

### Q2. Difference between Linear and Circular Buffer?

Linear:

* No wrap around.

Circular:

* Wraps around.
* Reuses memory.

---

### Q3. What causes Linear Buffer overflow?

```c
write >= size
```

Writing beyond allocated memory.

---

### Q4. Why is Circular Buffer preferred in UART?

Because:

* Continuous data arrives.
* Memory can be reused.
* Overflow probability is lower.

---

### Q5. Is Linear Buffer cache friendly?

Yes.

Because memory is:

```text
Continuous

Sequential
```

which improves cache efficiency.

---

# 16. Summary

Linear Buffer is:

* A contiguous memory block.
* Data is stored sequentially.
* Simple and fast.
* Common in:

  * UART
  * SPI
  * DMA
  * ADC
  * USB
  * Ethernet
  * Embedded Systems
  * Device Drivers

Advantages:

* Easy implementation
* Fast access
* Cache friendly

Disadvantages:

* Cannot reuse memory
* Overflow risk
* Not suitable for continuous streams

For continuous data streams:

```text
Linear Buffer  ❌

Circular Buffer ✔
```

Linear Buffers are one of the most fundamental data structures used in:

**Embedded Systems + Device Drivers + RTOS + Firmware Development.**
