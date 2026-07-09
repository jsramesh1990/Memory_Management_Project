# Ping Pong Buffer

# Table of Contents

1. Introduction
2. What is Ping Pong Buffer?
3. Why Do We Need Ping Pong Buffer?
4. Basic Concept
5. Memory Layout
6. How Ping Pong Buffer Works
7. Ping Pong vs Double Buffer
8. Ping Pong vs Circular Buffer
9. Implementation in C
10. Ping Pong Buffer with DMA
11. Ping Pong Buffer in Embedded Systems
12. Ping Pong Buffer in Linux Device Drivers
13. Applications
14. Advantages
15. Disadvantages
16. Best Practices
17. Interview Questions
18. Real-World Examples
19. Summary

---

# 1. Introduction

A **Ping Pong Buffer** is a buffering technique that uses **two equal-sized memory buffers** alternately.

* While one buffer is being filled with data,
* The other buffer is being processed.

After one operation finishes, the roles of the buffers are swapped.

This creates a continuous flow of data without interruption.

Ping Pong Buffer is widely used in:

* DMA Transfers
* ADC Sampling
* UART DMA
* Audio Streaming
* Video Processing
* Networking
* Linux Device Drivers
* DSP Applications

---

# 2. What is Ping Pong Buffer?

Ping Pong Buffer uses:

```text
Buffer A (Ping)

Buffer B (Pong)
```

At any instant:

```text
DMA writes → Ping

CPU reads  → Pong
```

After DMA finishes:

```text
DMA writes → Pong

CPU reads  → Ping
```

This swapping continues forever:

```text
Ping

↓

Pong

↓

Ping

↓

Pong

↓

Ping
```

Hence the name:

```text
Ping Pong
```

similar to a table tennis game.

---

# 3. Why Do We Need Ping Pong Buffer?

Suppose:

DMA continuously receives ADC samples.

Single Buffer:

```text
DMA

↓

Buffer

↑

CPU Reads
```

Problem:

DMA writes while CPU reads.

Possible result:

```text
CPU gets:

50% old samples

50% new samples
```

Corrupted data.

---

Ping Pong Buffer:

```text
DMA

↓

Ping


CPU

↓

Pong
```

Separate buffers.

No corruption.

---

# 4. Basic Concept

Initially:

```text
Ping Buffer

+----------------+

Empty

+----------------+


Pong Buffer

+----------------+

Empty

+----------------+
```

---

Step 1

DMA fills Ping:

```text
DMA

↓

Ping
```

CPU:

```text
Idle
```

---

Step 2

DMA complete.

Switch:

```text
DMA

↓

Pong


CPU

↓

Ping
```

CPU processes Ping.

DMA fills Pong.

---

Step 3

DMA complete again.

Switch:

```text
DMA

↓

Ping


CPU

↓

Pong
```

This continues:

```text
Ping

↓

Pong

↓

Ping

↓

Pong
```

forever.

---

# 5. Memory Layout

Example:

```c
uint16_t ping[256];

uint16_t pong[256];
```

Memory:

```text
Ping

↓

+----------------------+

256 Samples

+----------------------+


Pong

↓

+----------------------+

256 Samples

+----------------------+
```

Two independent buffers.

---

# 6. How Ping Pong Buffer Works

Assume:

```text
Buffer Size = 8
```

---

DMA writes:

```text
Ping

+---+---+---+---+

1 2 3 4

+---+---+---+---+
```

CPU:

```text
Pong

Empty
```

---

DMA complete.

Interrupt occurs.

Swap:

```text
DMA → Pong

CPU → Ping
```

---

DMA writes:

```text
Pong

+---+---+---+---+

5 6 7 8

+---+---+---+---+
```

CPU processes:

```text
Ping

1 2 3 4
```

---

DMA complete.

Swap again.

```text
DMA → Ping

CPU → Pong
```

Continuous streaming.

---

# 7. Ping Pong vs Double Buffer

Many engineers use these terms interchangeably.

Practically:

```text
Ping Pong Buffer

=

Double Buffer
```

because both use:

```text
Two Buffers

Alternate Usage
```

---

Small difference:

Double Buffer:

```text
General Technique
```

Ping Pong:

```text
Continuous Alternating Buffers
```

Mostly used for:

* DMA
* ADC
* Audio
* UART

---

# 8. Ping Pong vs Circular Buffer

| Feature           | Ping Pong | Circular Buffer |
| ----------------- | --------- | --------------- |
| Buffers           | Two       | One             |
| Memory Reuse      | Partial   | Complete        |
| Data Isolation    | Excellent | Moderate        |
| DMA Support       | Excellent | Excellent       |
| Complexity        | Low       | Moderate        |
| Continuous Stream | Excellent | Excellent       |
| Latency           | Fixed     | Variable        |

---

Ping Pong:

```text
DMA → Ping

CPU → Pong
```

---

Circular:

```text
DMA

↓

Ring Buffer

↑

CPU
```

Shared memory.

---

# 9. Implementation in C

---

## Structure

```c
typedef struct
{

uint8_t *ping;

uint8_t *pong;

uint8_t *read_buf;

uint8_t *write_buf;

uint32_t size;

}PingPong;
```

---

## Initialization

```c
uint8_t ping[256];

uint8_t pong[256];


PingPong pp;


pp.ping = ping;

pp.pong = pong;


pp.read_buf = ping;

pp.write_buf = pong;

pp.size = 256;
```

---

## Swap Buffers

```c
void swap(PingPong *pp)
{

uint8_t *temp;


temp = pp->read_buf;


pp->read_buf

=

pp->write_buf;


pp->write_buf

=

temp;

}
```

---

# 10. Ping Pong Buffer with DMA

Most DMA controllers support Ping Pong mode.

Example:

```text
ADC

↓

DMA

↓

Memory 0

Memory 1

↓

CPU
```

DMA automatically switches:

```text
Memory0

↓

Memory1

↓

Memory0

↓

Memory1
```

CPU processes completed buffer.

---

STM32 DMA:

Registers:

```text
DMA_SxM0AR

Memory 0


DMA_SxM1AR

Memory 1
```

Enable:

```text
DMA_SxCR_DBM
```

DBM:

```text
Double Buffer Mode
```

---

# 11. Ping Pong Buffer in Embedded Systems

Used heavily in:

---

## ADC Sampling

```text
ADC

↓

DMA

↓

Ping


CPU DSP

↓

Pong
```

Continuous sampling.

---

## UART DMA

```text
UART RX

↓

DMA

↓

Ping


CPU

↓

Pong
```

No data loss.

---

## Audio Processing

```text
Microphone

↓

DMA

↓

Ping


CPU DSP

↓

Pong

↓

Speaker
```

Continuous playback.

---

## SPI DMA

```text
SPI

↓

DMA

↓

Ping

Pong

↓

Application
```

---

# 12. Ping Pong Buffer in Linux Device Drivers

Used in:

---

## ALSA Audio Drivers

```text
DMA

↓

Ping

Pong

↓

Audio Engine
```

---

## Video Drivers

```text
Frame A

Frame B
```

Alternate rendering.

---

## Ethernet Drivers

```text
RX Ping

RX Pong

↓

CPU
```

Packet processing.

---

## SDR Drivers

Software Defined Radio:

```text
ADC

↓

Ping

Pong

↓

FFT

↓

Demodulation
```

---

# 13. Applications

Ping Pong Buffer is used in:

---

### Embedded Systems

* UART DMA
* ADC DMA
* SPI DMA
* Audio Streaming

---

### Graphics

* Frame Buffers
* OpenGL
* Vulkan

---

### Linux Drivers

* ALSA
* DRM
* Networking

---

### DSP

* FFT
* FIR Filters
* Audio Effects

---

### Networking

* Packet Buffers
* Ethernet Drivers

---

# 14. Advantages

## No Data Corruption

Read and write use different buffers.

---

## Parallel Processing

```text
DMA

+

CPU

work simultaneously
```

---

## Predictable Latency

Buffer size determines latency.

---

## Very DMA Friendly

Most DMA controllers support it.

---

## Easy to Implement

Simple:

```text
Swap Buffers
```

logic.

---

# 15. Disadvantages

## Extra Memory

Need:

```text
2 × Buffer Size
```

RAM usage doubles.

---

## CPU Must Keep Up

If CPU is slow:

```text
DMA

overwrites

buffer
```

Data loss occurs.

---

## Fixed Buffer Size

Cannot easily resize.

---

## Less Memory Efficient

Compared to:

```text
Circular Buffer
```

---

# 16. Best Practices

✔ Keep both buffers same size.

---

✔ Use DMA interrupts:

```text
Half Transfer

Transfer Complete
```

---

✔ Swap atomically:

```c
disable_irq();

swap();

enable_irq();
```

---

✔ Use cache maintenance:

```text
Clean Cache

Invalidate Cache
```

for DMA systems.

---

✔ Use Ping Pong when:

* Continuous stream exists
* DMA transfers data
* CPU processing time is predictable
* Fixed latency is required

---

# 17. Interview Questions

### Q1. What is Ping Pong Buffer?

A double buffering technique using two buffers alternately.

---

### Q2. Why is it called Ping Pong?

Because buffers alternate:

```text
Ping

↓

Pong

↓

Ping

↓

Pong
```

like a table tennis ball.

---

### Q3. Ping Pong vs Double Buffer?

Practically:

```text
Ping Pong

=

Double Buffer
```

Ping Pong usually refers to continuous DMA streaming.

---

### Q4. Ping Pong vs Circular Buffer?

Ping Pong:

```text
Two Buffers

Swap
```

Circular:

```text
One Buffer

Wrap Around
```

---

### Q5. Why use Ping Pong with DMA?

DMA writes:

```text
Ping
```

while CPU reads:

```text
Pong
```

No overlap.

No corruption.

---

# 18. Real-World Examples

---

## STM32 ADC DMA

```text
ADC

↓

DMA

↓

Ping

Pong

↓

CPU DSP
```

---

## Audio Processing

```text
Microphone

↓

Ping

↓

CPU

↓

Pong

↓

Speaker
```

Continuous streaming.

---

## SDR (Software Defined Radio)

```text
ADC

↓

Ping

↓

FFT

↓

Pong

↓

Demodulation
```

---

## Graphics

```text
Back Buffer

↓

Swap

↓

Front Buffer

↓

Display
```

No flickering.

---

# 19. Summary

Ping Pong Buffer is:

```text
Ping

+

Pong
```

Two buffers used alternately.

One buffer:

```text
Write
```

Other buffer:

```text
Read
```

After completion:

```text
Swap
```

Advantages:

✔ No data corruption

✔ Parallel CPU and DMA

✔ Predictable latency

✔ Continuous streaming

Widely used in:

* DMA
* ADC
* UART
* SPI
* Audio
* Linux Drivers
* Networking
* DSP
* Graphics

Ping Pong Buffer is one of the most important techniques for:

**DMA + Embedded Systems + Audio Processing + Linux Device Drivers + Real-Time Streaming Systems.**
