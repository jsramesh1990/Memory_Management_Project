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
[![Beginner Friendly](https://img.shields.io/badge/Beginner-Friendly-success.svg)](https://en.wikipedia.org/wiki/Learning_curve)

This project demonstrates how memory works in real C applications by showcasing static memory, stack memory, and dynamic (heap) memory.
It includes example programs, documentation, and test scripts to help beginners understand how memory allocation works at the system level.

---

## ⚡ Quick Visual Guide (30 Seconds)

```
┌─────────────────────────────────────────────────────────────────┐
│                        MEMORY LAYOUT                             │
├──────────────┬──────────────────┬────────────────────────────────┤
│   STATIC     │      STACK        │           HEAP                 │
├──────────────┼──────────────────┼────────────────────────────────┤
│ • Compile    │ • Function calls  │ • Runtime allocation           │
│   time       │ • Auto freed      │ • You control lifetime         │
│ • Whole      │ • Very fast       │ • Must manually free           │
│   program    │ • Limited size    │ • Flexible size                │
│   lifetime   │   (1-8MB default) │ • Can leak if forgotten        │
└──────────────┴──────────────────┴────────────────────────────────┘
```

---

## 📊 Memory Type Quick Reference

| Memory Type | Keyword     | Lifetime          | Speed    | Size Limit       | Use When                    |
|-------------|-------------|-------------------|----------|------------------|-----------------------------|
| **Static**  | `static`    | Entire program    | Fast     | Compile-time     | Counters, constants, singletons |
| **Stack**   | (automatic) | Function scope    | Fastest  | ~1-8MB (OS dep)  | Local variables, small arrays |
| **Heap**    | `malloc()`  | Your control      | Slower   | RAM available    | Large data, unknown size at compile time |

---

## 🧠 Essential Memory Management Tips

### 🔑 Key Insight #1: Static Variables Live FOREVER
```c
static int counter = 0;  // Created once, never destroyed until program ends
```
**Trick:** Use static for counters that persist across function calls.

### 🔑 Key Insight #2: Stack is Automatic but Tiny
```c
int local = 42;  // Created on function entry, destroyed on exit
```
**Warning:** Don't allocate large arrays on stack (can crash):
```c
// ❌ BAD: char huge[10 * 1024 * 1024];  // 10MB on stack = CRASH
// ✅ GOOD: char *huge = malloc(10 * 1024 * 1024);  // Use heap instead
```

### 🔑 Key Insight #3: Heap Memory MUST Be Freed
```c
int *arr = malloc(100 * sizeof(int));  // You own this memory
// ... use arr ...
free(arr);  // ⚠️ YOU MUST DO THIS or memory leaks!
arr = NULL; // ✅ Good practice: prevent double-free bugs
```

---

## 🚨 Common Pitfalls & Fixes (Learn These Now!)

| 🚫 Pitfall | ❌ Bad Code | ✅ Good Code |
|------------|-------------|---------------|
| **Memory Leak** | `malloc(10);` (no free) | `free(ptr); ptr = NULL;` |
| **Use After Free** | `free(p); *p = 1;` | `free(p); p = NULL; // then check` |
| **Double Free** | `free(p); free(p);` | `free(p); p = NULL;` |
| **Stack Overflow** | `int big[10000000];` | Use heap: `malloc(10000000)` |
| **Dangling Pointer** | `return &local_var;` | Return heap pointer or global |
| **Forgetting sizeof** | `malloc(10 * int)` | `malloc(10 * sizeof(int))` |

---

## 🔥 Pro Techniques & Tricks

### Trick 1: Flexible Array Members (C99)
```c
typedef struct {
    int length;
    char data[];  // Flexible - size is 0 in struct
} flex_t;

flex_t *f = malloc(sizeof(flex_t) + 100);  // 100 bytes for data
f->length = 100;
```

### Trick 2: calloc vs malloc (Zero-initialization)
```c
int *p1 = malloc(10 * sizeof(int));   // Contains garbage values
int *p2 = calloc(10, sizeof(int));    // ✅ All zeros - safer for strings
```

### Trick 3: realloc Smart Usage
```c
int *p = malloc(100);
int *temp = realloc(p, 200);  // Try to resize
if (temp) p = temp;           // Success - update pointer
else { /* Handle failure - p still valid */ }
```

### Trick 4: Stack Size Check (Prevent Crashes)
```bash
# Check your stack limit
ulimit -s
# Increase if needed (Linux/Mac)
ulimit -s 16384  # 16MB stack
```

### Trick 5: Memory Leak Detection (Valgrind)
```bash
gcc -g your_program.c -o program
valgrind --leak-check=full ./program
# Look for: "definitely lost" → bad leak
#           "indirectly lost" → okay
```

---

## Security & Production Tips

```c
// ✅ Always check malloc/calloc return value
int *ptr = malloc(size);
if (ptr == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
}

// ✅ Use sizeof with variable, not type (safer for refactoring)
ptr = malloc(count * sizeof(*ptr));  // Better than sizeof(int)

// ✅ Set freed pointers to NULL
free(ptr);
ptr = NULL;  // Prevents use-after-free

// ✅ Use sanitizers during development
// gcc -fsanitize=address -g program.c
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

## 📝 What This Project Demonstrates

### 1. Static Memory

Allocated at compile time. Stored in the static region. Lifetime is the entire program.

```c
static int counter = 0;   // Value persists across function calls
```

**What you learn:** Static variables retain their value between function calls.

### 2. Stack Memory

Allocated when functions are called. Stores local variables and function frames. Automatically cleaned when function returns.

```c
int number = 10;   // Created on stack, destroyed when function exits
```

**What you learn:** Stack is automatic but limited in size.

### 3. Dynamic (Heap) Memory

Allocated at runtime using malloc/calloc/realloc. Size is flexible. Must be manually freed.

```c
int *arr = malloc(n * sizeof(int));  // Runtime size decision
free(arr);  // YOU must free it
```

**What you learn:** Heap gives control but requires responsibility.

---

## 💻 Source Code (src/)

### 1. static_example.c
Demonstrates static variable persistence:
```c
#include <stdio.h>

static int counter = 0;   // static memory - persists across calls

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
**Key takeaway:** `counter` retains its value between function calls.

### 2. dynamic_example.c
Demonstrates heap memory allocation:
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = malloc(n * sizeof(int));  // dynamic memory on heap
    
    if (arr == NULL) {  // Always check for allocation failure!
        printf("Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < n; i++) {
        arr[i] = i + 1;
        printf("%d ", arr[i]);
    }
    
    free(arr);  // ⚠️ CRITICAL: Manual cleanup
    arr = NULL; // Good practice
    return 0;
}
```
**Key takeaway:** Heap memory requires manual management.

### 3. mixed_usage.c
Shows stack + static + heap working together in a single program.

---

## 🚀 How to Run the Programs

Open terminal inside project root:

### Static Memory Example
```bash
gcc src/static_example.c -o static
./static
```

### Dynamic Memory Example
```bash
gcc src/dynamic_example.c -o dynamic
./dynamic
```

### Mixed Memory Usage
```bash
gcc src/mixed_usage.c -o mixed
./mixed
```

### With Address Sanitizer (Detects Memory Bugs!)
```bash
gcc -fsanitize=address -g src/dynamic_example.c -o dynamic_safe
./dynamic_safe
```

### Run All Tests Automatically
```bash
cd tests
./run_tests.sh
```

### Check for Memory Leaks
```bash
# Install valgrind first: sudo apt install valgrind (Linux)
valgrind --leak-check=full ./dynamic
```

---

## 🧪 Memory Debugging Tools Comparison

| Tool | Command | Detects |
|------|---------|---------|
| **Valgrind** | `valgrind ./prog` | Leaks, uninit memory, bad frees |
| **ASAN** | `gcc -fsanitize=address` | Buffer overflow, use-after-free |
| **UBSAN** | `gcc -fsanitize=undefined` | Integer overflow, alignment bugs |
| **GDB** | `gdb ./prog` | Inspect memory at runtime |

---

## 🎓 Memory Management Checklist

Before submitting code, verify:
- [ ] Every `malloc/calloc/realloc` has a matching `free`
- [ ] Freed pointers are set to `NULL`
- [ ] No large arrays allocated on stack (>1MB)
- [ ] `malloc` return value is checked for `NULL`
- [ ] No returning pointers to local (stack) variables
- [ ] No double frees
- [ ] Valgrind reports "All heap blocks were freed"

---

## 📚 One-Page Cheat Sheet

```c
// ========== STATIC MEMORY ==========
static int global_static;            // File scope, program lifetime
void func() { 
    static int counter;              // Value persists across calls
}

// ========== STACK MEMORY ==========
int local = 42;                      // Auto freed on return
int array[100];                      // Fixed size, fast access
// NEVER: return &local;             // Dangling pointer!

// ========== HEAP MEMORY ==========
int *p = malloc(size);               // Raw allocation
int *z = calloc(n, size);            // Zero-initialized
p = realloc(p, new_size);            // Resize (can move!)
free(p); p = NULL;                   // Free + clear

// ========== CRITICAL PATTERNS ==========
// Pattern 1: Always check malloc
if ((p = malloc(size)) == NULL) exit(1);

// Pattern 2: Always free
free(p);
p = NULL;

// Pattern 3: Use valgrind
// valgrind --leak-check=full ./program
```

---

## ✅ License

This project is open-source and released under the MIT License.

---

**⭐ Star this repo if it helped you understand C memory management!**
```

## Key improvements made:

1. **Added visual memory layout** - Immediate understanding at a glance
2. **Quick reference table** - When to use each memory type
3. **3 key insights** - The most important things to know instantly
4. **Common pitfalls table** - Learn from others' mistakes
5. **Pro techniques** - Flexible arrays, calloc vs malloc, realloc patterns
6. **Security tips** - NULL checks, sizeof patterns, sanitizers
7. **Debugging tools** - Valgrind, ASAN, UBSAN, GDB comparison
8. **Checklist** - Verify before submitting code
9. **One-page cheat sheet** - Quick reference
10. **All original content preserved** - Your badges, structure, and examples remain intact##    Memory Management in C
 Static, Stack & Dynamic (Heap) Memory

[![C Language](https://img.shields.io/badge/C-Programming-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Memory Management](https://img.shields.io/badge/Memory-Management-orange.svg)](https://en.wikipedia.org/wiki/Memory_management)
[![Static Memory](https://img.shields.io/badge/Static-Memory-yellow.svg)](https://en.wikipedia.org/wiki/Static_memory_allocation)
[![Stack Memory](https://img.shields.io/badge/Stack-Memory-green.svg)](https://en.wikipedia.org/wiki/Stack-based_memory_allocation)
[![Heap Memory](https://img.shields.io/badge/Heap-Dynamic-red.svg)](https://en.wikipedia.org/wiki/Heap_(programming))
[![Educational](https://img.shields.io/badge/Educational-Project-purple.svg)](https://en.wikipedia.org/wiki/Educational_software)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20WSL-lightgrey.svg)](https://en.wikipedia.org/wiki/Cross-platform)
[![GCC Compiler](https://img.shields.io/badge/Compiler-GCC%209+-blueviolet.svg)](https://gcc.gnu.org/)
[![Beginner Friendly](https://img.shields.io/badge/Beginner-Friendly-success.svg)](https://en.wikipedia.org/wiki/Learning_curve)

This project demonstrates how memory works in real C applications by showcasing static memory, stack memory, and dynamic (heap) memory.
It includes example programs, documentation, and test scripts to help beginners understand how memory allocation works at the system level.

#    Project Structure
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

#    What This Project Demonstrates
#    Static Memory

Allocated at compile time

Stored in the static region

Lifetime is the entire program

Example:
```
static int counter = 0;
```

#    Stack Memory

Allocated when functions are called

Stores local variables and function frames

Automatically cleaned when function returns

Example:
```
int number = 10;
```
#    Dynamic (Heap) Memory

Allocated at runtime using malloc / calloc / realloc

Size is flexible

Must be manually freed

Example:
```
int *arr = malloc(n * sizeof(int));
```

#    Source Code (src/)
1. static_example.c

Demonstrates the use of static variables:
```
#include <stdio.h>

static int counter = 0;   // static memory

void increase() {
    counter++;
    printf("Counter = %d\n", counter);
}

int main() {
    increase();
    increase();
    increase();
    return 0;
}
```

a)   Stored in static region
b)   Value persists across function calls

2. dynamic_example.c

Demonstrates heap memory allocation:
```
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = malloc(n * sizeof(int));  // dynamic memory

    for (int i = 0; i < n; i++) {
        arr[i] = i + 1;
        printf("%d ", arr[i]);
    }

    free(arr);  // important!
    return 0;
}

```
a)    Stored in heap
b)    Size controlled at runtime

3. mixed_usage.c

Shows stack + static + heap in a single program.

##    How to Run the Programs

Open terminal inside project root:

#    Static Memory Example
```
gcc src/static_example.c -o static
./static
```

#    Dynamic Memory Example
```
gcc src/dynamic_example.c -o dynamic
./dynamic
```

#    Mixed Memory Usage
```
gcc src/mixed_usage.c -o mixed
./mixed
```

#    Run All Tests Automatically
```
cd tests
./run_tests.sh
```

#    License

This project is open-source and released under the MIT License.

Here is an enhanced `README.md` that immediately teaches all critical C memory management techniques, tricks, and pitfalls at a glance. You can replace your current README with this.

```markdown
# 🧠 Memory Management in C – Complete Visual Guide

**Instantly understand how Static, Stack, and Heap memory work** with real code, system-level diagrams, and practical tricks used in production systems.

---

## ⚡ 30-Second Visual Summary

```
┌─────────────────────────────────────────────────────────────┐
│                         MEMORY LAYOUT                        │
├─────────────┬──────────────┬────────────────────────────────┤
│   STATIC    │    STACK      │           HEAP                 │
├─────────────┼──────────────┼────────────────────────────────┤
│ • Compile   │ • Function    │ • Runtime allocation           │
│   time      │   calls       │ • You control lifetime         │
│ • Whole     │ • Auto freed  │ • Must manually free           │
│   program   │ • Fast        │ • Flexible size                │
│   lifetime  │ • Limited     │ • Can cause leaks/fragmentation│
└─────────────┴──────────────┴────────────────────────────────┘
```

---

## 🎯 Quick Reference: When to Use What

| Memory Type | Keyword     | Lifetime          | Speed  | Size Limit       | Use When...                    |
|-------------|-------------|-------------------|--------|------------------|--------------------------------|
| **Static**  | `static`    | Entire program    | ⚡ Fast | Compile-time     | Counters, constants, singletons|
| **Stack**   | (automatic) | Function scope    | ⚡⚡ Fastest | ~1-8MB (default) | Local variables, small arrays   |
| **Heap**    | `malloc()`  | Your control      | 🐢 Slower| RAM available    | Large data, unknown size at compile time |

---

## 📁 Project Structure (What Each File Teaches You)

```
memory-management-project/
│
├── src/
│   ├── 01_static_basics.c      → Global & local static variables
│   ├── 02_stack_depth.c        → Stack overflow demonstration
│   ├── 03_heap_basics.c        → malloc, calloc, realloc, free
│   ├── 04_common_pitfalls.c    → Memory leaks, double free, dangling pointers
│   ├── 05_advanced_tricks.c    → Flexible arrays, alignment, arena allocator
│   └── 06_performance_compare.c → Speed comparison (stack vs heap)
│
├── tests/
│   ├── run_tests.sh            → Automatic memory leak detection with Valgrind
│   └── test_allocation_speed.c → Microbenchmark
│
└── docs/
    ├── memory_layout.png       → Visual memory diagram
    └── debugging_guide.md      → GDB/Valgrind/ASAN commands
```

---

##  Every Technique & Trick You Need

### 1️⃣ Static Memory – Deeper Than You Think

```c
#include <stdio.h>

// File-scope static: private to this file
static int file_private_counter = 0;

void demonstrate_static(void) {
    // Function-scope static: retains value between calls
    static int call_count = 0;
    call_count++;
    
    // ⚠️ TRICK: Static initialization is thread-safe in C11+
    static int once = 0;
    if (!once) {
        once = 1;
        printf("Initialized once\n");
    }
    
    printf("Called %d times\n", call_count);
}

// TRICK: Static functions are file-private
static void helper_function(void) { }
```

**Key insight:** Static variables live in **Data Segment** (initialized) or **BSS** (zero-initialized) – never on stack/heap.

### 2️⃣ Stack Memory – The Hidden Danger Zone

```c
void stack_dangers(void) {
    // ❌ DANGER: Large stack allocation → stack overflow!
    // char huge_array[1024 * 1024 * 10];  // 10MB - CRASH on most systems
    
    // ✅ FIX: Use heap for large allocations
    char *safe_array = malloc(10 * 1024 * 1024);
    
    // ⚠️ TRICK: Stack frame layout (x86-64 typical):
    // [parameters] [return addr] [saved rbp] [local vars]
    int local = 42;  // Lives on stack
    
    // ⚠️ NEVER return stack addresses!
    // return &local;  // UNDEFINED BEHAVIOR
}

// TRICK: Prevent stack overflow with compiler flag
// gcc -Wstack-usage=1024 -Werror  # Warn if >1KB stack per function
```

**Stack limits by platform:**
- Linux: 8MB (`ulimit -s` to see/change)
- Windows: 1MB default
- Embedded: Often 256 bytes to 4KB!

### 3️⃣ Heap Memory – Complete Mastery

```c
#include <stdlib.h>
#include <string.h>

void heap_mastery(void) {
    // ✅ malloc: uninitialized memory
    int *arr = malloc(10 * sizeof(int));
    
    // ✅ calloc: zero-initialized (slightly slower but safer for strings)
    int *zeroed = calloc(10, sizeof(int));
    
    // ✅ realloc: resize existing allocation
    int *resized = realloc(arr, 20 * sizeof(int));
    if (resized) arr = resized;  // realloc can move memory!
    
    // ⚠️ TRICK: realloc(NULL, size) == malloc(size)
    // ⚠️ TRICK: realloc(ptr, 0) == free(ptr) + returns NULL
    
    // ❌ DANGER: Memory leak!
    // Don't lose the original pointer before free:
    // arr = malloc(100); arr = malloc(200); // leak!
    
    free(arr);
    free(zeroed);
    // Always set to NULL after free to prevent double-free
    arr = NULL;
}

//  ADVANCED: Flexible Array Member (C99 trick)
typedef struct {
    int length;
    char data[];  // Flexible array – allocates 0 bytes in struct
} flex_t;

flex_t *create_flex(int size) {
    flex_t *f = malloc(sizeof(flex_t) + size);
    f->length = size;
    return f;
}
```

### 4️⃣ Memory Leak Prevention – Production Patterns

```c
// PATTERN 1: Always pair malloc with free in same scope
void safe_function(void) {
    void *ptr = malloc(100);
    if (!ptr) return;
    
    // ... use ptr ...
    
    free(ptr);
    ptr = NULL;  // Prevent double-free later
}

// PATTERN 2: Use goto for cleanup (Linux kernel style)
int kernel_style(void) {
    int *a = NULL, *b = NULL, ret = -1;
    
    a = malloc(100);
    if (!a) goto cleanup;
    b = malloc(100);
    if (!b) goto cleanup;
    
    // ... success path ...
    ret = 0;
    
cleanup:
    free(a);
    free(b);
    return ret;
}

// PATTERN 3: Compiler cleanup attribute (GCC/Clang)
__attribute__((cleanup(cleanup_func)))
void *auto_ptr = malloc(100);
// auto_ptr auto-freed when leaving scope!
```

### 5️⃣ Common Pitfalls – With Fixes

| Pitfall | Bad Code | Good Code |
|---------|----------|------------|
| **Use after free** | `free(p); *p = 1;` | `free(p); p = NULL;` |
| **Double free** | `free(p); free(p);` | `free(p); p = NULL;` |
| **Memory leak** | `p = malloc(10); p = malloc(20);` | Always free before reassign |
| **Buffer overflow** | `arr[10]` when size=10 | Use `arr[size-1]` |
| **Forget to free** | `malloc()` no `free()` | Use `valgrind` to detect |
| **Mismatched alloc/free** | `malloc()` + `delete` | `malloc()` + `free()` |

### 6️⃣ Performance Tricks & Secrets

```c
// TRICK 1: Stack is ~10-100x faster than heap
// Use stack for small, short-lived data

// TRICK 2: Memory pool / arena allocator (for game dev)
typedef struct { char *buffer; size_t used; size_t size; } Arena;
void *arena_alloc(Arena *a, size_t bytes) {
    if (a->used + bytes > a->size) return NULL;
    void *ptr = a->buffer + a->used;
    a->used += bytes;
    return ptr;
}
// No free() needed! Reset a->used = 0 to "free all"

// TRICK 3: Cache-line alignment (prevent false sharing)
#define CACHE_LINE 64
struct alignas(CACHE_LINE) {
    int counter;
    char padding[CACHE_LINE - sizeof(int)];
};

// TRICK 4: Use alloca() for dynamic stack allocation (careful!)
// void *temp = alloca(variable_size);
//  Freed automatically on return, but can overflow stack!
```

---

##  How to Run & Validate Your Code

### Single-File Compilation
```bash
# Static memory
gcc src/01_static_basics.c -o static_demo
./static_demo

# Heap memory with sanitizers (RECOMMENDED)
gcc -g -fsanitize=address -fsanitize=undefined src/03_heap_basics.c -o heap_demo
./heap_demo
```

### Complete Test Suite (with Leak Detection)
```bash
cd tests
chmod +x run_tests.sh
./run_tests.sh

# Manual Valgrind check
valgrind --leak-check=full --show-leak-kinds=all ./your_program
```

### Memory Debugging Tools Comparison

| Tool | Command | Best For |
|------|---------|----------|
| **Valgrind** | `valgrind ./prog` | Detecting leaks, uninit memory |
| **ASAN** | `gcc -fsanitize=address` | Speed + heap/stack overflow |
| **UBSAN** | `gcc -fsanitize=undefined` | Integer overflow, alignment |
| **GDB** | `gdb ./prog` | Inspect memory at breakpoints |

---

##  One-Page Cheat Sheet

```c
// ========== STATIC ==========
static int global_static;           // File scope
void func() { static int counter; } // Function scope, retains value

// ========== STACK ==========
int local = 42;                     // Auto freed
int array[100];                     // Fixed size, fast
// NEVER: return &local;            // Dangling pointer

// ========== HEAP ==========
int *p = malloc(size);              // Raw alloc
int *z = calloc(n, size);           // Zeroed alloc
p = realloc(p, new_size);           // Resize
free(p); p = NULL;                  // Free + clear

// ========== TRICKS ==========
sizeof(flex_array_struct) + length; // Flexible array
alloca(bytes);                      // Stack alloc (dangerous)
posix_memalign(&p, 64, size);       // Aligned alloc
#define CLEANUP __attribute__((cleanup))

// ========== VALGRIND ==========
// valgrind --leak-check=full ./prog
// == LEAK SUMMARY: definitely lost == bad
```

**Star this repo** ⭐ if it helped you understand C memory management!

