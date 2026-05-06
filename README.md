# Memory Management in C
## Static, Stack & Dynamic (Heap) Memory

[![C Language](https://img.shields.io/badge/C-Programming-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Memory Management](https://img.shields.io/badge/Memory-Management-orange.svg)](https://en.wikipedia.org/wiki/Memory_management)
[![Static Memory](https://img.shields.io/badge/Static-Memory-yellow.svg)](https://en.wikipedia.org/wiki/Static_memory_allocation)
[![Stack Memory](https://img.shields.io/badge/Stack-Memory-green.svg)](https://en.wikipedia.org/wiki/Stack-based_memory_allocation)
[![Heap Memory](https://img.shields.io/badge/Heap-Dynamic-red.svg)](https://en.wikipedia.org/wiki/Heap_(programming))
[![Educational](https://img.shields.io/badge/Educational-Project-purple.svg)](https://en.wikipedia.org/wiki/Educational_software)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20WSL-lightgrey.svg)](https://en.wikipedia.org/wiki/Cross-platform)
[![GCC Compiler](https://img.shields.io/badge/Compiler-GCC%209+-blueviolet.svg)](https://gcc.gnu.org/)

This project demonstrates how memory works in real C applications by showcasing static memory, stack memory, and dynamic (heap) memory. It includes example programs, documentation, and test scripts to help beginners understand how memory allocation works at the system level.

---

##  3 Memory Types at a Glance

| Type | Keyword | Lifetime | Speed | Limit | When to Use |
|------|---------|----------|-------|-------|--------------|
| **Static** | `static` | Whole program |  Fast | Compile-time | Counters, constants |
| **Stack** | (auto) | Function scope |  Fastest | 1-8MB | Local variables, small arrays |
| **Heap** | `malloc()` | Your control |  Slower | RAM | Large data, unknown size |

---

##  Key Rules (Remember These!)

```
┌─────────────────────────────────────────────────────────┐
│  STATIC → Lives forever. Never freed.                   │
│  STACK  → Auto freed. Small & fast. Don't return it.    │
│  HEAP   → You allocate. You free. Or it LEAKS.          │
└─────────────────────────────────────────────────────────┘
```

---

##  Common Mistakes & Fixes

|  Mistake |  Fix |
|------------|--------|
| `malloc(10);` (no free) | `free(ptr);` |
| `free(p); *p = 1;` | `free(p); p = NULL;` |
| `free(p); free(p);` | `free(p); p = NULL;` |
| `int big[10000000];` | `int *big = malloc(10000000);` |
| `return &local;` | Return heap or global |

---

##  Project Structure

```
memory-management-project/
│
├── docs/
│   ├── README.md
│   ├── static_memory.md
│   ├── dynamic_memory.md
│   ├── memory_diagrams.png
│
├── src/
│   ├── static_example.c
│   ├── dynamic_example.c
│   ├── mixed_usage.c
│
├── tests/
│   ├── run_tests.sh
│
└── LICENSE
```

---

##  Source Code Examples

### 1. static_example.c (Static Memory)
```c
#include <stdio.h>

static int counter = 0;   // Lives forever

void increase() {
    counter++;
    printf("Counter = %d\n", counter);
}

int main() {
    increase();  // Counter = 1
    increase();  // Counter = 2
    increase();  // Counter = 3
    return 0;
}
```
> **What you learn:** Static variables keep their value between calls.

### 2. dynamic_example.c (Heap Memory)
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = malloc(n * sizeof(int));  // On heap
    
    if (arr == NULL) return 1;  // Always check!
    
    for (int i = 0; i < n; i++) {
        arr[i] = i + 1;
        printf("%d ", arr[i]);
    }
    
    free(arr);  // YOU must free it
    return 0;
}
```
> **What you learn:** Heap gives control. YOU must free it.

### 3. mixed_usage.c
Shows static + stack + heap in one program.

---

## 🔧 How to Run

```bash
# Static example
gcc src/static_example.c -o static
./static

# Dynamic example
gcc src/dynamic_example.c -o dynamic
./dynamic

# Mixed example
gcc src/mixed_usage.c -o mixed
./mixed

# Run tests
cd tests && ./run_tests.sh
```

### Debug Memory Leaks
```bash
# Install valgrind first
valgrind --leak-check=full ./dynamic

# Or use Address Sanitizer (built into GCC)
gcc -fsanitize=address -g src/dynamic_example.c -o dynamic
./dynamic
```

---

## 🧠 Quick Tips & Tricks

```c
// ✅ Always check malloc
int *p = malloc(size);
if (p == NULL) { /* handle error */ }

// ✅ Use calloc for zero-initialized memory
int *p = calloc(n, sizeof(int));  // All zeros

// ✅ Set freed pointers to NULL
free(p);
p = NULL;  // Prevents double-free bugs

// ✅ Use sizeof with variable (safer)
p = malloc(count * sizeof(*p));   // Better than sizeof(int)

// ✅ Stack is FAST but TINY (1-8MB default)
// Use heap for anything over ~1KB
```

---

##  Memory Debugging Tools

| Tool | Command | What it Finds |
|------|---------|----------------|
| Valgrind | `valgrind ./prog` | Memory leaks, uninit memory |
| ASAN | `gcc -fsanitize=address` | Buffer overflow, use-after-free |
| GDB | `gdb ./prog` | Inspect memory at runtime |

---

##  Cheat Sheet

```c
// ========== STATIC ==========
static int x;           // File scope, lives forever
void f() { static int y; }  // Value persists across calls

// ========== STACK ==========
int local = 42;         // Auto freed
int arr[100];           // Fixed size
// NEVER return &local; // Dangling pointer!

// ========== HEAP ==========
int *p = malloc(size);       // Allocate
int *z = calloc(n, size);    // Allocate + zero
p = realloc(p, new_size);    // Resize
free(p); p = NULL;           // Free + clear

// ========== ALWAYS REMEMBER ==========
// 1. Check malloc return value
// 2. Free what you allocate
// 3. Set freed pointers to NULL
// 4. Don't return stack addresses
// 5. Use valgrind to check
```

---

##  Memory Checklist

Before submitting code:

- [ ] Every `malloc/calloc/realloc` has a `free`
- [ ] Freed pointers set to `NULL`
- [ ] No large arrays on stack (>1KB)
- [ ] `malloc` return value checked
- [ ] No returning pointers to local variables
- [ ] Valgrind = "All heap blocks freed"

---
=========================================================================================================================================
Here's your **complete, deep knowledge README** with diagrams, flowcharts, and **ALL memory management topics** covered in a simple, visual way.

```markdown
# Memory Management in C
## Static, Stack & Dynamic (Heap) Memory - Complete Deep Dive

[![C Language](https://img.shields.io/badge/C-Programming-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Memory Management](https://img.shields.io/badge/Memory-Management-orange.svg)](https://en.wikipedia.org/wiki/Memory_management)
[![Static Memory](https://img.shields.io/badge/Static-Memory-yellow.svg)](https://en.wikipedia.org/wiki/Static_memory_allocation)
[![Stack Memory](https://img.shields.io/badge/Stack-Memory-green.svg)](https://en.wikipedia.org/wiki/Stack-based_memory_allocation)
[![Heap Memory](https://img.shields.io/badge/Heap-Dynamic-red.svg)](https://en.wikipedia.org/wiki/Heap_(programming))
[![Educational](https://img.shields.io/badge/Educational-Project-purple.svg)](https://en.wikipedia.org/wiki/Educational_software)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

This project demonstrates how memory works in real C applications. All topics covered with diagrams, examples, and test scripts.

---

## 🗺️ Complete Memory Map (Visual Diagram)

```
                    HIGH MEMORY ADDRESSES
    ┌─────────────────────────────────────────────┐
    │                 STACK (↓ grows down)         │
    │  ┌─────────────────────────────────────┐    │
    │  │ local variables, function calls      │    │
    │  │ return addresses, parameters         │    │
    │  └─────────────────────────────────────┘    │
    │                    ↓                         │
    │                    ↓                         │
    │                    ↓                         │
    ├─────────────────────────────────────────────┤
    │                  (free space)                │
    ├─────────────────────────────────────────────┤
    │                    ↑                         │
    │                    ↑                         │
    │                    ↑                         │
    │  ┌─────────────────────────────────────┐    │
    │  │ malloc, calloc, realloc             │    │
    │  │ dynamic data                        │    │
    │  └─────────────────────────────────────┘    │
    │                 HEAP (↑ grows up)            │
    ├─────────────────────────────────────────────┤
    │              DATA SEGMENT                    │
    │  ┌─────────────────────────────────────┐    │
    │  │ initialized static/global variables  │    │
    │  │ e.g., static int x = 10;             │    │
    │  └─────────────────────────────────────┘    │
    │  ┌─────────────────────────────────────┐    │
    │  │ BSS - uninitialized static/global    │    │
    │  │ e.g., static int y;                  │    │
    │  └─────────────────────────────────────┘    │
    ├─────────────────────────────────────────────┤
    │              TEXT SEGMENT                    │
    │         (code / instructions)                │
    └─────────────────────────────────────────────┘
                    LOW MEMORY ADDRESSES
```

---

## 🔄 Memory Allocation Decision Flowchart

```
                    ┌─────────────────┐
                    │  NEED MEMORY?   │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │ Data size known  │
                    │ at compile time? │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │ YES                         │ NO
              ▼                             ▼
    ┌─────────────────┐           ┌─────────────────┐
    │ Need it for     │           │ MUST use HEAP   │
    │ entire program? │           │ malloc/calloc   │
    └────────┬────────┘           └─────────────────┘
             │
      ┌──────┼──────┐
      │ YES         │ NO
      ▼             ▼
┌──────────┐  ┌──────────────┐
│  STATIC  │  │    STACK     │
│ memory   │  │  automatic   │
└──────────┘  └──────────────┘
```

---

## 🧠 Memory Allocation Decision Tree (Simple)

```
                         START
                           │
                    Need memory?
                           │
              ┌────────────┴────────────┐
              │                         │
         Small data?                Large data?
        (< 1KB typical)             (> 1KB)
              │                         │
              ▼                         ▼
         Use STACK                   Use HEAP
         (automatic)                 (malloc)
              │                         │
              ▼                         ▼
         Auto freed                  Must free()
         when function               manually!
         returns
```

---

## 📋 Complete Topics Covered

| # | Topic | Section |
|---|-------|---------|
| 1 | Static Memory | ↓ |
| 2 | Stack Memory | ↓ |
| 3 | Heap Memory (malloc, calloc, realloc, free) | ↓ |
| 4 | Memory Leaks | ↓ |
| 5 | Dangling Pointers | ↓ |
| 6 | Double Free | ↓ |
| 7 | Stack Overflow | ↓ |
| 8 | Buffer Overflow | ↓ |
| 9 | Memory Fragmentation | ↓ |
| 10 | Alignment | ↓ |
| 11 | Memory Pool / Arena | ↓ |
| 12 | Valgrind & Sanitizers | ↓ |

---

## 1️⃣ STATIC MEMORY (Deep Dive)

### Memory Layout Diagram
```
┌────────────────────────────────────────┐
│            DATA SEGMENT                 │
│  ┌──────────────────────────────────┐  │
│  │ Initialized static variables     │  │
│  │ int x = 10;  (4 bytes)           │  │
│  │ char c = 'A'; (1 byte)           │  │
│  └──────────────────────────────────┘  │
│  ┌──────────────────────────────────┐  │
│  │ BSS (Block Started by Symbol)    │  │
│  │ static int y;  (zero-initialized)│  │
│  │ static char buf[1000]; (zeros)   │  │
│  └──────────────────────────────────┘  │
└────────────────────────────────────────┘
```

### Code Examples
```c
// FILE-SCOPE STATIC (private to this file)
static int file_counter = 0;     // Lives in DATA segment
static int file_uninit;          // Lives in BSS (zero)

// FUNCTION-SCOPE STATIC (retains value between calls)
void counter_function() {
    static int call_count = 0;   // Initialized ONCE
    call_count++;                 // Value persists
    printf("Called %d times\n", call_count);
}

// STATIC FUNCTION (file-private)
static void helper_function() {
    // Cannot be called from other files
}
```

### Characteristics
- Allocated at **compile time**
- Lives for **entire program** lifetime
- **Zero-initialized** if not explicitly set
- Only **one copy** exists
- **Thread-safe** initialization in C11+

---

## 2️⃣ STACK MEMORY (Deep Dive)

### Stack Frame Diagram
```
Before function call:                After function call (stack grows):
                                     
     TOP                                TOP
      │                                  │
      ▼                                  ▼
┌─────────────┐                    ┌─────────────┐
│  Caller's   │                    │  Caller's   │
│  variables  │                    │  variables  │
├─────────────┤                    ├─────────────┤
│             │                    │  Return     │
│  (free)     │  ←─────────────────│  address    │
│             │     push frame     ├─────────────┤
└─────────────┘                    │  Saved      │
                                   │  base ptr   │
                                   ├─────────────┤
                                   │  Local      │
                                   │  variables  │
                                   ├─────────────┤
                                   │  Parameters │
                                   └─────────────┘
```

### Stack Overflow Illustration
```
Normal stack:                    Stack overflow:
                                 
    TOP                                TOP
     │                                  │
     ▼                                  ▼
┌───────────┐                    ┌───────────┐
│ function1 │                    │ function1 │
├───────────┤                    ├───────────┤
│ function2 │                    │ function2 │
├───────────┤                    ├───────────┤
│ function3 │                    │ function3 │
├───────────┤                    ├───────────┤
│  (space)  │                    │ function4 │
│     │     │                    ├───────────┤
│     │     │                    │ function5 │
│     ▼     │                    ├───────────┤
└───────────┘                    │ function6 │
    BOTTOM                        ├───────────┤
                                  │    ⚠️     │
                                  │  CRASH!   │
                                  └───────────┘
                                      BOTTOM
```

### Code Examples
```c
// ✅ GOOD: Small stack usage
void good_function() {
    int small_array[100];        // 400 bytes (safe)
    char buffer[1024];           // 1KB (safe)
}

// ❌ BAD: Stack overflow risk
void bad_function() {
    int huge_array[10000000];    // 40MB → CRASH!
}

// ❌ DANGER: Returning stack address
int* dangerous() {
    int local = 42;
    return &local;               // UNDEFINED BEHAVIOR!
}

// ✅ FIX: Use heap
int* safe() {
    int *p = malloc(sizeof(int));
    *p = 42;
    return p;                    // Safe: caller must free()
}
```

### Stack Limits by Platform
| Platform | Default Stack Size | Check Command |
|----------|-------------------|----------------|
| Linux | 8MB | `ulimit -s` |
| macOS | 8MB | `ulimit -s` |
| Windows | 1MB | `dumpbin /headers` |
| Embedded | 256B - 4KB | Check linker script |

---

## 3️⃣ HEAP MEMORY (Deep Dive)

### Heap Allocation Flowchart
```
                    ┌─────────────┐
                    │   malloc()  │
                    │  calloc()   │
                    │  realloc()  │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │ Request from│
                    │    OS       │
                    └──────┬──────┘
                           │
            ┌──────────────┼──────────────┐
            │              │              │
            ▼              ▼              ▼
     ┌──────────┐   ┌──────────┐   ┌──────────┐
     │ Success  │   │  Failed  │   │ Partial  │
     │ returns  │   │ returns  │   │ returns  │
     │ pointer  │   │  NULL    │   │ smaller  │
     └────┬─────┘   └────┬─────┘   └────┬─────┘
          │              │              │
          ▼              ▼              ▼
    Use memory      Handle error    Use what's
                    (exit/retry)    available
```

### malloc vs calloc vs realloc
```
malloc(4 * sizeof(int)):

    Memory: [?] [?] [?] [?]     (garbage values)


calloc(4, sizeof(int)):

    Memory: [0] [0] [0] [0]     (all zeros)


realloc(ptr, 8 * sizeof(int)):

    Before: [1] [2] [3] [4]     (old data)
    
    After:  [1] [2] [3] [4] [?] [?] [?] [?]
            (old data preserved, new space uninit)
```

### Complete Heap Example
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // malloc - uninitialized
    int *arr1 = malloc(5 * sizeof(int));
    // arr1 contains garbage!
    
    // calloc - zero initialized
    int *arr2 = calloc(5, sizeof(int));
    // arr2 contains all zeros
    
    // realloc - resize
    int *temp = realloc(arr1, 10 * sizeof(int));
    if (temp) {
        arr1 = temp;  // OK: realloc succeeded
    } else {
        // realloc failed, original arr1 still valid
        free(arr1);
        return 1;
    }
    
    // Always check allocation!
    if (!arr2) {
        printf("Allocation failed\n");
        return 1;
    }
    
    // Use memory
    for (int i = 0; i < 5; i++) {
        arr2[i] = i + 1;
    }
    
    // MUST free both
    free(arr1);
    free(arr2);
    
    // Good practice: set to NULL
    arr1 = NULL;
    arr2 = NULL;
    
    return 0;
}
```

---

## 4️⃣ MEMORY LEAKS (Deep Dive)

### How Memory Leaks Happen
```
Time ─────────────────────────────────────────────────►

Allocation 1:   [=====used=====]  ✓ freed
Allocation 2:   [==========used==========]  ✗ NOT freed → LEAK!
Allocation 3:                       [=====used=====]  ✓ freed
Allocation 4:                             [=====used=====]  ✗ LEAK!

                     ↑
              Memory lost forever
              until program ends
```

### Leak Patterns
```c
// LEAK #1: Lost pointer
void leak1() {
    int *p = malloc(100);
    p = malloc(200);     // First allocation LOST!
    free(p);             // Only frees second allocation
}

// LEAK #2: Early return
void leak2() {
    int *p = malloc(100);
    if (some_error) {
        return;          // BUG: forgot to free p!
    }
    free(p);
}

// LEAK #3: No free at all
void leak3() {
    int *p = malloc(100);
    // use p...
    // forgot free() completely
}

// ✅ FIX: Always pair malloc with free
void fixed() {
    int *p = malloc(100);
    if (!p) return;
    
    // use p...
    
    free(p);
    p = NULL;
}
```

---

## 5️⃣ DANGLING POINTERS (Deep Dive)

### Visual Explanation
```
Step 1: Allocate and point
    p ──→ [some data] (valid memory)

Step 2: Free memory
    p ──→ [some data] (freed memory)
           ↑
    Dangling! Memory returned to OS

Step 3: Use dangling pointer
    p ──→ [? ? ? ? ?] (UNDEFINED BEHAVIOR!)
           ↑
    Could crash, corrupt data, or work randomly!
```

### Code Examples
```c
// DANGLING #1: Use after free
void dangling1() {
    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    *p = 100;   // ⚠️ WRITE to freed memory! (UB)
}

// DANGLING #2: Return stack address
int* dangling2() {
    int local = 42;
    return &local;   // ⚠️ local dies when function returns
}

// DANGLING #3: Out of scope
int* dangling3() {
    int arr[10];
    return arr;      // ⚠️ arr dies after return
}

// ✅ FIX: Set to NULL after free
void fixed() {
    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    p = NULL;        // Now safe: NULL check will catch
    if (p) {
        *p = 100;    // This won't execute
    }
}
```

---

## 6️⃣ DOUBLE FREE (Deep Dive)

### Visual
```
        free(p)           free(p) again
            │                  │
            ▼                  ▼
    ┌─────────────┐    ┌─────────────┐
    │ Memory is   │    │  ⚠️ CRASH!  │
    │ returned to │    │ Heap        │
    │ heap        │    │ corrupted   │
    └─────────────┘    └─────────────┘
```

### Code
```c
// ❌ BAD: Double free
void bad_free() {
    int *p = malloc(100);
    free(p);
    free(p);      // CRASH or heap corruption!
}

// ✅ GOOD: Set to NULL
void good_free() {
    int *p = malloc(100);
    free(p);
    p = NULL;     // Now safe
    // free(p);   // Would do nothing (free(NULL) is safe)
}

// free(NULL) is safe - does nothing
free(NULL);  // No problem
```

---

## 7️⃣ MEMORY FRAGMENTATION

### Visual
```
External Fragmentation:

Initial:  [████████████████████████████████]
          (free contiguous memory)

After allocations:  [█][ ][██][   ][█][    ][██]
                          ↑
                    Small gaps (can't use)

After frees:        [█][ ][██][   ][█][    ][██]
                    ↑   ↑    ↑   ↑   ↑     ↑
                    Free blocks scattered!
                    
Cannot allocate large block even though total free is enough!
```

### Solution: Memory Pool
```c
// Arena allocator (prevents fragmentation)
typedef struct Arena {
    char *buffer;
    size_t used;
    size_t size;
} Arena;

Arena* arena_create(size_t size) {
    Arena *a = malloc(sizeof(Arena));
    a->buffer = malloc(size);
    a->used = 0;
    a->size = size;
    return a;
}

void* arena_alloc(Arena *a, size_t bytes) {
    if (a->used + bytes > a->size) return NULL;
    void *ptr = a->buffer + a->used;
    a->used += bytes;
    return ptr;  // No free needed! Reset a->used to 0 to "free all"
}

void arena_destroy(Arena *a) {
    free(a->buffer);
    free(a);
}
```

---

## 8️⃣ MEMORY ALIGNMENT

### Visual
```
Memory with 4-byte alignment:

Address: 0  1  2  3  4  5  6  7  8  9  10 11
        [█][█][█][█][▓][▓][▓][▓][█][█][█][█]
         ↑           ↑           ↑
    Aligned      Aligned     Aligned
    (0,4,8)      (4,8,12)    (8,12)

Misaligned access (BAD):
        int *p = (int*)0x01;  // Not multiple of 4
        *p = 42;  // ⚠️ May crash or slow on some CPUs!
```

### Code
```c
#include <stdalign.h>

// Force 16-byte alignment (for SIMD, cache lines)
struct alignas(16) AlignedStruct {
    float data[4];  // Exactly 16 bytes
};

// Dynamic aligned allocation
int *aligned_ptr;
posix_memalign((void**)&aligned_ptr, 64, 1024);  // 64-byte aligned
free(aligned_ptr);

// C11 aligned_alloc
int *p = aligned_alloc(64, 1024);  // 64-byte alignment
free(p);
```

---

## 9️⃣ MEMORY DEBUGGING TOOLS

### Valgrind Flow
```
    Your Program
         │
         ▼
   ┌──────────┐
   │ Valgrind │ ← Tracks every malloc/free
   └──────────┘
         │
         ▼
    Execution
         │
         ▼
┌─────────────────────────────────┐
│ LEAK SUMMARY:                   │
│ definitely lost: 100 bytes      │ ← BAD
│ indirectly lost: 0 bytes        │
│ possibly lost: 0 bytes          │
│ still reachable: 0 bytes        │ ← OK
└─────────────────────────────────┘
```

### Tool Comparison

| Tool | Command | Detects |
|------|---------|---------|
| Valgrind | `valgrind ./prog` | Leaks, uninit, bad frees |
| ASAN | `gcc -fsanitize=address` | Overflow, use-after-free |
| UBSAN | `gcc -fsanitize=undefined` | Integer overflow, alignment |
| MSAN | `gcc -fsanitize=memory` | Uninitialized reads |
| LSAN | `gcc -fsanitize=leak` | Memory leaks only |

### Using Sanitizers
```bash
# Compile with Address Sanitizer
gcc -fsanitize=address -g -O1 program.c -o program

# Run (it will report issues)
./program

# With Leak Sanitizer
gcc -fsanitize=leak -g program.c -o program
./program
```

---

## 🧪 Complete Test Script (tests/run_tests.sh)

```bash
#!/bin/bash

echo "=== Memory Management Test Suite ==="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test 1: Static Memory
echo -e "\n${YELLOW}Test 1: Static Memory${NC}"
gcc ../src/static_example.c -o static_test
./static_test
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Static test passed${NC}"
else
    echo -e "${RED}✗ Static test failed${NC}"
fi

# Test 2: Dynamic Memory with Valgrind
echo -e "\n${YELLOW}Test 2: Dynamic Memory (Valgrind)${NC}"
gcc -g ../src/dynamic_example.c -o dynamic_test
valgrind --leak-check=full --error-exitcode=1 ./dynamic_test > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ No memory leaks detected${NC}"
else
    echo -e "${RED}✗ Memory leaks found!${NC}"
    valgrind --leak-check=full ./dynamic_test
fi

# Test 3: Address Sanitizer
echo -e "\n${YELLOW}Test 3: Address Sanitizer${NC}"
gcc -fsanitize=address -g ../src/dynamic_example.c -o asan_test
./asan_test > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ No address issues${NC}"
else
    echo -e "${RED}✗ Address issues detected${NC}"
fi

# Cleanup
rm -f static_test dynamic_test asan_test

echo -e "\n${GREEN}=== Tests Complete ===${NC}"
```

---

## 📁 Project Structure

```
memory-management-project/
│
├── docs/
│   ├── README.md
│   ├── static_memory.md
│   ├── dynamic_memory.md
│   ├── memory_diagrams.png
│
├── src/
│   ├── static_example.c
│   ├── dynamic_example.c
│   ├── mixed_usage.c
│
├── tests/
│   ├── run_tests.sh
│
└── LICENSE
```

---

## 💻 Source Code Examples

### 1. static_example.c
```c
#include <stdio.h>

static int counter = 0;   // static memory

void increase() {
    counter++;
    printf("Counter = %d\n", counter);
}

int main() {
    increase();  // Counter = 1
    increase();  // Counter = 2
    increase();  // Counter = 3
    return 0;
}
```

### 2. dynamic_example.c
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = malloc(n * sizeof(int));  // dynamic memory
    
    if (arr == NULL) {
        printf("Allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < n; i++) {
        arr[i] = i + 1;
        printf("%d ", arr[i]);
    }
    
    free(arr);  // important!
    arr = NULL;
    return 0;
}
```

### 3. mixed_usage.c
Shows stack + static + heap in a single program.

---

## 🚀 How to Run

```bash
# Static example
gcc src/static_example.c -o static
./static

# Dynamic example
gcc src/dynamic_example.c -o dynamic
./dynamic

# Mixed example
gcc src/mixed_usage.c -o mixed
./mixed

# Run all tests
cd tests
chmod +x run_tests.sh
./run_tests.sh
```

---

## 📚 Quick Reference Cheat Sheet

```c
// ========== STATIC ==========
static int x;              // File scope, zero-init
static int y = 5;          // File scope, init to 5
void f() { static int z; } // Value persists

// ========== STACK ==========
int local = 42;            // Auto freed
int arr[100];              // Fixed size
// NEVER return &local;

// ========== HEAP ==========
int *p = malloc(size);     // Uninit
int *z = calloc(n, size);  // Zero-init
p = realloc(p, new);       // Resize
free(p); p = NULL;         // Free + clear

// ========== COMMANDMENTS ==========
// 1. Check malloc return value
// 2. Every malloc needs a free
// 3. Set freed pointers to NULL
// 4. Don't return stack addresses
// 5. Use valgrind to verify
```

---

## ✅ Memory Management Checklist

- [ ] Every `malloc/calloc/realloc` has matching `free`
- [ ] Freed pointers set to `NULL`
- [ ] No large arrays on stack (>1KB)
- [ ] `malloc` return value checked for `NULL`
- [ ] No returning pointers to local variables
- [ ] No double frees
- [ ] Valgrind reports no leaks
- [ ] Address Sanitizer passes


