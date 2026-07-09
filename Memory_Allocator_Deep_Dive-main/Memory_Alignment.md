# Memory Alignment

## Overview

Memory alignment is a fundamental concept in systems programming that affects performance, correctness, and memory usage. Understanding alignment is crucial for embedded systems, performance-critical code, and writing portable software.

## What is Memory Alignment?

### Basic Concept

```
MEMORY ALIGNMENT VISUALIZATION
═══════════════════════════════════════════════════════════════════

Memory is organized into addressable bytes, but CPUs access memory in word-sized chunks.

32-bit system (4-byte words):
┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │ 6  │ 7  │ 8  │ 9  │
└────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
└──────────┘ └──────────┘ └──────────┘ └──────────┘
  Word 0       Word 1       Word 2       Word 3

ALIGNED ACCESS (address multiple of 4):
Address 4: ┌────┬────┬────┬────┐
           │    │    │    │    │ ← Single CPU operation
           └────┴────┴────┴────┘

MISALIGNED ACCESS (address not multiple of 4):
Address 3:    ┌────┬────┬────┬────┐
              │    │    │    │    │ ← Spans 2 words!
              └────┴────┴────┴────┘
         ┌────┘                    └────┐
         Word 0                        Word 1
         
Requires 2 CPU operations + shift + merge!
```

### Alignment Rules by Architecture

```
CPU ARCHITECTURE ALIGNMENT REQUIREMENTS
═══════════════════════════════════════════════════════════════════

Data Type    x86 (allowed)    x86-64    ARM (required)    ARM64
───────────────────────────────────────────────────────────────────
char         1-byte           1-byte    1-byte            1-byte
short        2-byte           2-byte    2-byte            2-byte  
int          4-byte           4-byte    4-byte (required) 4-byte
long         4-byte           8-byte    4-byte            8-byte
long long    8-byte           8-byte    8-byte            8-byte
void*        4-byte           8-byte    4-byte            8-byte
float        4-byte           4-byte    4-byte            4-byte
double       8-byte           8-byte    8-byte (required) 8-byte

x86: Natural alignment not required (but slower)
ARM: Natural alignment REQUIRED (hardware fault!)
```

## Structure Padding Deep Dive

### Why Padding Occurs

```
STRUCTURE PADDING VISUALIZATION
═══════════════════════════════════════════════════════════════════

Consider struct A:
struct A {
    char c;   // 1 byte
    int x;    // 4 bytes
};

Without padding (packed):
Memory layout (assuming 4-byte alignment requirement for int):
Address:  0    1    2    3    4    5    6    7
         ┌────┬────┬────┬────┬────┬────┬────┬────┐
         │ c  │ x[0]│ x[1]│ x[2]│ x[3]│    │    │    │
         └────┴────┴────┴────┴────┴────┴────┴────┘
              ↑ int starts at offset 1 (MISALIGNED!)
              Causes performance penalty or fault on ARM

With padding (natural alignment):
         ┌────┬────┬────┬────┬────┬────┬────┬────┐
         │ c  │PAD │PAD │PAD │ x[0]│ x[1]│ x[2]│ x[3]│
         └────┴────┴────┴────┴────┴────┴────┴────┘
         ↑         ↑ int starts at offset 4 (ALIGNED!)
         1 byte   3 bytes padding
         actual   
```

### Complete Padding Example

```c
#include <stdio.h>
#include <stddef.h>

// Example 1: Basic padding
struct Example1 {
    char c;     // offset 0, size 1
    int i;      // offset 4 (padding 3), size 4
    char d;     // offset 8, size 1
};  // Total size: 12 (with trailing padding for array alignment)

// Memory layout:
// Offset:  0    1    2    3    4    5    6    7    8    9   10   11
//        ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
//        │ c  │PAD │PAD │PAD │ i[0]│ i[1]│ i[2]│ i[3]│ d  │PAD │PAD │PAD │
//        └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
//                               ↑                         ↑
//                          aligned to 4               trailing pad
//                                                    (for array)

// Example 2: Reordering for efficiency
struct Example2 {
    int i;      // offset 0, size 4
    char c;     // offset 4, size 1
    char d;     // offset 5, size 1
};  // Total size: 8 (less padding!)

// Memory layout:
// Offset:  0    1    2    3    4    5    6    7
//        ┌────┬────┬────┬────┬────┬────┬────┬────┐
//        │ i[0]│ i[1]│ i[2]│ i[3]│ c  │ d  │PAD │PAD │
//        └────┴────┴────┴────┴────┴────┴────┴────┘

int main() {
    printf("sizeof(Example1) = %zu\n", sizeof(struct Example1)); // 12
    printf("sizeof(Example2) = %zu\n", sizeof(struct Example2)); // 8
    
    printf("offsets in Example1:\n");
    printf("  c: %zu\n", offsetof(struct Example1, c)); // 0
    printf("  i: %zu\n", offsetof(struct Example1, i)); // 4
    printf("  d: %zu\n", offsetof(struct Example1, d)); // 8
    
    printf("offsets in Example2:\n");
    printf("  i: %zu\n", offsetof(struct Example2, i)); // 0
    printf("  c: %zu\n", offsetof(struct Example2, c)); // 4
    printf("  d: %zu\n", offsetof(struct Example2, d)); // 5
    
    return 0;
}
```

### Complex Structure Padding Rules

```c
// Comprehensive padding demonstration
struct Complex {
    char a;      // offset 0, size 1
    double b;    // offset 8 (padding 7), needs 8-byte alignment
    char c;      // offset 16, size 1
    int d;       // offset 20 (padding 3), needs 4-byte alignment
    char e;      // offset 24, size 1
};  // Total: 32 bytes (trailing padding to multiple of largest alignment 8)

// Detailed layout visualization:
// Offset:0   1   2   3   4   5   6   7
//       ┌───┬───┬───┬───┬───┬───┬───┬───┐
//       │ a │PAD│PAD│PAD│PAD│PAD│PAD│PAD│
//       └───┴───┴───┴───┴───┴───┴───┴───┘
// Offset:8   9   10  11  12  13  14  15
//       ┌───┬───┬───┬───┬───┬───┬───┬───┐
//       │ b[0] b[1] b[2] b[3] b[4] b[5] b[6] b[7]│
//       └───┴───┴───┴───┴───┴───┴───┴───┘
// Offset:16  17  18  19  20  21  22  23
//       ┌───┬───┬───┬───┬───┬───┬───┬───┐
//       │ c │PAD│PAD│PAD│ d[0] d[1] d[2] d[3]│
//       └───┴───┴───┴───┴───┴───┴───┴───┘
// Offset:24  25  26  27  28  29  30  31
//       ┌───┬───┬───┬───┬───┬───┬───┬───┐
//       │ e │PAD│PAD│PAD│PAD│PAD│PAD│PAD│ ← trailing padding
//       └───┴───┴───┴───┴───┴───┴───┴───┘
```

## Alignment Rules Summary

### Fundamental Alignment Rules

```
ALIGNMENT RULES (The Golden Rules)
═══════════════════════════════════════════════════════════════════

Rule 1: Natural Alignment
───────────────────────────────────────────────────────────────────
A type's alignment requirement = its size (up to word size)

Examples:
- char: 1-byte aligned (any address)
- short: 2-byte aligned (even addresses)
- int: 4-byte aligned (address multiple of 4)
- double: 8-byte aligned (address multiple of 8)

Rule 2: Structure Alignment
───────────────────────────────────────────────────────────────────
- Each member aligned according to its own requirement
- Structure's alignment = largest member's alignment
- Structure size padded to multiple of its alignment

Rule 3: Array Alignment
───────────────────────────────────────────────────────────────────
- Array elements are contiguous
- Element alignment = element size (up to word size)

Rule 4: Union Alignment
───────────────────────────────────────────────────────────────────
- Size = largest member
- Alignment = largest member's alignment
```

### Alignment Calculation Examples

```c
// Helper macros
#define ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

#define ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

// Example usage
size_t calculate_padded_size(size_t current_offset, size_t field_size) {
    size_t alignment = field_size;  // Natural alignment
    return ALIGN_UP(current_offset, alignment);
}

// Real example
struct Demo {
    char a;     // offset=0, needs 1-byte alignment
    int b;      // offset=4 (ALIGN_UP(1, 4) = 4)
    short c;    // offset=8 (ALIGN_UP(5, 2) = 8)
    double d;   // offset=16 (ALIGN_UP(10, 8) = 16)
    char e;     // offset=24, needs 1-byte alignment
};  // Final size = ALIGN_UP(25, 8) = 32
```

## Visual Alignment Examples

### 32-bit vs 64-bit System Differences

```
COMPARING 32-BIT AND 64-BIT LAYOUTS
═══════════════════════════════════════════════════════════════════

struct Mixed {
    char c;     // 1 byte
    void* p;    // 4 bytes (32-bit) or 8 bytes (64-bit)
    int i;      // 4 bytes
};

32-BIT SYSTEM (4-byte pointer):
───────────────────────────────────────────────────────────────────
Offset: 0    1    2    3    4    5    6    7    8    9   10   11
       ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
       │ c  │PAD │PAD │PAD │ p[0]│ p[1]│ p[2]│ p[3]│ i[0]│ i[1]│ i[2]│ i[3]│
       └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
       0         4               8               12
Size = 12 bytes (multiple of 4)

64-BIT SYSTEM (8-byte pointer):
───────────────────────────────────────────────────────────────────
Offset: 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
       ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
       │ c  │PAD │PAD │PAD │PAD │PAD │PAD │PAD │ p[0] p[1] p[2] p[3] p[4] p[5] p[6] p[7]│
       └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
       0                        8                                                      16
       Then i at offset 16 (already aligned)
       
Size = 24 bytes (multiple of 8)
```

### Nested Structures Alignment

```c
struct Inner {
    char a;     // offset 0
    int b;      // offset 4
};  // size = 8, alignment = 4

struct Outer {
    char c;         // offset 0
    struct Inner d; // offset 4 (ALIGN_UP(1, 4) = 4)
    short e;        // offset 12 (ALIGN_UP(12, 2) = 12)
};  // size = 16 (multiple of 4)

// Visualization:
// offset:0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
//       ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
//       │ c │PAD│PAD│PAD│ a │PAD│PAD│PAD│ b[0] b[1] b[2] b[3]│ e0│ e1│PAD│PAD│
//       └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
//                         └─────── Inner d ───────┘
```

## Alignment Functions and Operators

### alignof and alignas (C11/C++11)

```c
#include <stdalign.h>

// alignof - get alignment requirement
printf("Alignment of char: %zu\n", alignof(char));     // 1
printf("Alignment of int: %zu\n", alignof(int));       // 4
printf("Alignment of double: %zu\n", alignof(double)); // 8

// alignas - specify custom alignment
struct alignas(64) CacheAligned {
    int data[16];  // Will be aligned to 64-byte cache line
};  // Useful for avoiding false sharing in multi-threading

// Dynamic alignment query
_Alignof(max_align_t);  // Maximum alignment required by any type (typically 16)
```

### Manual Alignment Functions

```c
// Generic alignment macros
#define ALIGN_UP(x, alignment) \
    (((x) + (alignment) - 1) & ~((alignment) - 1))

#define ALIGN_DOWN(x, alignment) \
    ((x) & ~((alignment) - 1))

#define IS_ALIGNED(x, alignment) \
    (((x) & ((alignment) - 1)) == 0)

// Get alignment of a pointer
#define PTR_ALIGN(ptr, alignment) \
    ((void*)ALIGN_UP((uintptr_t)(ptr), alignment))

// Example: Implement aligned malloc
void* aligned_malloc(size_t size, size_t alignment) {
    // Allocate extra space for alignment + pointer storage
    size_t total_size = size + alignment + sizeof(void*);
    void* raw_ptr = malloc(total_size);
    if (!raw_ptr) return NULL;
    
    // Calculate aligned address
    void* aligned_ptr = (void*)ALIGN_UP((uintptr_t)raw_ptr + sizeof(void*), 
                                         alignment);
    
    // Store original pointer before aligned address
    ((void**)aligned_ptr)[-1] = raw_ptr;
    
    return aligned_ptr;
}

void aligned_free(void* aligned_ptr) {
    if (aligned_ptr) {
        void* raw_ptr = ((void**)aligned_ptr)[-1];
        free(raw_ptr);
    }
}
```

## GCC Packed Attribute

### Using __attribute__((packed))

```c
// Packed structure (no padding)
struct __attribute__((packed)) PackedStruct {
    char c;     // offset 0
    int i;      // offset 1 (NOT aligned!)
    short s;    // offset 5
};  // size = 7 bytes (no padding)

// Memory layout (packed):
// offset:0   1   2   3   4   5   6
//       ┌───┬───┬───┬───┬───┬───┬───┐
//       │ c │ i[0]│ i[1]│ i[2]│ i[3]│ s[0]│ s[1]│
//       └───┴───┴───┴───┴───┴───┴───┘
//           ↑ int is misaligned! (address 1)

// Network packet example (typical packed usage)
struct __attribute__((packed)) EthernetHeader {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
};  // No padding - matches wire format exactly

// Combining packed and aligned
struct __attribute__((packed, aligned(4))) PackedAligned {
    uint8_t flags;
    uint32_t value;
};  // Packed internally but aligned to 4-byte boundary
```

### Risks of Packed Structures

```c
struct __attribute__((packed)) Unaligned {
    char c;
    int i;
};

void dangerous_access() {
    struct Unaligned u;
    u.c = 'A';
    u.i = 42;  // May cause issues on some architectures!
    
    int* ptr = &u.i;  
    // On ARM, this pointer is misaligned!
    // Accessing *ptr could crash or be extremely slow
}

// Safe way to access packed structure fields
void safe_access(struct Unaligned* u) {
    int value;
    // Copy byte-by-byte to avoid misaligned access
    memcpy(&value, &u->i, sizeof(int));
    // Use value
}

// Or use unaligned access macros
#include <endian.h>
int value = le32toh(*(uint32_t*)&u->i);  // Still risky!

// Better: explicit byte extraction
int value = (uint32_t)u->i_bytes[0] |
            (uint32_t)u->i_bytes[1] << 8 |
            (uint32_t)u->i_bytes[2] << 16 |
            (uint32_t)u->i_bytes[3] << 24;
```

## Performance Impact

### Aligned vs Misaligned Access

```
PERFORMANCE COMPARISON
═══════════════════════════════════════════════════════════════════

Testing on x86_64 (relative cycles):

Operation                  Aligned    Misaligned    Penalty
───────────────────────────────────────────────────────────────────
char read                   1x          1x          0%
int read                    1x          2x         100%
int write                   1x          2x         100%
double read                 1x          3x         200%
double write                1x          3x         200%
SIMD (16-byte)              1x          5x         400%

Real-world impact:
┌─────────────────────────────────────────────────────────────────┐
│  Misaligned 4-byte access:                                     │
│  ┌────────────┐    ┌────────────┐                             │
│  │ Word 0     │    │ Word 1     │                             │
│  └────────────┘    └────────────┘                             │
│       ↓                  ↓                                     │
│    Read word 0      Read word 1                               │
│       ↓                  ↓                                     │
│    Shift right      Shift left                                │
│       ↓                  ↓                                     │
│         └────── OR ──────┘                                    │
│                ↓                                               │
│            Result (5-10 cycles penalty)                       │
└─────────────────────────────────────────────────────────────────┘

ARM Cortex-A (aligned REQUIRED):
┌─────────────────────────────────────────────────────────────────┐
│  Misaligned int access:                                        │
│  → Data Abort exception (handler invoked)                     │
│  → Kernel fixes in software (thousands of cycles!)            │
│  → 1000x+ performance penalty!                                │
└─────────────────────────────────────────────────────────────────┘
```

### Cache Line Alignment

```c
// False sharing prevention
#define CACHE_LINE_SIZE 64

struct alignas(CACHE_LINE_SIZE) PerThreadData {
    int counter;
    char padding[CACHE_LINE_SIZE - sizeof(int)];
};  // Each structure on its own cache line

// Avoid false sharing visualization:
// Without padding:
// CPU0: [counter0][counter1] ← same cache line
// CPU1:            ↑ writes here, invalidates CPU0's cache
// 
// With padding:
// CPU0: [counter0][padding...] ← cache line 0
// CPU1: [counter1][padding...] ← cache line 1
// No cache invalidation!

// Critical for multi-threading
struct alignas(64) SharedData {
    atomic<int> counter;     // Will be on its own cache line
    char padding[56];         // Pad to 64 bytes
};
```

## Embedded Systems Considerations

### Memory-Constrained Alignment

```c
// Balancing size vs performance on embedded systems

// Option 1: Natural alignment (fast, more memory)
struct FastSensorData {
    uint32_t timestamp;  // 4 bytes
    uint16_t values[8];  // 16 bytes
    uint8_t status;      // 1 byte
};  // Size = 24 bytes (with padding)

// Option 2: Packed (slower, less memory)
struct __attribute__((packed)) PackedSensorData {
    uint32_t timestamp;
    uint16_t values[8];
    uint8_t status;
};  // Size = 21 bytes (13% smaller!)

// Option 3: Manual packing with reordering
struct OptimizedSensorData {
    uint32_t timestamp;
    uint16_t values[8];
    uint8_t status;
    uint8_t padding;  // Explicit for alignment
};  // Size = 24, but explicit control

// Access packed data safely on ARM
static inline uint32_t read_packed_32(const void* ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));  // Compiler handles alignment
    return result;
}
```

### Alignment in Interrupt Handlers

```c
// Critical for embedded RTOS
struct __attribute__((aligned(8))) InterruptContext {
    uint32_t registers[16];  // Ensure 64-bit alignment for CMSIS
    uint32_t xpsr;
    void* return_address;
};  // Must be properly aligned for hardware

// Double-check alignment at compile time
_Static_assert(alignof(struct InterruptContext) >= 8,
               "Interrupt context must be 8-byte aligned");
```

## Common Alignment Pitfalls

### Pitfall 1: Casting Pointers

```c
// BAD: Creates misaligned pointer
char buffer[100];
int* ptr = (int*)&buffer[1];  // Likely misaligned!
*ptr = 42;  // Potential crash on ARM

// GOOD: Ensure alignment
int* ptr = (int*)ALIGN_UP((uintptr_t)buffer, alignof(int));
```

### Pitfall 2: Network Protocols

```c
// BAD: Casting network buffer directly
void process_packet(char* packet) {
    struct EthernetHeader* eth = (struct EthernetHeader*)packet;
    // If packet isn't aligned, this crashes on some CPUs!
}

// GOOD: Use memcpy or packed attribute
void process_packet(char* packet) {
    struct EthernetHeader eth;
    memcpy(&eth, packet, sizeof(eth));  // Safe on all platforms
}

// Or define header as packed
struct __attribute__((packed)) EthernetHeader {
    // ...
};  // Then cast may be safe, but still slower
```

### Pitfall 3: DMA Buffers

```c
// DMA controllers often require specific alignment
#define DMA_ALIGNMENT 32  // Common for cache line

// BAD: Heap may not provide required alignment
void* dma_buffer = malloc(4096);  // Alignment unspecified

// GOOD: Use aligned allocation
void* dma_buffer;
posix_memalign(&dma_buffer, DMA_ALIGNMENT, 4096);

// Or use aligned attribute (C11)
alignas(32) uint8_t dma_buffer[4096];

// Static alignment with GCC
__attribute__((aligned(32))) uint8_t dma_buffer[4096];
```

## Best Practices Summary

```
ALIGNMENT BEST PRACTICES
═══════════════════════════════════════════════════════════════════

✅ DO's:
┌─────────────────────────────────────────────────────────────────┐
│  ✓ Order struct members from largest to smallest              │
│  ✓ Use alignas() for explicit alignment needs                 │
│  ✓ Use memcpy() for unaligned access                          │
│  ✓ Check alignment with alignof()                             │
│  ✓ Use static assertions to verify critical alignments        │
│  ✓ Pad structures to cache line boundaries in multi-threading │
└─────────────────────────────────────────────────────────────────┘

❌ DON'Ts:
┌─────────────────────────────────────────────────────────────────┐
│  ✗ Don't assume natural alignment                               │
│  ✗ Don't cast arbitrary pointers without checking alignment    │
│  ✗ Don't use packed structures for frequently accessed data    │
│  ✗ Don't ignore alignment warnings                              │
│  ✗ Don't rely on packed for pointer storage                    │
│  ✗ Don't mix packed and non-packed in the same structure       │
└─────────────────────────────────────────────────────────────────┘

Embedded specific:
┌─────────────────────────────────────────────────────────────────┐
│  ✓ Use packed only for wire protocols and MMIO                 │
│  ✓ Ensure DMA buffers are cache-line aligned                   │
│  ✓ Verify alignment for interrupt context structures           │
│  ✓ Use __attribute__((aligned)) for critical data              │
└─────────────────────────────────────────────────────────────────┘
```

This comprehensive guide covers everything about memory alignment with visual diagrams, practical examples, and platform-specific considerations essential for systems programming interviews!
