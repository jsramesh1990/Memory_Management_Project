# FIFO (First-In-First-Out) - Complete Guide

## 1. Definition

A **FIFO (First-In-First-Out)** is a data structure or hardware buffer where the first element inserted is the first one to be removed. It operates similarly to a real-world queue (e.g., people standing in line).

### Key Characteristics
- **Order preservation**
- **Two primary operations**: Write (enqueue) and Read (dequeue)
- **Two pointers**: Write pointer (head) and Read pointer (tail)
- **Commonly used** in data buffering, clock domain crossing, and pipeline management

---

## 2. Types of FIFO

### 2.1 Based on Implementation

| Type | Description |
|------|-------------|
| **Software FIFO** | Implemented in RAM using array/linked list (e.g., queues in C/Python) |
| **Hardware FIFO** | Dedicated hardware block inside FPGA/ASIC (using registers or block RAM) |

### 2.2 Based on Synchronization

| Type | Description |
|------|-------------|
| **Synchronous FIFO** | Same clock for read and write; simpler design |
| **Asynchronous FIFO** | Different clocks for read/write; used in clock domain crossing (CDC) |

### 2.3 Based on Memory Type

| Type | Description |
|------|-------------|
| **Register-based FIFO** | Small depth, fast, uses flip-flops |
| **Block RAM (BRAM) FIFO** | Large depth, efficient memory usage |
| **Distributed RAM FIFO** | Medium depth, balanced performance |

---

## 3. Working Flow

### 3.1 Basic Operations

```
+----------------+
|   Write (Push)  |  --> Data stored at write pointer
+----------------+
         |
         v
+----------------+
|   Read (Pop)    |  --> Data retrieved from read pointer
+----------------+
```

### 3.2 State Diagram

```
        RESET
          |
          v
      +-------+
      | EMPTY |<---------+
      +-------+          |
       |   ^             |
    Write | | Read       | Write (when full)
       v   |             |
      +-------+          |
      | PARTIAL|---------+
      +-------+
          |
          v
      +-------+
      |  FULL |
      +-------+
```

### 3.3 FIFO Flow Algorithm

```
Initialize: write_ptr = 0, read_ptr = 0, count = 0

WRITE Operation:
if (count < DEPTH):
    buffer[write_ptr] = data
    write_ptr = (write_ptr + 1) % DEPTH
    count = count + 1
else:
    flag FULL = 1

READ Operation:
if (count > 0):
    data = buffer[read_ptr]
    read_ptr = (read_ptr + 1) % DEPTH
    count = count - 1
else:
    flag EMPTY = 1
```

---

## 4. Block Diagrams

### 4.1 Synchronous FIFO Block Diagram

```
                    +-------------------------------------+
                    |              FIFO CORE              |
                    |                                     |
    wr_clk -------->|                                     |
    rd_clk -------->|    +---------+    +---------+       |
                    |    | Write   |    | Memory  |       |
    wr_en --------->|--->| Control +--->| Array   |       |
                    |    | & Ptr   |    | (RAM)   |       |
    rd_en --------->|--->| Logic   |<---+         |       |
                    |    |         |    |         |       |
    wr_data[7:0] -->|---->---------+    +----+----+       |
                    |                     |              |
    full  <---------|                     |              |
    empty <---------|    +---------+      |              |
    half <----------|    | Read    |<-----+              |
                    |    | Control |                     |
                    |    | & Ptr   |                     |
                    |    +---------+                     |
                    |                                     |
                    +-------------------------------------+
```

### 4.2 Asynchronous FIFO Block Diagram (with Gray Code)

```
    wr_clk domain                      rd_clk domain
    -------------                      -------------
    
    wr_data ---------+                                 +------> rd_data
                     |                                 |
                     v                                 |
    +---------------------+        +----------------+  |
    | Write Control       |        | Read Control   |  |
    | & Pointer (binary)  |        | & Pointer (bin)|  |
    +----------+----------+        +-------+--------+  |
               |                           |           |
               v                           v           |
          binary-->gray                binary-->gray   |
               |                           |           |
               v                           v           |
    +---------------------+        +----------------+  |
    | Gray Counter (wr_ptr)|        | Gray Counter   |  |
    +----------+----------+        | (rd_ptr)       |  |
               |                    +-------+--------+
               |                            |
               |       +------------+        |
               +------>| Sync to    |<-------+
                       | rd_clk     |
                       +------------+
                              |
                              v
                      Compare for Full/Empty
```

---

## 5. How to Add FIFO in BSP (Board Support Package)

### 5.1 For Embedded Systems (Example: STM32CubeMX / Zephyr / FreeRTOS)

#### Step 1: Include Headers
```c
#include "fifo.h"
#include "board.h"
```

#### Step 2: Define FIFO Configuration in BSP
```c
// bsp_fifo_config.h
#define FIFO_DEPTH      256
#define FIFO_WIDTH      8       // bits

// Choose FIFO type
#define FIFO_TYPE_SYNC   1
#define FIFO_TYPE_ASYNC  0
```

#### Step 3: Initialize FIFO in BSP Init
```c
// bsp_fifo.c
static uint8_t fifo_buffer[FIFO_DEPTH];
static fifo_handle_t g_uart_fifo;

void BSP_FIFO_Init(void)
{
    FIFO_Init(&g_uart_fifo, 
              fifo_buffer, 
              FIFO_DEPTH, 
              FIFO_WIDTH);
}
```

#### Step 4: Register with Driver Layer
```c
void BSP_UART_Init(void)
{
    // Link FIFO to UART RX interrupt
    UART_Register_RX_FIFO(&g_uart_fifo);
}
```

### 5.2 For FPGA BSP (Example: Xilinx Vivado / Intel Quartus)

**Method A: Using IP Catalog**
1. Open IP Catalog → FIFO Generator (Xilinx) or Dual Clock FIFO (Intel)
2. Configure:
   - FIFO Depth: 512
   - Data Width: 32 bits
   - Type: Synchronous / Asynchronous
   - Flags: Full, Empty, Almost Full/Empty
3. Generate IP and add to BSP
4. Connect to AXI/Avalon bus

**Method B: Manual RTL Instantiation in BSP Top**
```verilog
// bsp_fifo.v
module bsp_fifo_inst (
    input        wr_clk, rd_clk,
    input        wr_en, rd_en,
    input [7:0]  wr_data,
    output [7:0] rd_data,
    output       full, empty
);

// Instantiate vendor-specific FIFO
xpm_fifo_sync #(
    .FIFO_MEMORY_TYPE("block"),
    .FIFO_WRITE_DEPTH(512),
    .WRITE_DATA_WIDTH(8)
) u_fifo (
    .wr_clk(wr_clk),
    .wr_en(wr_en),
    .din(wr_data),
    .full(full),
    .rd_en(rd_en),
    .dout(rd_data),
    .empty(empty)
);
endmodule
```

---

## 6. How to Maintain FIFO

### 6.1 Runtime Maintenance Checklist

| Task | Frequency | Method |
|------|-----------|--------|
| Check overflow | Every write | Monitor `full` flag before push |
| Check underflow | Every read | Monitor `empty` flag before pop |
| Reset on error | On fault | Assert reset signal |
| Watermark monitoring | Periodic | Read `count` or `almost_full/empty` |

### 6.2 Code Example: Safe FIFO Maintenance
```c
// Safe write with overflow prevention
bool FIFO_SafeWrite(FIFO_Handle *fifo, uint8_t data)
{
    if (FIFO_IsFull(fifo)) {
        // Log error
        error_counter++;
        // Optional: overwrite oldest (overwrite mode)
        if (fifo->overwrite_enable) {
            FIFO_ForceWrite(fifo, data);
        }
        return false;
    }
    FIFO_Write(fifo, data);
    return true;
}

// Periodic health check task
void FIFO_MaintenanceTask(void *params)
{
    while(1) {
        uint32_t fill_level = FIFO_GetCount(&g_fifo);
        if (fill_level > FIFO_WATERMARK_HIGH) {
            // Signal overflow risk
            event_post(EVENT_FIFO_NEAR_FULL);
        }
        if (fill_level < FIFO_WATERMARK_LOW) {
            // Prefetch more data
            BSP_DMA_StartPrefetch();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 6.3 Debugging & Monitoring

```c
// Dump FIFO status for debugging
void FIFO_DumpStatus(FIFO_Handle *fifo)
{
    printf("=== FIFO Status ===\n");
    printf("Depth: %d\n", fifo->depth);
    printf("Count: %d\n", FIFO_GetCount(fifo));
    printf("Empty: %d\n", FIFO_IsEmpty(fifo));
    printf("Full:  %d\n", FIFO_IsFull(fifo));
    printf("Overflows:  %lu\n", fifo->overflow_cnt);
    printf("Underflows: %lu\n", fifo->underflow_cnt);
}
```

### 6.4 Common Issues & Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| Data loss | Write when full | Increase depth or check `full` before write |
| Stale data | Read when empty | Check `empty` flag or use peek operation |
| Metastability (async FIFO) | Poor synchronization | Use Gray code + multi-flop sync |
| Deadlock | Wrong pointer reset | Ensure both sides reset together |

---

## 7. Performance Considerations

### 7.1 Depth Selection Formula
```
FIFO Depth = (Bandwidth_mismatch * Max_burst_length) / Clock_ratio
```

### 7.2 Typical Use Cases & Depths

| Application | Typical Depth | Type |
|-------------|---------------|------|
| UART RX buffer | 16-256 bytes | Sync |
| Ethernet packet buffer | 2K-16K bytes | Async |
| Video line buffer | 1920-4096 pixels | Sync |
| Audio sample buffer | 128-2048 samples | Sync |

---

## 8. Code Templates

### 8.1 Simple Circular FIFO in C
```c
typedef struct {
    uint8_t *buffer;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t depth;
} fifo_t;

void fifo_init(fifo_t *f, uint8_t *buf, uint16_t depth) {
    f->buffer = buf;
    f->head = 0;
    f->tail = 0;
    f->count = 0;
    f->depth = depth;
}

bool fifo_push(fifo_t *f, uint8_t data) {
    if (f->count >= f->depth) return false;
    f->buffer[f->head] = data;
    f->head = (f->head + 1) % f->depth;
    f->count++;
    return true;
}

bool fifo_pop(fifo_t *f, uint8_t *data) {
    if (f->count == 0) return false;
    *data = f->buffer[f->tail];
    f->tail = (f->tail + 1) % f->depth;
    f->count--;
    return true;
}
```

### 8.2 Verilog Synchronous FIFO Template
```verilog
module sync_fifo #(parameter DEPTH=8, WIDTH=8) (
    input clk, rst,
    input wr_en, rd_en,
    input [WIDTH-1:0] wr_data,
    output reg [WIDTH-1:0] rd_data,
    output full, empty
);
    reg [$clog2(DEPTH)-1:0] wr_ptr, rd_ptr;
    reg [WIDTH-1:0] mem [0:DEPTH-1];
    reg [$clog2(DEPTH):0] count;
    
    assign full = (count == DEPTH);
    assign empty = (count == 0);
    
    always @(posedge clk) begin
        if (rst) begin
            wr_ptr <= 0; rd_ptr <= 0; count <= 0;
        end else begin
            if (wr_en && !full) begin
                mem[wr_ptr] <= wr_data;
                wr_ptr <= wr_ptr + 1;
                count <= count + 1;
            end
            if (rd_en && !empty) begin
                rd_data <= mem[rd_ptr];
                rd_ptr <= rd_ptr + 1;
                count <= count - 1;
            end
        end
    end
endmodule
```

