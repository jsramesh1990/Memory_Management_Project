# Embedded Memory Management

## Overview

Embedded systems have unique memory constraints that make traditional dynamic allocation dangerous or impossible. Understanding embedded memory management is crucial for real-time systems, IoT devices, and safety-critical applications.

## Why Dynamic Allocation is Discouraged in Embedded

```
THE PROBLEM WITH MALLOC IN EMBEDDED SYSTEMS
═══════════════════════════════════════════════════════════════════

Traditional malloc/free problems in embedded context:

┌─────────────────────────────────────────────────────────────────┐
│  PROBLEM 1: Fragmentation                                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ After running for weeks:                                │   │
│  │ [32B][free][64B][free][128B][free][256B][free]         │   │
│  │   ↑      ↑      ↑       ↑                               │   │
│  │  Small holes everywhere! Cannot allocate 300B!         │   │
│  │  System crashes even though 480B total free!           │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  PROBLEM 2: Non-deterministic Timing                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ malloc(64) times:                                       │   │
│  │ • First call: 200ns                                     │   │
│  │ • Second call: 150ns                                    │   │
│  │ • Third call: 5000ns ← coalescing, list traversal!     │   │
│  │ • Fourth call: 180ns                                    │   │
│  │                                                         │   │
│  │ ❌ CANNOT GUARANTEE DEADLINE!                           │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  PROBLEM 3: Allocation Failures                               │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ • No virtual memory to fall back on                     │   │
│  │ • No swap space                                          │   │
│  │ • malloc returns NULL → what now?                       │   │
│  │ • Medical device: NULL → patient monitoring stops!      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  PROBLEM 4: Memory Leaks                                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ • No OS to clean up on process exit                     │   │
│  │ • Device runs for years                                 │   │
│  │ • Leak 8 bytes per day → 2.9KB/year→ eventual crash    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  PROBLEM 5: Stack Overflow Risk                               │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ • Small stack (typically 2-8KB)                         │   │
│  │ • Recursion dangerous                                   │   │
│  │ • No guard pages (wastes memory)                        │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Embedded Memory Types

### Memory Hierarchy

```
EMBEDDED MEMORY ARCHITECTURE
═══════════════════════════════════════════════════════════════════

                    ┌─────────────────────────────────────┐
                    │        CPU Registers                │
                    │        (32-64 bytes)                │
                    │        Speed: 1 cycle               │
                    └──────────────┬──────────────────────┘
                                   │
                    ┌──────────────▼──────────────────────┐
                    │        L1 Cache (if present)        │
                    │        (4-32KB)                     │
                    │        Speed: 2-3 cycles            │
                    └──────────────┬──────────────────────┘
                                   │
                    ┌──────────────▼──────────────────────┐
                    │        Internal SRAM                │
                    │        (32-512KB)                   │
                    │        Speed: 1-2 cycles            │
                    │        ↓ CRITICAL ↓                  │
                    └──────────────┬──────────────────────┘
                                   │
                    ┌──────────────▼──────────────────────┐
                    │        External RAM (SDRAM)         │
                    │        (1-64MB)                     │
                    │        Speed: 10-50 cycles          │
                    └──────────────┬──────────────────────┘
                                   │
                    ┌──────────────▼──────────────────────┐
                    │        Flash (Code Storage)         │
                    │        (64KB-2MB)                   │
                    │        Speed: 50-100 cycles (read)  │
                    │        Write: very slow             │
                    └─────────────────────────────────────┘

TYPICAL MCU MEMORY MAP (STM32F4):
┌─────────────────────────────────────────────────────────────────┐
│  Address        | Size    | Type    | Description              │
│  ───────────────────────────────────────────────────────────────│
│  0x0000 0000    | 1MB     | Flash   | Code storage             │
│  0x2000 0000    | 128KB   | SRAM    | Data + heap + stack      │
│  0x2000 0000    | 112KB   | SRAM1   | Main RAM                 │
│  0x2001 C000    | 16KB    | SRAM2   | USB/CAN buffers          │
│  0x4000 0000    | 512MB   | MMIO    | Peripherals              │
└─────────────────────────────────────────────────────────────────┘
```

### Memory Section Attributes

```c
// Different memory regions in embedded C
// Code in flash (read-only)
const char firmware_version[] __attribute__((section(".rodata"))) = "1.0.0";

// Initialized data goes to RAM, initialized from flash
int initialized_var = 42;  // .data section

// Uninitialized data (zero-initialized at startup)
int uninit_var;  // .bss section

// Place in specific RAM region (STM32 example)
__attribute__((section(".ccmram"))) int critical_buffer[1024];
// This goes to Core-Coupled Memory (faster access)

// Place in external SDRAM
__attribute__((section(".sdram"))) uint8_t framebuffer[320 * 240 * 2];

// Keep variable in register (compiler hint)
register int fast_counter;

// Force alignment for DMA
__attribute__((aligned(32))) uint8_t dma_buffer[1024];
```

## Static Allocation Strategies

### Compile-Time Allocation

```c
// 1. Global/Static arrays (most common)
static uint8_t heap_pool[32 * 1024];  // 32KB fixed pool
static message_t message_queue[100];   // 100 messages
static int task_stack[TASK_STACK_SIZE]; // Task stack

// 2. Linker script sections
// In linker script:
// .my_section : {
//   _my_section_start = .;
//   *(.my_section)
//   _my_section_end = .;
// } >RAM

__attribute__((section(".my_section"))) uint8_t special_pool[4096];

// 3. Const data (stays in flash)
const struct config system_config = {
    .baudrate = 115200,
    .timeout_ms = 1000
};  // Never copied to RAM!

// 4. Zero-initialized (no flash storage)
static uint8_t zerod_data[1024];  // .bss, no space in binary

// 5. Explicit placement
#define MY_BUFFER_ADDR 0x20000000
uint8_t* my_buffer = (uint8_t*)MY_BUFFER_ADDR;  // Fixed address
```

### Static Allocation Visual

```
STATIC ALLOCATION LAYOUT
═══════════════════════════════════════════════════════════════════

Binary layout in Flash (permanent storage):
┌─────────────────────────────────────────────────────────────────┐
│  .vectors      │ .text (code)   │ .rodata   │ .data (init)     │
│  (exception    │                │ (const    │ (initialized     │
│   handlers)    │                │  data)    │  data)           │
└─────────────────────────────────────────────────────────────────┘
                                   ↓
                            Copied to RAM at boot
                                   ↓
RAM layout at runtime:
┌─────────────────────────────────────────────────────────────────┐
│  .data (init values)  │ .bss (zero)    │ Heap     │ Stack      │
│  (from flash)         │                │ (optional)│            │
└─────────────────────────────────────────────────────────────────┘
         ↓                       ↓
    Fixed size!             Fixed size!

Advantages:
✓ No fragmentation
✓ Predictable memory usage
✓ Fast (no allocation overhead)
✓ Known at compile time (can check bounds)
✓ No runtime failures possible
```

## Memory Pools (Fixed-Block Allocator)

### Pool Implementation

```c
// Memory Pool Implementation - Zero Fragmentation
#include <stdint.h>
#include <stdbool.h>

// Pool configuration
#define POOL_BLOCK_SIZE 64
#define POOL_BLOCK_COUNT 100

// Pool structure
typedef struct {
    uint8_t memory[POOL_BLOCK_SIZE * POOL_BLOCK_COUNT];
    uint16_t free_stack[POOL_BLOCK_COUNT];  // Stack of free indices
    uint16_t free_count;
    uint16_t magic;  // Integrity check
} memory_pool_t;

// Initialize pool
void pool_init(memory_pool_t* pool) {
    pool->free_count = POOL_BLOCK_COUNT;
    pool->magic = 0x5A5A;
    
    // Initialize free stack (LIFO for cache efficiency)
    for (uint16_t i = 0; i < POOL_BLOCK_COUNT; i++) {
        pool->free_stack[i] = POOL_BLOCK_COUNT - 1 - i;  // Reverse order
    }
}

// O(1) allocation
void* pool_alloc(memory_pool_t* pool) {
    if (pool->free_count == 0) {
        return NULL;  // Pool exhausted
    }
    
    // Pop from free stack
    uint16_t index = pool->free_stack[--pool->free_count];
    return &pool->memory[index * POOL_BLOCK_SIZE];
}

// O(1) free
void pool_free(memory_pool_t* pool, void* ptr) {
    // Validate pointer belongs to pool
    if (ptr < pool->memory || ptr >= pool->memory + sizeof(pool->memory)) {
        return;  // Invalid pointer
    }
    
    // Calculate index
    uint16_t index = ((uint8_t*)ptr - pool->memory) / POOL_BLOCK_SIZE;
    
    // Push onto free stack
    pool->free_stack[pool->free_count++] = index;
}

// Check pool integrity
bool pool_validate(memory_pool_t* pool) {
    return pool->magic == 0x5A5A;
}

// Usage example
static memory_pool_t message_pool;

void init_messages(void) {
    pool_init(&message_pool);
}

void* get_message(void) {
    return pool_alloc(&message_pool);
}

void return_message(void* msg) {
    pool_free(&message_pool, msg);
}
```

### Pool Memory Visualization

```
MEMORY POOL VISUALIZATION
═══════════════════════════════════════════════════════════════════

Pool with 10 blocks of 64 bytes:

Physical memory layout:
┌─────────────────────────────────────────────────────────────────┐
│ Block 0 │ Block 1 │ Block 2 │ Block 3 │ ... │ Block 9         │
│ [64B]   │ [64B]   │ [64B]   │ [64B]   │     │ [64B]           │
└─────────────────────────────────────────────────────────────────┘

Free stack after initialization (LIFO):
free_stack[0..9] = [9,8,7,6,5,4,3,2,1,0]
free_count = 10

After 3 allocations:
┌─────────────────────────────────────────────────────────────────┐
│ Block 9 │ Block 8 │ Block 7 │ Block 6 │ ... │ Block 0         │
│ [USED]  │ [USED]  │ [USED]  │ [FREE]  │     │ [FREE]          │
└─────────────────────────────────────────────────────────────────┘
Free stack: [6,5,4,3,2,1,0]
free_count = 7

After freeing block 9 (first allocated):
┌─────────────────────────────────────────────────────────────────┐
│ Block 9 │ Block 8 │ Block 7 │ Block 6 │ ... │ Block 0         │
│ [FREE]  │ [USED]  │ [USED]  │ [FREE]  │     │ [FREE]          │
└─────────────────────────────────────────────────────────────────┘
Free stack: [9,6,5,4,3,2,1,0]  (LIFO - returns most recently freed)
free_count = 8

Note: No fragmentation! Every free block is exactly 64 bytes.
Any free block can satisfy any allocation!
```

### Thread-Safe Pool

```c
// Pool with interrupt disabling (single-threaded RTOS)
typedef struct {
    uint8_t memory[BLOCK_SIZE * BLOCK_COUNT];
    uint16_t free_stack[BLOCK_COUNT];
    uint16_t free_count;
    volatile uint8_t in_use;  // Critical section flag
} rtos_pool_t;

void* rtos_pool_alloc(rtos_pool_t* pool) {
    // Disable interrupts (critical section)
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    void* ptr = NULL;
    if (pool->free_count > 0) {
        uint16_t index = pool->free_stack[--pool->free_count];
        ptr = &pool->memory[index * BLOCK_SIZE];
    }
    
    // Restore interrupts
    if (!primask) __enable_irq();
    
    return ptr;
}
```

## Ring Buffer (Circular Buffer)

### Common Embedded Pattern

```c
// Lock-free ring buffer for ISR/thread communication
typedef struct {
    uint8_t* buffer;
    size_t size;
    volatile size_t head;  // Write index
    volatile size_t tail;  // Read index
} ring_buffer_t;

// Initialize
void ring_init(ring_buffer_t* rb, uint8_t* buffer, size_t size) {
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

// ISR-safe write (single writer, single reader)
bool ring_write(ring_buffer_t* rb, uint8_t data) {
    size_t next_head = (rb->head + 1) % rb->size;
    
    if (next_head == rb->tail) {
        return false;  // Buffer full
    }
    
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    return true;
}

// ISR-safe read
bool ring_read(ring_buffer_t* rb, uint8_t* data) {
    if (rb->head == rb->tail) {
        return false;  // Buffer empty
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return true;
}

// Usage: UART RX interrupt
ring_buffer_t uart_rx_ring;
static uint8_t uart_buffer[256];

void UART_IRQHandler(void) {
    uint8_t byte = UART->DR;
    ring_write(&uart_rx_ring, byte);  // No malloc!
}

// Task reads data
void process_uart(void) {
    uint8_t byte;
    while (ring_read(&uart_rx_ring, &byte)) {
        process_byte(byte);  // Deterministic!
    }
}
```

### Ring Buffer Visualization

```
RING BUFFER STATES
═══════════════════════════════════════════════════════════════════

EMPTY:
tail → 0
head → 0
┌────┬────┬────┬────┬────┬────┬────┬────┐
│    │    │    │    │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘

PARTIALLY FULL:
      head → 3
tail → 0
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ A  │ B  │ C  │    │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘

FULL (wraparound):
head → 2
      tail → 0
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ H  │ A  │ B  │ C  │ D  │ E  │ F  │ G  │
└────┴────┴────┴────┴────┴────┴────┴────┘
          tail → 5
```

## Stack Allocator (LIFO)

### Linear/Stack Allocator

```c
// Stack-based allocator - fastest and simplest
typedef struct {
    uint8_t* start;
    uint8_t* current;
    uint8_t* end;
    size_t peak_used;
} stack_alloc_t;

void stack_init(stack_alloc_t* sa, void* memory, size_t size) {
    sa->start = (uint8_t*)memory;
    sa->current = sa->start;
    sa->end = sa->start + size;
    sa->peak_used = 0;
}

void* stack_alloc(stack_alloc_t* sa, size_t size) {
    // Align size
    size = (size + 7) & ~7;
    
    if (sa->current + size > sa->end) {
        return NULL;  // Out of memory
    }
    
    void* ptr = sa->current;
    sa->current += size;
    
    size_t used = sa->current - sa->start;
    if (used > sa->peak_used) sa->peak_used = used;
    
    return ptr;
}

void stack_free(stack_alloc_t* sa, void* ptr) {
    // Only works if freeing in reverse order!
    if (ptr == sa->current - /* size unknown */) {
        // Can reset to that point
    }
}

// Reset entire stack (free everything)
void stack_reset(stack_alloc_t* sa) {
    sa->current = sa->start;
}

// Get marker for partial reset
void* stack_get_marker(stack_alloc_t* sa) {
    return sa->current;
}

void stack_reset_to_marker(stack_alloc_t* sa, void* marker) {
    sa->current = (uint8_t*)marker;
}

// Usage pattern
void process_frame(void) {
    stack_alloc_t temp_stack;
    uint8_t temp_mem[4096];
    stack_init(&temp_stack, temp_mem, sizeof(temp_mem));
    
    // Allocate temporary buffers
    uint8_t* headers = stack_alloc(&temp_stack, 512);
    uint8_t* payload = stack_alloc(&temp_stack, 1024);
    
    process_data(headers, payload);
    
    // All done - discard all!
    // No need to free individually
}  // temp_mem goes out of scope
```

## Deterministic Allocation Strategies

### Comparison Table

```
DETERMINISTIC ALLOCATION METHODS
═══════════════════════════════════════════════════════════════════

Strategy        | Time        | Fragmentation | Use Case
───────────────────────────────────────────────────────────────────
Static Array    | O(1) fixed  | None          | Known max size
Memory Pool     | O(1) fixed  | None          | Fixed-size objects
Stack Allocator | O(1) fixed  | None (LIFO)   | Temporary data
Ring Buffer     | O(1) fixed  | None          | Producer/consumer
Slab Allocator  | O(1) avg    | Low           | Kernel objects
Buddy System    | O(log N)    | Low internal  | Power-of-two sizes

Worst-case timing guarantees:
┌─────────────────────────────────────────────────────────────────┐
│  malloc()      : 200-5000 ns   (non-deterministic!) ❌         │
│  Pool alloc    : 50-100 ns     (always same)        ✓          │
│  Stack alloc   : 10-20 ns      (always same)        ✓          │
│  Static array  : 0 ns (compile time)                ✓✓         │
└─────────────────────────────────────────────────────────────────┘
```

### Real-Time Guarantee Example

```c
// Medical device: must allocate within 10 microseconds
// Using pool allocator guarantees this

typedef struct {
    uint32_t heart_rate;
    uint32_t spo2;
    uint32_t timestamp;
} patient_data_t;

// Pre-allocate pool for 100 measurements
#define MAX_MEASUREMENTS 100
static patient_data_t measurements[MAX_MEASUREMENTS];
static uint16_t free_indices[MAX_MEASUREMENTS];
static uint16_t free_count = MAX_MEASUREMENTS;

// ISR-safe allocation - always < 1 microsecond
patient_data_t* alloc_measurement(void) {
    if (free_count == 0) return NULL;
    
    // Disable interrupts for index access
    uint32_t irq = __get_PRIMASK();
    __disable_irq();
    
    uint16_t idx = free_indices[--free_count];
    
    if (!irq) __enable_irq();
    
    return &measurements[idx];
}

// Called from heartbeat interrupt (critical!)
void heartbeat_ISR(void) {
    patient_data_t* data = alloc_measurement();
    if (data) {
        data->heart_rate = read_heart_rate();
        data->spo2 = read_spo2();
        data->timestamp = get_systick();
    }
    // If allocation fails, we lose one reading
    // But system continues! No crash!
}
```

## Embedded Allocator Design Patterns

### Pattern 1: Memory Budget

```c
// Track memory usage at compile time
#define TOTAL_RAM_SIZE (128 * 1024)

// Reserve memory regions
#define RESERVED_STACK (8 * 1024)
#define RESERVED_HEAP  (16 * 1024)
#define RESERVED_POOLS (32 * 1024)

// Static pools
static uint8_t pool_32[32 * 100];
static uint8_t pool_64[64 * 50];
static uint8_t pool_128[128 * 25];

// Verification at compile time
_Static_assert(sizeof(pool_32) + sizeof(pool_64) + sizeof(pool_128) +
               RESERVED_STACK + RESERVED_HEAP <= TOTAL_RAM_SIZE,
               "Total memory exceeds available RAM!");
```

### Pattern 2: Arena Allocator

```c
// Arena for transaction-based allocation
typedef struct {
    uint8_t* start;
    uint8_t* current;
    size_t size;
} arena_t;

void arena_init(arena_t* arena, void* memory, size_t size) {
    arena->start = memory;
    arena->current = memory;
    arena->size = size;
}

void* arena_alloc(arena_t* arena, size_t size) {
    size = (size + 7) & ~7;
    if (arena->current + size > arena->start + arena->size) {
        return NULL;
    }
    void* ptr = arena->current;
    arena->current += size;
    return ptr;
}

void arena_reset(arena_t* arena) {
    arena->current = arena->start;
}

// Usage: Reset after each connection
typedef struct {
    arena_t conn_arena;
    uint8_t arena_mem[8192];
} connection_t;

void handle_connection(connection_t* conn) {
    arena_reset(&conn->conn_arena);
    
    http_request_t* req = arena_alloc(&conn->conn_arena, sizeof(*req));
    http_response_t* resp = arena_alloc(&conn->conn_arena, sizeof(*resp));
    
    process_http(req, resp);
    
    // All memory automatically freed!
}
```

### Pattern 3: Buddy Allocator for Embedded

```c
// Lightweight buddy allocator for power-of-two sizes
#define BUDDY_MAX_ORDER 8  // 2^8 = 256 bytes
#define BUDDY_MIN_ORDER 3  // 2^3 = 8 bytes

typedef struct {
    uint32_t free_lists[BUDDY_MAX_ORDER + 1];
    uint8_t memory[1 << BUDDY_MAX_ORDER];
} buddy_t;

// Bitmap-based buddy (no list traversal!)
void buddy_init(buddy_t* buddy) {
    for (int i = 0; i <= BUDDY_MAX_ORDER; i++) {
        buddy->free_lists[i] = 0;
    }
    buddy->free_lists[BUDDY_MAX_ORDER] = 1;  // One free block
}

int get_order(size_t size) {
    int order = BUDDY_MIN_ORDER;
    size_t block_size = 1 << BUDDY_MIN_ORDER;
    while (block_size < size && order < BUDDY_MAX_ORDER) {
        block_size <<= 1;
        order++;
    }
    return order;
}

void* buddy_alloc(buddy_t* buddy, size_t size) {
    int order = get_order(size);
    if (order > BUDDY_MAX_ORDER) return NULL;
    
    // Find available block
    int current = order;
    while (current <= BUDDY_MAX_ORDER) {
        if (buddy->free_lists[current]) {
            // Found block
            buddy->free_lists[current]--;
            
            // Split down to required order
            while (current > order) {
                current--;
                buddy->free_lists[current] += 2;  // Two blocks
            }
            
            // Return address (simplified)
            return buddy->memory;  // Would need address calculation
        }
        current++;
    }
    return NULL;
}
```

## RTOS Memory Management

### FreeRTOS Memory Options

```c
// FreeRTOS memory allocation schemes

// Option 1: Global array (simplest)
static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
void* pvPortMalloc(size_t xWantedSize) {
    // Simple pointer increment - no free!
    // Good for tasks that never delete
}

// Option 2: Best-fit with coalescing
void* pvPortMalloc(size_t xWantedSize) {
    // Similar to our custom allocator
    // Includes coalescing, splitting
    // Good for general use
}

// Option 3: Pool allocation for RTOS objects
// Task Control Blocks use pool
StaticTask_t xTaskBuffer;  // Static allocation
StackType_t xStack[configMINIMAL_STACK_SIZE];

TaskHandle_t xTask = xTaskCreateStatic(
    vTaskCode, "Task", configMINIMAL_STACK_SIZE,
    NULL, 1, xStack, &xTaskBuffer
);  // No malloc!

// Queue static allocation
static uint8_t queueStorageArea[10 * sizeof(int)];
static StaticQueue_t queueBuffer;
QueueHandle_t xQueue = xQueueCreateStatic(10, sizeof(int),
                                          queueStorageArea, &queueBuffer);
```

### ARM CMSIS-RTOS Example

```c
// CMSIS-RTOS v2 static allocation
#include "cmsis_os2.h"

// Pre-allocate thread control block and stack
static osThreadId_t app_thread_id;
static osThreadAttr_t app_thread_attr = {
    .name = "app_thread",
    .stack_mem = &app_thread_stack,
    .stack_size = 1024,
    .cb_mem = &app_thread_cb,
    .cb_size = sizeof(app_thread_cb),
};

// Create thread without dynamic allocation
app_thread_id = osThreadNew(app_thread_func, NULL, &app_thread_attr);

// Benefits:
// ✓ All memory known at compile time
// ✓ No runtime allocation failures
// ✓ MISRA compliant
```

## Real-World Examples

### Example 1: IoT Sensor Device

```c
// Battery-powered sensor node
#define NUM_SENSORS 4
#define BUFFER_SIZE 256

// All memory statically allocated
typedef struct {
    uint32_t timestamp;
    uint16_t sensor_data[NUM_SENSORS];
    uint8_t battery_level;
} sensor_reading_t;

// Pre-allocated buffers
static sensor_reading_t readings[100];  // Store 100 readings
static uint16_t read_index = 0;
static uint8_t tx_buffer[BUFFER_SIZE];
static uint8_t rx_buffer[BUFFER_SIZE];

// No malloc anywhere!
void sensor_task(void) {
    if (read_index < 100) {
        sensor_reading_t* reading = &readings[read_index++];
        reading->timestamp = get_time();
        for (int i = 0; i < NUM_SENSORS; i++) {
            reading->sensor_data[i] = read_sensor(i);
        }
    }
}

void tx_task(void) {
    // Pack readings into tx_buffer
    pack_data(tx_buffer, readings, read_index);
    send_radio(tx_buffer, calculate_size(read_index));
    read_index = 0;  // Reset after sending
}

// Result: Runs for months without fragmentation
```

### Example 2: Automotive ECU

```c
// CAN bus message handler (safety-critical)
#define MAX_CAN_IDS 32
#define MAX_PENDING 16

// Static message pools
typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
    uint32_t timestamp;
} can_message_t;

// Double-buffered pools (no dynamic allocation)
static can_message_t incoming_pool[MAX_PENDING];
static can_message_t outgoing_pool[MAX_PENDING];
static uint8_t in_head = 0, in_tail = 0;
static uint8_t out_head = 0, out_tail = 0;

// ISR context - must be deterministic!
void CAN_IRQHandler(void) {
    can_message_t msg;
    msg.id = CAN->sRIR;
    msg.data[0] = CAN->sRDLR;
    // ...
    
    // Circular buffer - O(1), deterministic
    uint8_t next = (in_head + 1) % MAX_PENDING;
    if (next != in_tail) {  // Not full
        incoming_pool[in_head] = msg;
        in_head = next;
    } else {
        // Overflow - drop message (better than crashing!)
        error_counter++;
    }
}

// Main loop processes messages
void process_can_messages(void) {
    while (in_tail != in_head) {
        can_message_t* msg = &incoming_pool[in_tail];
        process_message(msg);
        in_tail = (in_tail + 1) % MAX_PENDING;
    }
}
```

## Best Practices Summary

```
EMBEDDED MEMORY BEST PRACTICES
═══════════════════════════════════════════════════════════════════

✓ Use Static Allocation Whenever Possible
  • Know your maximums at compile time
  • Use arrays instead of pointers
  • Leverage linker scripts for placement

✓ Use Memory Pools for Variable Needs
  • Fixed-size blocks eliminate fragmentation
  • O(1) deterministic timing
  • Great for message queues, object pools

✓ Use Ring Buffers for Data Streams
  • Perfect for ISR/main communication
  • Lock-free when single reader/writer
  • No allocation overhead

✓ Stack Allocation for Temporary Data
  • Very fast (just move pointer)
  • Perfect for per-transaction data
  • Reset entire scope at once

✓ Avoid malloc/free in Critical Paths
  • Don't allocate in ISRs
  • Don't allocate in real-time tasks
  • Pre-allocate everything at startup

✓ Validate Memory Usage
  • Use _Static_assert for compile-time checks
  • Monitor high-water marks
  • Add memory poisoning in debug builds

❌ Common Mistakes:
  • Using recursion (stack overflow risk)
  • Ignoring alignment for DMA/peripherals
  • Fragmentation from mixed allocation sizes
  • Not handling allocation failures
  • Using variable-length arrays (VLA) on stack
```

## Interview Questions & Answers

### Q: Why avoid malloc in embedded systems?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

1. Non-deterministic timing
   • Search time depends on fragmentation
   • Can't guarantee real-time deadlines

2. Fragmentation
   • Small free blocks accumulate over time
   • System crashes even with free memory

3. Failure handling complexity
   • No virtual memory to fall back on
   • Handling NULL returns everywhere is tedious

4. Memory leaks
   • No OS cleanup on exit
   • Device runs for years

5. Stack overflow risk
   • Small stacks (2-8KB)
   • Recursion dangerous
   
Example: Medical infusion pump
- Must deliver precise dose every 10ms
- malloc could take 5ms or 5000ms
- Missed deadline → patient risk!
```

### Q: What is deterministic allocation?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Deterministic allocation = Execution time is predictable and bounded

Examples:
┌─────────────────────────────────────────────────────────────────┐
│  malloc(64):           200-5000 ns   ❌ NOT deterministic      │
│  pool_alloc():         50-60 ns       ✓ Deterministic (O(1))   │
│  stack_alloc():        10-15 ns       ✓ Deterministic (O(1))   │
│  static array:         0 ns (compile) ✓ Deterministic          │
└─────────────────────────────────────────────────────────────────┘

Key properties:
✓ Upper bound known at compile time
✓ No worst-case spikes
✓ Suitable for hard real-time systems

Implementation:
- Use fixed-time algorithms (no loops over free list)
- Avoid searching (use arrays, bitmaps)
- Pre-compute everything possible
```

### Q: How do you handle out-of-memory in embedded?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Strategy 1: Graceful Degradation
• Drop non-critical features
• Log error but continue

Strategy 2: Pool Reservation  
• Reserve emergency pool
• Critical allocations always succeed

Strategy 3: Pre-allocation
• Allocate everything at startup
• No runtime allocations

Strategy 4: Watermark Monitoring
• Track peak usage
• Reset/reboot if approaching limit

Example:
static uint8_t emergency_pool[512];
static void* emergency_alloc(size_t size) {
    return (size <= 512) ? emergency_pool : NULL;
}
```

This comprehensive guide covers everything about embedded memory management with practical examples perfect for embedded systems interviews!
