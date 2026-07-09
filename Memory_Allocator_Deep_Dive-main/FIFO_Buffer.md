# FIFO Buffer (First In First Out Buffer)

# Table of Contents

1. Introduction
2. What is a FIFO Buffer?
3. Why Do We Need FIFO?
4. FIFO Principle
5. FIFO Memory Layout
6. FIFO Operations
7. FIFO Implementation in C
8. Linear Buffer vs FIFO Buffer
9. FIFO vs Circular Buffer
10. Hardware FIFO vs Software FIFO
11. FIFO in Embedded Systems
12. FIFO in Device Drivers
13. FIFO in Linux Kernel
14. FIFO in DMA
15. Applications
16. Advantages
17. Disadvantages
18. Best Practices
19. Interview Questions
20. Summary

---

# 1. Introduction

FIFO stands for:

```text
First In First Out
```

It is a buffer where:

* The first data inserted is the first data removed.
* Data is processed in the same order it arrives.

Think of it like a queue at a ticket counter:

```text
Person A arrives first

↓

Person B arrives

↓

Person C arrives


A leaves first

↓

B leaves

↓

C leaves
```

FIFO preserves the order of data.

---

# 2. What is a FIFO Buffer?

A FIFO Buffer is a data structure that stores data sequentially and removes data in the same order.

Operations:

```text
Push / Enqueue

↓

Store Data

↓

Pop / Dequeue

↓

Retrieve Data
```

FIFO uses:

* Read Pointer (Head)
* Write Pointer (Tail)
* Buffer Array
* Size

---

# 3. Why Do We Need FIFO?

Suppose UART receives:

```text
A B C D E
```

CPU is busy.

Without FIFO:

```text
UART

↓

CPU Busy

↓

Data Lost
```

With FIFO:

```text
UART

↓

FIFO Buffer

↓

CPU

↓

Application
```

FIFO temporarily stores data.

---

# 4. FIFO Principle

Example:

Insert:

```text
A

B

C

D
```

Memory:

```text
Front                    Rear

↓

+---+---+---+---+

| A | B | C | D |

+---+---+---+---+

                    ↑
```

---

Remove:

```text
Remove A
```

Memory:

```text
Front

↓

+---+---+---+---+

|   | B | C | D |

+---+---+---+---+

            ↑

           Rear
```

---

Remove again:

```text
Remove B
```

Memory:

```text
+---+---+---+---+

|   |   | C | D |

+---+---+---+---+
```

FIFO order preserved.

---

# 5. FIFO Memory Layout

Typical structure:

```c
typedef struct
{

uint8_t *buffer;

uint32_t size;

uint32_t head;

uint32_t tail;

uint32_t count;

}FIFO_t;
```

---

Meaning:

```text
buffer

↓

+---+---+---+---+---+

| A | B | C | D | E |

+---+---+---+---+---+

  ↑               ↑

head             tail
```

---

# 6. FIFO Operations

FIFO supports:

---

## Push (Enqueue)

Insert data.

```c
fifo_push(data);
```

Example:

Before:

```text
+---+---+---+

| A | B |   |

+---+---+---+
```

Push:

```text
C
```

After:

```text
+---+---+---+

| A | B | C |

+---+---+---+
```

---

## Pop (Dequeue)

Remove oldest element.

```c
fifo_pop();
```

Before:

```text
+---+---+---+

| A | B | C |

+---+---+---+
```

Pop:

```text
A
```

After:

```text
+---+---+---+

|   | B | C |

+---+---+---+
```

---

## Peek

View first element.

```c
fifo_peek();
```

Returns:

```text
A
```

without removing.

---

## Check Empty

```c
fifo_is_empty();
```

Condition:

```c
count == 0
```

---

## Check Full

```c
fifo_is_full();
```

Condition:

```c
count == size
```

---

# 7. FIFO Implementation in C

---

## Structure

```c
typedef struct
{

uint8_t *buf;

uint32_t size;

uint32_t head;

uint32_t tail;

uint32_t count;

}FIFO_t;
```

---

## Initialization

```c
void fifo_init(FIFO_t *fifo,
               uint8_t *buffer,
               uint32_t size)
{

fifo->buf = buffer;

fifo->size = size;

fifo->head = 0;

fifo->tail = 0;

fifo->count = 0;

}
```

---

## Push

```c
int fifo_push(FIFO_t *fifo,
              uint8_t data)
{

if(fifo->count == fifo->size)

return -1;


fifo->buf[fifo->tail] = data;

fifo->tail++;

fifo->count++;

return 0;

}
```

---

## Pop

```c
int fifo_pop(FIFO_t *fifo,
             uint8_t *data)
{

if(fifo->count == 0)

return -1;


*data = fifo->buf[fifo->head];

fifo->head++;

fifo->count--;

return 0;

}
```

---

# 8. Linear Buffer vs FIFO Buffer

| Feature      | Linear Buffer     | FIFO Buffer        |
| ------------ | ----------------- | ------------------ |
| Access Order | Sequential        | First In First Out |
| Read         | Sequential        | Oldest first       |
| Write        | Sequential        | Append at tail     |
| Reuse Memory | Usually No        | Possible           |
| Applications | Temporary Storage | Queueing           |
| Complexity   | Low               | Moderate           |

---

Example:

Linear:

```text
Read any position.

buffer[5]

buffer[10]
```

FIFO:

```text
Must read:

A

then

B

then

C
```

---

# 9. FIFO vs Circular Buffer

Many people confuse these.

---

FIFO:

```text
FIFO

↓

Logical Data Structure
```

---

Circular Buffer:

```text
Circular Buffer

↓

Memory Management Technique
```

---

Most FIFOs are implemented as:

```text
FIFO Logic

+

Circular Buffer Memory
```

because memory can be reused.

---

Example:

```text
0 1 2 3 4

+---+---+---+---+---+

| A | B | C | D | E |

+---+---+---+---+---+

              tail


tail wraps


+---+---+---+---+---+

| F | B | C | D | E |

+---+---+---+---+---+

tail = 0
```

---

# 10. Hardware FIFO vs Software FIFO

---

## Hardware FIFO

Built into peripherals.

Examples:

* UART FIFO
* SPI FIFO
* I2C FIFO
* USB FIFO
* Ethernet FIFO

Example:

```text
UART RX FIFO

16 Bytes

32 Bytes

64 Bytes
```

CPU reads later.

---

## Software FIFO

Implemented in RAM.

Example:

```c
uint8_t fifo[256];
```

Managed by:

```text
head

tail

count
```

---

# 11. FIFO in Embedded Systems

FIFO is heavily used in:

---

## UART

```text
UART RX

↓

Hardware FIFO

↓

ISR

↓

Software FIFO

↓

Application
```

Stores incoming bytes.

---

## ADC

Stores:

```text
ADC Sample 1

ADC Sample 2

ADC Sample 3
```

---

## SPI

Stores:

```text
TX FIFO

RX FIFO
```

---

## Audio

Stores:

```text
PCM samples

Microphone samples
```

---

## Sensors

Stores:

```text
Temperature

Pressure

Gyroscope

Accelerometer
```

---

# 12. FIFO in Device Drivers

Used in:

---

## UART Drivers

```text
ISR

↓

RX FIFO

↓

User Space
```

---

## Ethernet Drivers

Stores:

```text
Packets

Frames

Descriptors
```

---

## USB Drivers

Stores:

```text
Endpoint Data

Transfer Requests
```

---

## CAN Drivers

Stores:

```text
CAN Frames
```

---

# 13. FIFO in Linux Kernel

Linux provides:

```c
kfifo
```

Structure:

```c
DECLARE_KFIFO(fifo,
              unsigned char,
              256);
```

Initialize:

```c
INIT_KFIFO(fifo);
```

Insert:

```c
kfifo_in();
```

Remove:

```c
kfifo_out();
```

Check:

```c
kfifo_is_empty();

kfifo_is_full();
```

Widely used in:

* UART Drivers
* Networking
* USB
* Audio Drivers

---

# 14. FIFO in DMA

DMA transfers:

```text
Peripheral

↓

DMA

↓

FIFO Buffer

↓

CPU
```

Examples:

* Audio DMA
* ADC DMA
* Ethernet DMA
* SPI DMA

FIFO stores data until CPU processes it.

---

# 15. Applications

FIFO is used in:

### Embedded Systems

* UART
* SPI
* I2C
* ADC
* DMA

---

### Operating Systems

* Linux Kernel
* RTOS
* FreeRTOS
* Zephyr

---

### Networking

* Packet Queues
* Ring Buffers
* Socket Buffers

---

### Audio

* PCM Buffers
* Microphone Streams

---

### Video

* Frame Queues
* Image Buffers

---

### Databases

* Task Queues
* Event Queues

---

# 16. Advantages

## Preserves Order

First data arrives:

First data leaves.

---

## Fast

Most operations:

```text
O(1)
```

---

## Efficient

Very little overhead.

---

## Suitable for Streaming

Examples:

* UART
* Ethernet
* Audio
* DMA

---

## Easy Synchronization

Useful in:

* Producer Consumer
* ISR to Thread
* DMA to CPU

---

# 17. Disadvantages

## Fixed Size

Overflow occurs if:

```c
count == size
```

---

## Overflow Risk

Example:

```text
Producer Faster

↓

FIFO Full

↓

Data Lost
```

---

## Synchronization Required

In:

* Multithreading
* SMP
* Interrupt Context

Need:

```text
Mutex

Spinlock

Atomic Operations
```

---

## Dynamic Resize Difficult

Usually FIFO size is fixed.

---

# 18. Best Practices

✔ Use Circular Buffer internally.

✔ Check:

```c
fifo_is_full();
```

before push.

---

✔ Check:

```c
fifo_is_empty();
```

before pop.

---

✔ Use power of 2 size:

```text
64

128

256

512
```

for faster index calculation.

---

✔ Use lock-free FIFO when:

* ISR writes
* Thread reads
* DMA transfers

---

✔ Use Linux:

```c
kfifo
```

instead of custom implementation.

---

# 19. Interview Questions

### Q1. What is FIFO?

FIFO means:

```text
First In First Out
```

Oldest element is removed first.

---

### Q2. FIFO vs Stack?

FIFO:

```text
A

B

C

Remove A
```

Stack:

```text
A

B

C

Remove C
```

---

### Q3. FIFO vs Circular Buffer?

FIFO:

Logical queue.

Circular Buffer:

Memory implementation.

---

### Q4. FIFO Applications?

* UART
* DMA
* SPI
* Ethernet
* USB
* Audio
* Linux Drivers

---

### Q5. Linux FIFO implementation?

```c
kfifo
```

Kernel API for FIFO operations.

---

# 20. Summary

FIFO Buffer is:

* A queue data structure.
* First element inserted is the first element removed.
* Uses:

  * Head pointer
  * Tail pointer
  * Count
  * Buffer array

Used heavily in:

* UART Drivers
* DMA
* Linux Kernel
* Embedded Systems
* Networking
* Audio Processing
* USB Drivers

Most FIFOs are implemented using:

```text
FIFO Logic

+

Circular Buffer Memory
```

because it:

✔ Reuses memory

✔ Prevents memory waste

✔ Supports continuous streaming

FIFO is one of the most important concepts in:

**Embedded Systems + Linux Device Drivers + RTOS + Multithreading + Networking.**
