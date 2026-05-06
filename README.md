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

##  License

MIT License - Free for learning and production.

---

** Star this repo if it helped you!**
```
