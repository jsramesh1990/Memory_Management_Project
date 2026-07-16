# Communication Protocols

<a id="table-of-protocols"></a>

## Table of Protocols

| [1. UART](#1-uart)
| [2. USART](#2-usart)
| [3. SPI](#3-spi)
| [4. I2C](#4-i2c)
| [5. CAN](#5-can)
| [6. CAN FD](#6-can-fd)
| [7. LIN](#7-lin)
| [8. FlexRay](#8-flexray)
| [9. RS-232](#9-rs-232)
| [10. RS-485](#10-rs-485)
| [11. USB](#11-usb)
| [12. Ethernet](#12-ethernet)
| [13. GPIO](#13-gpio)

---

## 1. UART

**UART** (Universal Asynchronous Receiver-Transmitter) is a hardware communication protocol that enables asynchronous serial communication between devices. It converts parallel data from the CPU into serial data for transmission and vice versa for reception.

### Real-World Analogy

Think of UART like a **two-way radio conversation**:
- **Transmit (TX)**: One person talking (sending data)
- **Receive (RX)**: The other person listening (receiving data)
- **Asynchronous**: No shared clock - like walkie-talkies (just agree on speed)
- **Baud Rate**: How fast you speak (words per minute)
- **Start/Stop Bits**: "Over" and "Out" to mark message boundaries

---

## 🎯 Key Characteristics

| Characteristic | Description |
|----------------|-------------|
| **Asynchronous** | No clock signal shared between devices |
| **Full-Duplex** | Can transmit and receive simultaneously |
| **Serial** | Data sent one bit at a time |
| **Point-to-Point** | Direct connection between two devices |
| **Simple** | Only two data lines (TX and RX) |
| **Flexible** | Wide range of baud rates |

---

## 🔍 How UART Works

### Basic Architecture

```
Device A (UART)                      Device B (UART)
┌─────────────────────┐              ┌─────────────────────┐
│      CPU            │              │      CPU            │
├─────────────────────┤              ├─────────────────────┤
│  ┌───────────────┐  │              │  ┌───────────────┐  │
│  │   FIFO/TX     │  │              │  │   FIFO/RX     │  │
│  └───────┬───────┘  │              │  └───────▲───────┘  │
│          │          │   TX (Data)   │          │          │
│          └──────────┼──────────────┼──────────┘          │
│                     │              │                     │
│          ┌──────────┼──────────────┼──────────┐          │
│          │          │   RX (Data)  │          │          │
│  ┌───────▼───────┐  │              │  ┌───────┴───────┐  │
│  │   FIFO/RX     │  │              │  │   FIFO/TX     │  │
│  └───────────────┘  │              │  └───────────────┘  │
├─────────────────────┤              ├─────────────────────┤
│     Control/        │              │     Control/        │
│    Configuration    │              │    Configuration    │
└─────────────────────┘              └─────────────────────┘
```

---

## 📊 UART Frame Format

### Standard Data Frame

```
UART Frame (8-N-1):
┌─────┐   ┌─────────────┐   ┌─────┐
│Start│   │   Data      │   │Stop │
│ Bit │   │   Bits      │   │ Bit │
│  0  │   │ 0 1 2 3 4 5 6 7│   │  1  │
└─────┘   └─────────────┘   └─────┘

8-N-1 = 8 data bits, No parity, 1 stop bit

Full Frame (with parity):
┌─────┐   ┌─────────────┐   ┌─────┐   ┌─────┐
│Start│   │   Data      │   │Parity│   │Stop │
│ Bit │   │   Bits      │   │ Bit  │   │ Bit │
│  0  │   │ 0 1 2 3 4 5 6 7│   │  P  │   │  1  │
└─────┘   └─────────────┘   └─────┘   └─────┘
```

### Frame Components

| Component | Description | Bits | Purpose |
|-----------|-------------|------|---------|
| **Start Bit** | Low signal (0) | 1 | Marks beginning of frame |
| **Data Bits** | Actual data | 5-9 | Contains payload |
| **Parity Bit** | Error checking | 0-1 | Detects errors |
| **Stop Bit(s)** | High signal (1) | 1-2 | Marks end of frame |
| **Idle State** | High (1) | - | No data on line |

### Common Frame Formats

| Format | Data Bits | Parity | Stop Bits | Use Case |
|--------|-----------|--------|-----------|----------|
| **8N1** | 8 | None | 1 | Most common (default) |
| **8E1** | 8 | Even | 1 | Error detection |
| **8O1** | 8 | Odd | 1 | Error detection |
| **7E1** | 7 | Even | 1 | Older systems |
| **9N1** | 9 | None | 1 | Special applications |

---

## ⏱️ Baud Rate and Timing

### Baud Rate vs Bit Rate
- **Baud Rate**: Signal changes per second
- **Bit Rate**: Data bits per second
- For UART: Baud Rate = Bit Rate (one bit per symbol)

### Common Baud Rates
```
Low Speed:
  1200 baud   (0.001s per bit)
  2400 baud   (0.000416s per bit)
  4800 baud   (0.000208s per bit)
  9600 baud   (0.000104s per bit)
  19200 baud  (0.000052s per bit)

Medium Speed:
  38400 baud  (0.000026s per bit)
  57600 baud  (0.000017s per bit)
  115200 baud (0.0000086s per bit)

High Speed:
  230400 baud (0.0000043s per bit)
  460800 baud (0.0000021s per bit)
  921600 baud (0.00000108s per bit)
  2M baud    (0.0000005s per bit)
```

### Timing Example (9600 baud, 8N1)
```
Frame Duration:
Start Bit:     1 bit  → 104 µs
Data Bits:     8 bits → 832 µs (8 × 104 µs)
Stop Bit:      1 bit  → 104 µs
Total:         10 bits → 1.04 ms per character

Throughput:
104 characters per second
(1 / 0.000104s × 8 = 9600 bps)
```

---

## 🔧 UART Hardware Signals

### Standard Signals

| Signal | Direction | Description |
|--------|-----------|-------------|
| **TX** | Out | Transmit Data |
| **RX** | In | Receive Data |
| **RTS** | Out | Request To Send (flow control) |
| **CTS** | In | Clear To Send (flow control) |
| **DTR** | Out | Data Terminal Ready |
| **DSR** | In | Data Set Ready |
| **DCD** | In | Data Carrier Detect |
| **RI** | In | Ring Indicator |

### Signal Levels

```
UART Signal Levels (TTL):
┌─────────────────────────────────────────┐
│ Logic High (1): 3.3V or 5V             │
│ Logic Low (0):  0V                     │
│ Idle State:     High (1)               │
│ Start Bit:      Low (0)                │
│ Stop Bit:       High (1)               │
└─────────────────────────────────────────┘

UART Signal Levels (RS-232):
┌─────────────────────────────────────────┐
│ Logic High (1): -3V to -15V            │
│ Logic Low (0):  +3V to +15V            │
│ Idle State:     High (-3V to -15V)     │
└─────────────────────────────────────────┘
```

---

## 💡 UART Types

### 1. Standard UART
- Basic TX/RX only
- FIFO buffers
- Interrupt support
- Simple devices

### 2. USART (Universal Synchronous/Asynchronous)
- Both synchronous and asynchronous modes
- Supports clock signal (for synchronous)
- More flexible

### 3. UART with Flow Control
- Hardware flow control (RTS/CTS)
- Software flow control (XON/XOFF)
- Prevents buffer overflow

### 4. DMA-Enabled UART
- Uses DMA for data transfer
- Reduces CPU overhead
- Higher throughput

### 5. Multi-drop UART
- Multiple devices on same bus
- Addressable communication
- RS-485 support

---

## 🎯 Flow Control Methods

### 1. Hardware Flow Control (RTS/CTS)

```
Transmitter                     Receiver
┌─────────────────┐            ┌─────────────────┐
│  RTS (Request)  │───────────▶│  CTS (Clear)    │
├─────────────────┤            ├─────────────────┤
│  TX (Data)      │───────────▶│  RX (Data)      │
└─────────────────┘            └─────────────────┘

Process:
1. Transmitter: Assert RTS (request to send)
2. Receiver: Assert CTS (clear to send) when ready
3. Transmitter: Send data
4. Receiver: Deassert CTS when buffer full
5. Transmitter: Stop sending until CTS reasserted
```

### 2. Software Flow Control (XON/XOFF)

```
Process:
1. Receiver: Send XOFF (0x13) when buffer is full
2. Transmitter: Pause transmission
3. Receiver: Send XON (0x11) when buffer has space
4. Transmitter: Resume transmission

Characters:
- XON (DC1):  0x11 (17 decimal)
- XOFF (DC3): 0x13 (19 decimal)
```

### Flow Control Comparison

| Method | Pros | Cons | Use Case |
|--------|------|------|----------|
| **Hardware** | Fast, reliable | Requires extra wires | High speed |
| **Software** | No extra wires | Slower, data dependent | Simple systems |
| **None** | Simple | Risk of overflow | Low speed |

---

## 🎯 Real-World Applications

### 1. Debugging and Development
```
Target Device                    Debug Console
┌─────────────────┐            ┌─────────────────┐
│  UART TX        │───────────▶│  UART RX        │
├─────────────────┤            ├─────────────────┤
│  Printf output  │            │  Printf output  │
│  Log messages   │            │  Log messages   │
│  Error reports  │            │  Error reports  │
└─────────────────┘            └─────────────────┘

- Using printf over UART
- Firmware debugging
- Logging system events
- Serial console
```

### 2. GPS Modules
```
GPS Module                      Host System
┌─────────────────┐            ┌─────────────────┐
│  NMEA Sentences │───────────▶│  Parse NMEA     │
│  (Position,     │            │  Extract:       │
│   Time, Speed)  │            │  - Latitude     │
│                 │            │  - Longitude    │
│                 │            │  - Time/Date    │
│                 │            │  - Altitude     │
└─────────────────┘            └─────────────────┘
```

### 3. Sensor Data Acquisition
```
Sensors (ADC)                   
┌─────────────────┐            
│  Temperature    │───────────▶│  Log to SD card │
│  Humidity       │───────────▶│  Display UI     │
│  Pressure       │───────────▶│  Network send   │
│  Light          │───────────▶│                 │
└─────────────────┘            └─────────────────┘
```

### 4. Industrial Automation
```
PLC/Controller                  SCADA/HMI
┌─────────────────┐            ┌─────────────────┐
│  Modbus Protocol│───────────▶│  Process Data   │
│  Register read  │            │  Control        │
│  Device control │            │  Monitoring     │
└─────────────────┘            └─────────────────┘
```

---

## 🛠️ UART Configuration Parameters

### Standard Configuration Steps

```
1. Clock Configuration
   ┌────────────────────────────────────┐
   │ • Source clock (system clock)      │
   │ • Baud rate calculation            │
   │ • Prescaler settings               │
   └────────────────────────────────────┘

2. Parameter Settings
   ┌────────────────────────────────────┐
   │ • Baud rate                        │
   │ • Data bits (5-9)                  │
   │ • Parity (None/Even/Odd)           │
   │ • Stop bits (1/2)                  │
   └────────────────────────────────────┘

3. FIFO/Buffer Configuration
   ┌────────────────────────────────────┐
   │ • TX FIFO threshold                │
   │ • RX FIFO threshold                │
   │ • Buffer sizes                     │
   └────────────────────────────────────┘

4. Interrupt Configuration
   ┌────────────────────────────────────┐
   │ • Enable TX interrupt              │
   │ • Enable RX interrupt              │
   │ • Enable error interrupts          │
   └────────────────────────────────────┘

5. Enable UART
   ┌────────────────────────────────────┐
   │ • Configure pins (TX/RX)           │
   │ • Enable UART module               │
   │ • Start operation                  │
   └────────────────────────────────────┘
```

---

## ⚠️ Common UART Issues

### Issue 1: Baud Rate Mismatch
```
Transmitter: 9600 baud
Receiver:   115200 baud

Result: Data is corrupted (garbage characters)

Solution: Match baud rates on both ends
```

### Issue 2: Frame Mismatch
```
Transmitter: 8N1 (8 bits, No parity, 1 stop)
Receiver:    7E1 (7 bits, Even parity, 1 stop)

Result: Frame errors, corrupted data

Solution: Match frame format on both ends
```

### Issue 3: Buffer Overflow
```
Receiver Buffer Full → New data lost

Solution:
- Use flow control
- Increase buffer size
- Process data faster
```

### Issue 4: Signal Levels
```
Transmitter: 5V TTL
Receiver:    3.3V TTL

Result: Possible damage to receiver

Solution: Use level shifters
```

### Issue 5: Missing Common Ground
```
No shared ground between devices

Result: Unstable communication

Solution: Connect GND lines
```

---

## 🎯 UART vs Other Protocols

| Feature | UART | SPI | I2C | USB |
|---------|------|-----|-----|-----|
| **Speed** | Low-Medium (115K-3M) | High (10M+) | Medium (100K-3.4M) | High (480M+) |
| **Wires** | 2 (TX/RX) | 4 (MOSI/MISO/SCK/SS) | 2 (SDA/SCL) | 4 (D+/D-/VCC/GND) |
| **Devices** | 2 (point-to-point) | Multiple | Multiple (addressing) | Multiple |
| **Clock** | No (Asynchronous) | Yes (Synchronous) | Yes (Synchronous) | Yes |
| **Protocol** | Simple | Moderate | Complex | Very Complex |
| **Flow Control** | Optional | No | No | Yes |
| **Error Detection** | Parity only | No | ACK/NACK | CRC |
| **Power** | No | No | No | Yes (VCC) |

---

## 💼 Interview Questions & Answers

### Q1: What is UART and how does it work?
**Answer:** UART (Universal Asynchronous Receiver-Transmitter) is a serial communication protocol. It works by:
- Transmitting data serially (one bit at a time)
- Using start/stop bits for frame synchronization
- Operating asynchronously (no shared clock)
- Using agreed baud rate for timing

### Q2: What does "asynchronous" mean in UART?
**Answer:** Asynchronous means there's no shared clock signal between devices. Instead, both devices agree on the baud rate (timing) in advance, and each frame contains start/stop bits for synchronization.

### Q3: What is the difference between RS-232 and UART?
**Answer:** 
- **UART**: Protocol/logic (data formatting, timing)
- **RS-232**: Physical layer (voltage levels, connectors)
- UART provides logic-level signals (0V/3.3V/5V)
- RS-232 provides voltage-level signals (±3V to ±15V)

### Q4: How do you calculate baud rate?
**Answer:** 
```
Baud Rate = Clock Frequency / (16 × BRR)

Where BRR is a configurable register divider.
Example: 16MHz clock, want 115200 baud
BRR = 16,000,000 / (16 × 115200) = 8.68
```

### Q5: What is the purpose of start and stop bits?
**Answer:**
- **Start Bit**: Marks beginning of frame, synchronizes receiver
- **Stop Bit(s)**: Marks end of frame, returns line to idle state
- Ensures receiver stays synchronized with transmitter

### Q6: What are common UART errors?
**Answer:**
- Framing error (stop bit incorrect)
- Parity error (parity mismatch)
- Overrun error (data lost)
- Buffer overflow (FIFO full)
- Break condition (line low too long)

### Q7: When would you use hardware vs software flow control?
**Answer:**
- **Hardware (RTS/CTS)**: For high speed, reliable, uses extra wires
- **Software (XON/XOFF)**: When no extra wires available, lower speed
- **None**: When speed is low enough to keep up

### Q8: What's the difference between UART and USART?
**Answer:** 
- **UART**: Universal Asynchronous Receiver-Transmitter (only async)
- **USART**: Universal Synchronous/Asynchronous Receiver-Transmitter (both sync and async)
- USART includes a clock output for synchronous mode

### Q9: How do you connect two UART devices?
**Answer:**
```
Device A TX → Device B RX
Device A RX ← Device B TX
Device A GND ← Device B GND (common ground)
```

### Q10: What factors affect maximum UART speed?
**Answer:**
- Clock frequency (higher clock = higher baud)
- Cable length and quality
- Signal levels (RS-232 vs TTL)
- Noise/interference
- Receiver/transmitter capability
- Wiring capacitance

---

## 📋 Quick Reference Card

```
UART QUICK REFERENCE
═══════════════════════════════════════════════════════════

DEFINITION:
┌─────────────────────────────────────────────────────────┐
│ Universal Asynchronous Receiver-Transmitter            │
│ Serial communication protocol                          │
│ Asynchronous, full-duplex, point-to-point              │
└─────────────────────────────────────────────────────────┘

FRAME (8N1):
┌─────────────────────────────────────────────────────────┐
│ Start (0) | Data (8 bits) | Stop (1)                   │
│ Bit Time = 1/Baud Rate                                 │
└─────────────────────────────────────────────────────────┘

COMMON SIGNALS:
┌─────────────────────────────────────────────────────────┐
│ TX    - Transmit Data                                  │
│ RX    - Receive Data                                   │
│ RTS   - Request To Send                               │
│ CTS   - Clear To Send                                 │
└─────────────────────────────────────────────────────────┘

BAUD RATES:
┌─────────────────────────────────────────────────────────┐
│ 9600   - Standard (most common)                       │
│ 115200 - High speed                                    │
│ 921600 - Very high speed                              │
│ 3M     - Maximum typical                              │
└─────────────────────────────────────────────────────────┘
```

---

## 🎓 Summary

1. **UART**: Universal Asynchronous Receiver-Transmitter
2. **Asynchronous**: No shared clock, uses agreed baud rate
3. **Frame**: Start bit + Data bits + (Parity) + Stop bit(s)
4. **Baud Rate**: Speed of communication
5. **Flow Control**: Prevents data loss (hardware/software)
6. **Use Cases**: Debugging, sensors, GPS, industrial
7. **Common Issues**: Baud mismatch, frame mismatch, overflow
8. **vs Others**: Simple but slower than SPI, I2C, USB

### Key Takeaways
- UART is simple and widely supported
- Match baud rate and frame format on both ends
- Always connect common ground
- Use flow control for reliability
- Understand signal levels (TTL vs RS-232)
- Perfect for debugging and simple communication

---

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 2. USART

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 3. SPI

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 4. I2C

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 5. CAN

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 6. CAN FD

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 7. LIN

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 8. FlexRay

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 9. RS-232

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 10. RS-485

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 11. USB

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 12. Ethernet

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---

## 13. GPIO

⬆️ **[Back to Table of Protocols](#table-of-protocols)**

[🔝 Back to Table of Contents](#table-of-contents)

---
