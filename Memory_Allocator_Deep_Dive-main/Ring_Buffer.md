# Ring Buffer (Circular Buffer) 

## 1. What is a Ring Buffer?

### Simple Definition
A **Ring Buffer** (also called Circular Buffer) is a fixed-size memory area that connects end-to-end, like a circle. When you reach the end, you automatically go back to the beginning.

### Real-World Analogy
Imagine a **circular parking lot** with 10 spaces:
- Cars enter and park in empty spaces
- When the lot is full, new cars either wait or replace the oldest car
- The first car to enter is the first to leave (FIFO)

```
Traditional Array (Linear):     Ring Buffer (Circular):
[0][1][2][3][4][5]              
                                ┌─[0]─┐
[0][1][2][3][4][5][6][7]       [7]   [1]
                                │     │
Linear: End stops here          [6]   [2]
                                │     │
                                └─[5]─[3]┘
                                   [4]
                            Wraps around!
```

---

## 2. Why Create a Ring Buffer?

### Problems It Solves

| Problem | Traditional Solution | Ring Buffer Solution |
|---------|---------------------|---------------------|
| **Memory shifting** | Moving all elements when removing first item (slow) | Just move pointer (fast) |
| **Fixed memory** | Dynamic allocation (risk of fragmentation) | Static allocation (predictable) |
| **Real-time data** | Blocking operations | Lock-free possible |
| **Producer-Consumer** | Complex synchronization | Simple with pointers |

### Key Benefits for Embedded Systems

 **No Dynamic Memory Allocation** - Critical for safety-critical systems  
 **Predictable Performance** - O(1) for read/write operations  
 **Memory Efficient** - Fixed size, no overhead  
 **Thread-Safe Possible** - Can use with simple locks  
 **No Memory Fragmentation** - Static allocation  

### Common Applications

```
UART Communication    → Buffer incoming/outgoing bytes
ADC Sampling         → Store continuous sensor data
Audio Processing     → Circular audio samples
Logging Systems      → Store recent log messages
USB/SPI/I2C Drivers  → Data buffering
Command Queues       → Store user commands
```

---

## 3. Syntax and Structure

### Basic Structure Definition

```c
typedef struct {
    uint8_t *buffer;     // Pointer to storage array
    int head;           // Write position (where to add next)
    int tail;           // Read position (where to remove next)
    int size;           // Total capacity
    int count;          // Current number of items
} ring_buffer_t;
```

### Variable Naming Conventions

| Variable | Other Names | Purpose |
|----------|------------|---------|
| `head` | `write_ptr`, `in`, `producer` | Points to next write location |
| `tail` | `read_ptr`, `out`, `consumer` | Points to next read location |
| `count` | `len`, `used`, `available` | Number of items currently stored |
| `size` | `capacity`, `max` | Maximum items buffer can hold |

---

## 4. How to Create

### Step-by-Step Creation

#### Step 1: Choose Buffer Size
```c
#define BUFFER_SIZE 32  // Must be power of 2 for optimization
```

#### Step 2: Create Storage Array
```c
uint8_t storage[BUFFER_SIZE];  // Static array
```

#### Step 3: Initialize Structure
```c
ring_buffer_t my_buffer;

void init_buffer(void) {
    my_buffer.buffer = storage;
    my_buffer.size = BUFFER_SIZE;
    my_buffer.head = 0;
    my_buffer.tail = 0;
    my_buffer.count = 0;
}
```

### Complete Creation Example

```c
// Method 1: Static (Recommended for embedded)
#define RB_SIZE 64
static uint8_t rb_storage[RB_SIZE];
static ring_buffer_t static_rb;

void create_static_ring_buffer(void) {
    static_rb.buffer = rb_storage;
    static_rb.size = RB_SIZE;
    static_rb.head = 0;
    static_rb.tail = 0;
    static_rb.count = 0;
}

// Method 2: With Helper Function
void ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, int size) {
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

// Usage
ring_buffer_t my_rb;
uint8_t my_data[128];
ring_buffer_init(&my_rb, my_data, 128);
```

---

## 5. Working Principle

### Core Operations

#### 1. **Adding Data (Write/Push/Enqueue)**

```c
bool write_to_buffer(ring_buffer_t *rb, uint8_t data) {
    // Check if full
    if (rb->count >= rb->size) {
        return false;  // Buffer full
    }
    
    // Write data at head position
    rb->buffer[rb->head] = data;
    
    // Move head forward (wrap if needed)
    rb->head = (rb->head + 1) % rb->size;
    
    // Increment count
    rb->count++;
    
    return true;
}
```

#### 2. **Reading Data (Read/Pop/Dequeue)**

```c
bool read_from_buffer(ring_buffer_t *rb, uint8_t *data) {
    // Check if empty
    if (rb->count == 0) {
        return false;  // Buffer empty
    }
    
    // Read data from tail position
    *data = rb->buffer[rb->tail];
    
    // Move tail forward (wrap if needed)
    rb->tail = (rb->tail + 1) % rb->size;
    
    // Decrement count
    rb->count--;
    
    return true;
}
```

#### 3. **The Magic of Modulo Operator `%`**

```c
// Without modulo (manual wrap)
int new_head = head + 1;
if (new_head >= size) {
    new_head = 0;
}

// With modulo (automatic wrap)
int new_head = (head + 1) % size;  // Much cleaner!
```

### Visual Working Example

```
Size = 8 buffer
Initial state: [ _  _  _  _  _  _  _  _ ]
                H=0,T=0, Count=0

Write 3 items:  [ A  B  C  _  _  _  _  _ ]
                T=0     H=3, Count=3

Read 2 items:   [ A  B  C  _  _  _  _  _ ]
                   T=2     H=3, Count=1

Write 2 more:   [ A  B  C  D  E  _  _  _ ]
                   T=2        H=5, Count=3

Read 1 item:    [ A  B  C  D  E  _  _  _ ]
                      T=3     H=5, Count=2

Write 6 items (buffer fills): [ F  G  H  D  E  I  J  K ]
                              T=3           H=2 (wrapped!)
                              Count=8 (FULL)

Read all:       Values come out: D, E, F, G, H, I, J, K
```

---

## 6. Flow Diagrams

### Write Operation Flow

```
START
  │
  ▼
Is buffer full? ──YES──▶ Return FAIL (or overwrite oldest)
  │ NO
  ▼
Write data at buffer[head]
  │
  ▼
head = (head + 1) % size
  │
  ▼
count++
  │
  ▼
Return SUCCESS
```

### Read Operation Flow

```
START
  │
  ▼
Is buffer empty? ──YES──▶ Return FAIL
  │ NO
  ▼
Read data = buffer[tail]
  │
  ▼
tail = (tail + 1) % size
  │
  ▼
count--
  │
  ▼
Return data and SUCCESS
```

### State Transitions

```
        Write                     Read
    ┌──────────┐             ┌──────────┐
    │          ▼             │          ▼
Empty ──▶ Partial ──▶ Full ──▶ Partial ──▶ Empty
         ◀──────            ◀──────
          Read                Write
```

---

## 7. Complete Code Examples

### Example 1: Basic Integer Buffer

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 5

typedef struct {
    int buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
} simple_ring_t;

void init(simple_ring_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool write(simple_ring_t *rb, int value) {
    if (rb->count >= BUFFER_SIZE) {
        printf("Buffer full! Can't write %d\n", value);
        return false;
    }
    
    rb->buffer[rb->head] = value;
    rb->head = (rb->head + 1) % BUFFER_SIZE;
    rb->count++;
    
    printf("Wrote: %d, Count: %d\n", value, rb->count);
    return true;
}

bool read(simple_ring_t *rb, int *value) {
    if (rb->count == 0) {
        printf("Buffer empty!\n");
        return false;
    }
    
    *value = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BUFFER_SIZE;
    rb->count--;
    
    printf("Read: %d, Count: %d\n", *value, rb->count);
    return true;
}

void print_buffer(simple_ring_t *rb) {
    printf("Buffer: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf(" %d ", rb->buffer[i]);
    }
    printf("] H=%d, T=%d, C=%d\n", rb->head, rb->tail, rb->count);
}

int main() {
    simple_ring_t rb;
    init(&rb);
    int value;
    
    printf("=== Ring Buffer Demo ===\n");
    print_buffer(&rb);
    
    write(&rb, 10);
    write(&rb, 20);
    write(&rb, 30);
    write(&rb, 40);
    write(&rb, 50);
    print_buffer(&rb);
    
    write(&rb, 60);  // This will fail (full)
    
    read(&rb, &value);
    read(&rb, &value);
    print_buffer(&rb);
    
    write(&rb, 60);
    write(&rb, 70);
    print_buffer(&rb);
    
    return 0;
}
```

**Output:**
```
=== Ring Buffer Demo ===
Buffer: [ 0  0  0  0  0 ] H=0, T=0, C=0
Wrote: 10, Count: 1
Wrote: 20, Count: 2
Wrote: 30, Count: 3
Wrote: 40, Count: 4
Wrote: 50, Count: 5
Buffer: [ 10  20  30  40  50 ] H=0, T=0, C=5
Buffer full! Can't write 60
Read: 10, Count: 4
Read: 20, Count: 3
Buffer: [ 10  20  30  40  50 ] H=0, T=2, C=3
Wrote: 60, Count: 4
Wrote: 70, Count: 5
Buffer: [ 60  70  30  40  50 ] H=2, T=2, C=5
```

### Example 2: Embedded UART Ring Buffer

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 128

typedef struct {
    uint8_t buffer[RX_BUFFER_SIZE];
    volatile uint16_t head;     // Volatile for ISR access
    volatile uint16_t tail;
    volatile uint16_t count;
} uart_ring_t;

static uart_ring_t rx_ring;

// Initialize UART ring buffer
void uart_rx_init(void) {
    rx_ring.head = 0;
    rx_ring.tail = 0;
    rx_ring.count = 0;
}

// Called from UART ISR (Interrupt Service Routine)
void uart_rx_isr(uint8_t received_byte) {
    uint16_t next_head = (rx_ring.head + 1) % RX_BUFFER_SIZE;
    
    if (next_head != rx_ring.tail) {  // Not full
        rx_ring.buffer[rx_ring.head] = received_byte;
        rx_ring.head = next_head;
        rx_ring.count++;
    } else {
        // Buffer full - handle overflow
        // Could set error flag or overwrite oldest
    }
}

// Called from main loop to get data
bool uart_get_byte(uint8_t *byte) {
    if (rx_ring.tail == rx_ring.head) {
        return false;  // Empty
    }
    
    *byte = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) % RX_BUFFER_SIZE;
    rx_ring.count--;
    return true;
}

// Check how many bytes available
uint16_t uart_bytes_available(void) {
    return rx_ring.count;
}

// Example usage with interrupt safety
void main_loop(void) {
    uint8_t data;
    
    while(1) {
        // Disable interrupts while checking/reading
        __disable_irq();
        if (uart_bytes_available() > 0) {
            uart_get_byte(&data);
            __enable_irq();
            
            // Process received byte
            process_uart_data(data);
        } else {
            __enable_irq();
        }
        
        // Do other tasks
    }
}
```

### Example 3: Power of 2 Optimization

```c
// When size is power of 2 (e.g., 64, 128, 256, 512)
// Use bitwise AND instead of modulo (faster!)

#define BUFFER_SIZE 64   // 2^6
#define BUFFER_MASK (BUFFER_SIZE - 1)  // 63

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
} fast_ring_t;

// Fast write (no modulo, uses bitwise AND)
void fast_write(fast_ring_t *rb, uint8_t data) {
    uint8_t new_head = (rb->head + 1) & BUFFER_MASK;
    
    if (new_head != rb->tail) {  // Not full
        rb->buffer[rb->head] = data;
        rb->head = new_head;
    }
}

// Fast read
bool fast_read(fast_ring_t *rb, uint8_t *data) {
    if (rb->head == rb->tail) {
        return false;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & BUFFER_MASK;
    return true;
}
```

### Example 4: Overwrite Mode (Always Keep Latest)

```c
typedef struct {
    uint8_t *buffer;
    int head;
    int tail;
    int size;
    int count;
    bool overwrite;  // New: Overwrite oldest when full
} overwrite_ring_t;

bool write_with_overwrite(overwrite_ring_t *rb, uint8_t data) {
    int new_head = (rb->head + 1) % rb->size;
    
    if (new_head == rb->tail) {
        if (rb->overwrite) {
            // Overwrite: move tail forward
            rb->tail = (rb->tail + 1) % rb->size;
            rb->count--;
        } else {
            return false;
        }
    }
    
    rb->buffer[rb->head] = data;
    rb->head = new_head;
    rb->count++;
    return true;
}
```

---

## 8. Real-World Use Cases

### Use Case 1: Serial Communication Logger

```c
// Keep last 1000 debug messages
#define LOG_SIZE 1000

typedef struct {
    char messages[LOG_SIZE][64];  // 1000 messages of 64 chars
    int head;
    int tail;
    int count;
} log_ring_t;

log_ring_t debug_log;

void log_message(const char *msg) {
    strncpy(debug_log.messages[debug_log.head], msg, 63);
    debug_log.messages[debug_log.head][63] = '\0';
    
    debug_log.head = (debug_log.head + 1) % LOG_SIZE;
    if (debug_log.count < LOG_SIZE) {
        debug_log.count++;
    } else {
        debug_log.tail = (debug_log.tail + 1) % LOG_SIZE;
    }
}

void print_all_logs(void) {
    int i = debug_log.tail;
    int count = debug_log.count;
    
    while (count > 0) {
        printf("%s\n", debug_log.messages[i]);
        i = (i + 1) % LOG_SIZE;
        count--;
    }
}
```

### Use Case 2: Audio Sample Buffer

```c
#define SAMPLE_RATE 44100
#define BUFFER_MS 20
#define AUDIO_BUFFER_SIZE (SAMPLE_RATE * BUFFER_MS / 1000)  // 882 samples

typedef struct {
    int16_t left[AUDIO_BUFFER_SIZE];
    int16_t right[AUDIO_BUFFER_SIZE];
    int write_index;
    int read_index;
} audio_buffer_t;

void audio_callback(audio_buffer_t *buf, int16_t *output, int samples) {
    for (int i = 0; i < samples; i++) {
        if (buf->read_index != buf->write_index) {
            output[i] = buf->left[buf->read_index];
            buf->read_index = (buf->read_index + 1) % AUDIO_BUFFER_SIZE;
        } else {
            output[i] = 0;  // Underflow
        }
    }
}
```

### Use Case 3: Task Scheduler Queue

```c
typedef struct {
    void (*task)(void *);
    void *param;
    uint32_t execute_time;
} task_t;

#define TASK_QUEUE_SIZE 32

typedef struct {
    task_t queue[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} task_queue_t;

bool schedule_task(task_queue_t *tq, void (*task)(void*), void *param, uint32_t delay_ms) {
    if (tq->count >= TASK_QUEUE_SIZE) {
        return false;
    }
    
    tq->queue[tq->head].task = task;
    tq->queue[tq->head].param = param;
    tq->queue[tq->head].execute_time = get_tick() + delay_ms;
    
    tq->head = (tq->head + 1) % TASK_QUEUE_SIZE;
    tq->count++;
    return true;
}
```

---

## 9. Common Pitfalls and Solutions

### Pitfall 1: Full vs Empty Ambiguity

**Problem:** When head == tail, buffer could be full OR empty.

**Solutions:**

```c
// Method 1: Use count variable (simplest)
typedef struct {
    // ... other members
    int count;  // Track actual items
} safe_ring_t;

// Method 2: Leave one slot empty
#define BUFFER_SIZE 16
typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    int head;
    int tail;
} one_slot_ring_t;

bool is_full(one_slot_ring_t *rb) {
    return ((rb->head + 1) % BUFFER_SIZE) == rb->tail;
}

bool is_empty(one_slot_ring_t *rb) {
    return rb->head == rb->tail;
}
```

### Pitfall 2: Concurrency Issues

**Problem:** Interrupts can corrupt buffer state.

**Solution:** Use critical sections

```c
// For bare-metal embedded
volatile ring_buffer_t uart_rb;

void ISR_Handler(void) {
    uint8_t data = UART->DR;
    
    // Single write - safe if head is volatile
    uint16_t next = (uart_rb.head + 1) % SIZE;
    if (next != uart_rb.tail) {
        uart_rb.buffer[uart_rb.head] = data;
        uart_rb.head = next;
    }
}

// In main code - disable interrupts when accessing shared data
void main_process(void) {
    __disable_irq();
    uint8_t data;
    if (uart_rb.head != uart_rb.tail) {
        data = uart_rb.buffer[uart_rb.tail];
        uart_rb.tail = (uart_rb.tail + 1) % SIZE;
    }
    __enable_irq();
}
```

### Pitfall 3: Buffer Overflow

**Solution:** Always check before writing

```c
// Safe write function
bool safe_write(ring_buffer_t *rb, uint8_t data) {
    if ((rb->head + 1) % rb->size == rb->tail) {
        // Set overflow flag
        rb->overflow = true;
        return false;
    }
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;
    return true;
}
```

---

## 10. Quick Reference Card

### Buffer States

| State | Condition | Head vs Tail | Can Read? | Can Write? |
|-------|-----------|--------------|-----------|------------|
| Empty | count == 0 | head == tail | No | Yes |
| Partial | 0 < count < size | Different | Yes | Yes |
| Full | count == size | head == tail | Yes | No |

### Operation Complexity

| Operation | Time Complexity | Memory Access |
|-----------|----------------|---------------|
| Write | O(1) | 1 write |
| Read | O(1) | 1 read |
| Peek | O(1) | 1 read |
| Clear | O(n) | n writes |

### When to Use vs Not Use

** Use Ring Buffer When:**
- Fixed-size data queue needed
- Real-time constraints (no malloc)
- Producer-consumer pattern
- Continuous data streams
- Memory is limited

** Don't Use When:**
- Variable size needed
- Random access required
- Need to search data
- Very large size (inefficient wrap)

---

## 11. Summary

### Key Takeaways

1. **Ring Buffer = Circular FIFO** using fixed memory
2. **Two pointers**: Head (write) and Tail (read)
3. **Wrap around** using modulo `%` or bitwise `&`
4. **No memory allocation** → Perfect for embedded systems
5. **O(1) operations** → Fast and predictable
6. **Prevents buffer overflow** with proper checks

### Memory Formula
```
Memory needed = buffer_size × element_size + structure_overhead
Example: 100 × 4 bytes = 400 bytes + ~16 bytes overhead
```

### Performance Tips
- Use power of 2 sizes for bitwise operations
- Make head/tail volatile if used in ISRs
- Consider cache line alignment for large buffers
- Use DMA with ring buffers for high performance

---

## 12. Complete Working Example

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Configuration
#define RING_BUFFER_SIZE 8
#define RING_BUFFER_TYPE uint8_t

// Structure definition
typedef struct {
    RING_BUFFER_TYPE buffer[RING_BUFFER_SIZE];
    volatile int head;
    volatile int tail;
    volatile int count;
} ring_buffer_t;

// Initialize
void ring_init(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    memset((void*)rb->buffer, 0, RING_BUFFER_SIZE);
}

// Write single byte
bool ring_write(ring_buffer_t *rb, RING_BUFFER_TYPE data) {
    if (rb->count >= RING_BUFFER_SIZE) {
        return false;
    }
    
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;
    rb->count++;
    return true;
}

// Read single byte
bool ring_read(ring_buffer_t *rb, RING_BUFFER_TYPE *data) {
    if (rb->count == 0) {
        return false;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    rb->count--;
    return true;
}

// Check status
bool ring_is_empty(ring_buffer_t *rb) { return rb->count == 0; }
bool ring_is_full(ring_buffer_t *rb) { return rb->count >= RING_BUFFER_SIZE; }
int ring_available(ring_buffer_t *rb) { return rb->count; }
int ring_free_space(ring_buffer_t *rb) { return RING_BUFFER_SIZE - rb->count; }

// Display buffer
void ring_display(ring_buffer_t *rb) {
    printf("[");
    for (int i = 0; i < RING_BUFFER_SIZE; i++) {
        if (i == rb->head) printf(" H");
        if (i == rb->tail) printf(" T");
        printf(" %02d", rb->buffer[i]);
    }
    printf(" ] C=%d\n", rb->count);
}

// Demo
int main() {
    ring_buffer_t rb;
    ring_init(&rb);
    uint8_t data;
    
    printf("========== RING BUFFER DEMO ==========\n\n");
    
    printf("Writing 5 values...\n");
    for (int i = 1; i <= 5; i++) {
        ring_write(&rb, i * 10);
        printf("Wrote: %d\n", i * 10);
        ring_display(&rb);
    }
    
    printf("\nReading 3 values...\n");
    for (int i = 0; i < 3; i++) {
        ring_read(&rb, &data);
        printf("Read: %d\n", data);
        ring_display(&rb);
    }
    
    printf("\nWriting 6 more values...\n");
    for (int i = 1; i <= 6; i++) {
        if (ring_write(&rb, i * 5)) {
            printf("Wrote: %d\n", i * 5);
        } else {
            printf("Failed to write: %d (buffer full)\n", i * 5);
        }
        ring_display(&rb);
    }
    
    printf("\nReading remaining values...\n");
    while (!ring_is_empty(&rb)) {
        ring_read(&rb, &data);
        printf("Read: %d\n", data);
        ring_display(&rb);
    }
    
    return 0;
}
```
