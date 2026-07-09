# Memory Fundamentals

## Process Memory Layout

### Visual Memory Map

```
                    HIGH MEMORY ADDRESSES
                    ┌─────────────────────────────┐
                    │       Environment Variables  │
                    │       Command Line Args      │
                    ├─────────────────────────────┤
                    │          STACK              │
                    │    (grows downward)         │
                    │                             │
                    │  ┌─────────────────────┐    │
                    │  │ Function Call Frames │    │
                    │  │ Local Variables      │    │
                    │  │ Return Addresses     │    │
                    │  │ Saved Registers      │    │
                    │  └─────────────────────┘    │
                    │         ↓ ↓ ↓               │
                    │                             │
                    │         (free space)        │
                    │                             │
                    │         ↑ ↑ ↑               │
                    │  ┌─────────────────────┐    │
                    │  │    HEAP             │    │
                    │  │ (grows upward)      │    │
                    │  │ Dynamic allocations  │    │
                    │  │ malloc/new          │    │
                    │  └─────────────────────┘    │
                    ├─────────────────────────────┤
                    │        BSS Segment          │
                    │  (Block Started by Symbol)   │
                    │  Uninitialized globals       │
                    │  Uninitialized statics       │
                    │  (zero-initialized at start) │
                    ├─────────────────────────────┤
                    │       Data Segment          │
                    │  Initialized globals         │
                    │  Initialized statics         │
                    │  String literals             │
                    ├─────────────────────────────┤
                    │       Text Segment          │
                    │  (Code Segment)             │
                    │  Executable instructions    │
                    │  Read-only                  │
                    └─────────────────────────────┘
                    LOW MEMORY ADDRESSES
```

## Detailed Memory Segment Breakdown

### 1. Text/Code Segment

**Characteristics:**
- Read-only (prevents accidental modification)
- Shared across multiple processes (saves memory)
- Contains machine code instructions
- Fixed size determined at compile time

**What's stored:**
- Function implementations
- Executable instructions
- Constant strings (depending on compiler)

**Protection flags:** `r-x` (read-execute)

### 2. Data Segment (Initialized Data)

**Characteristics:**
- Read-write
- Fixed size determined at compile time
- Loaded from executable file

**What's stored:**
- Global variables with initial values
- Static variables with initial values
- External variables with definitions

**Protection flags:** `rw-` (read-write)

### 3. BSS Segment (Block Started by Symbol)

**Characteristics:**
- Read-write
- Zero-initialized by OS before program starts
- Doesn't occupy space in executable file (just stores size)

**What's stored:**
- Uninitialized global variables
- Uninitialized static variables

**Protection flags:** `rw-` (read-write)

**Why BSS matters:**
- Saves executable file size
- Example: `int big_array[1000000];` takes 4MB in memory but 0 bytes in executable

### 4. Heap

**Characteristics:**
- Dynamic memory allocated at runtime
- Grows upward (toward higher addresses)
- Managed by malloc/calloc/realloc/free
- Requires manual memory management in C/C++

**What's stored:**
- Dynamically allocated data
- Objects created with `malloc()` or `new`

**Key properties:**
- Size can change during program execution
- Memory can be allocated/deallocated any order
- Access via pointers
- Prone to fragmentation

### 5. Stack

**Characteristics:**
- Automatic memory for function calls
- Grows downward (toward lower addresses)
- LIFO (Last-In-First-Out) structure
- Managed automatically by compiler

**What's stored per function call (Stack Frame):**
```
┌─────────────────────────────┐
│  Local variables            │
│  Function arguments         │
│  Return address             │
│  Saved frame pointer (ebp)  │
│  Saved registers            │
└─────────────────────────────┘
```

**Stack Frame Layout:**
```
    HIGHER ADDRESSES
    ┌──────────────────────┐
    │  Arguments (if any)   │ ← Previous frame
    ├──────────────────────┤
    │  Return Address       │
    ├──────────────────────┤
    │  Old Frame Pointer    │
    ├──────────────────────┤
    │  Saved Registers      │
    ├──────────────────────┤
    │  Local Variables      │
    │  (grows downward)     │
    └──────────────────────┘
    LOWER ADDRESSES
```

## Stack vs Heap - Deep Comparison

### Complete Comparison Table

| Aspect | Stack | Heap |
|--------|-------|------|
| **Memory Management** | Automatic (compiler) | Manual (programmer) |
| **Allocation Speed** | Extremely fast (few CPU cycles) | Slower (needs search algorithm) |
| **Deallocation Speed** | Instant (just move stack pointer) | Slow (insertion into free list) |
| **Memory Layout** | Sequential, contiguous | Random, scattered |
| **Size Limit** | Small (typically 1-8MB) | Large (available RAM - stack) |
| **Fragmentation** | None (perfectly sequential) | External and internal possible |
| **Lifetime** | Function scope only | Until explicitly freed |
| **Thread Safety** | Each thread has its own stack | Shared across threads (needs synchronization) |
| **Data Access** | Direct (through variable names) | Via pointers |
| **Resizing** | Not possible (fixed per function) | Possible via realloc |
| **Error on Full** | Stack overflow | Out of memory (NULL return) |
| **Cache Behavior** | Excellent (spatial locality) | Can be poor (random access) |
| **Memory Overhead** | None per allocation | Typical 8-32 bytes metadata per block |

### Visual Representation

```
STACK (LIFO - Ordered)
─────────────────────────────────────
Allocation sequence:  A  →  B  →  C
                            ↓
Stack pointer:        sp → C
                            B
                            A
─────────────────────────────────────
Deallocation (reverse order): C → B → A
Works like a stack of plates


HEAP (Unordered)
─────────────────────────────────────
Allocation sequence:  A  →  B  →  C
Memory layout:        A      B      C
                     [███]  [███]  [███]
                      不同地址， 不連續
─────────────────────────────────────
Deallocation (any order): B → A → C
Can leave gaps → FRAGMENTATION
Memory layout:        A      free   C
                     [███]  [····] [███]
                          ↑
                     Fragmentation hole
```

## Why Stack is Faster? - Detailed Analysis

### 1. **No Search Algorithm Required**
- Stack: Just move stack pointer (1-2 CPU instructions)
- Heap: Must traverse free list to find suitable block (O(n) search)

### 2. **Perfect Cache Locality**
- Stack allocations are sequential in memory
- CPU prefetcher predicts next stack accesses perfectly
- Heap allocations can be scattered across address space

### 3. **No Metadata Overhead**
- Stack: No need to store block sizes, next pointers, magic numbers
- Heap: Each allocation has 8-32 bytes of metadata before pointer

### 4. **No Synchronization Needed**
- Stack: Each thread has dedicated stack (no sharing)
- Heap: Multi-threaded allocators need locks or atomic operations

### 5. **Constant-Time Operations**
- Stack allocation/deallocation is O(1) always
- Heap best-case O(1) but worst-case can be O(n)

### Performance Numbers (Typical)

```
Operation              Stack           Heap
─────────────────────────────────────────────
Allocation 16 bytes    1-2 ns         50-100 ns
Allocation 1K bytes    1-2 ns         80-150 ns
Deallocation           1-2 ns         40-80 ns
Cache misses           Almost none    10-30% of accesses
```

## Stack Overflow vs Heap Overflow

### Stack Overflow

**Definition:** Exceeding the fixed stack size limit

**Common Causes:**
```
1. Infinite recursion
   void recurse() { recurse(); }  // No base case

2. Large stack allocations
   int huge_array[1000000];  // 4MB on stack!

3. Deep recursion chain
   // 100,000 nested function calls

4. Allocating large structures by value
   struct BigStruct data;  // if struct is 1MB
```

**Symptoms:**
- Segmentation fault (SIGSEGV)
- Program crashes immediately
- Corrupted return addresses
- Silent data corruption (hard to debug)

**Detection:**
- Guard pages at stack limit
- Compiler flags: `-fstack-protector`
- Runtime: Watch for segment violation

**Prevention:**
- Use iteration instead of recursion
- Move large arrays to heap
- Increase stack size (ulimit -s)
- Check recursion depth

### Heap Overflow

**Definition:** Writing beyond allocated heap memory bounds

**Common Causes:**
```
1. Buffer overflow
   char *ptr = malloc(10);
   strcpy(ptr, "this string is too long");

2. Off-by-one errors
   for(int i = 0; i <= size; i++)  // should be < size

3. Use-after-free
   free(ptr);
   ptr[0] = 'x';  // Writing to freed memory

4. Double free
   free(ptr);
   free(ptr);  // Corrupts heap metadata
```

**Symptoms (often delayed):**
- malloc/free crashes
- Strange program behavior
- Data corruption elsewhere
- Crashes in unrelated code
- May not crash immediately (hard to debug)

**Detection:**
```
Heap metadata corruption:
┌────────────┬────────────┬─────────────┐
│ size │ next │ magic │ used │  user data  │
└────────────┴────────────┴─────────────┘
     ↑
Overflow here corrupts metadata
→ Next malloc/free crashes
```

**Tools:**
- Valgrind (detects all heap errors)
- Address Sanitizer (ASAN)
- Heap debugging (`MALLOC_CHECK_=1`)
- Electric Fence

### Comparison Table

| Aspect | Stack Overflow | Heap Overflow |
|--------|---------------|---------------|
| **Detection** | Immediate (crash) | Delayed, random |
| **Debugging** | Easier (stack trace) | Hard (corruption spreads) |
| **Common cause** | Recursion, large locals | Buffer overrun, off-by-one |
| **Exploit type** | Return address overwrite | Function pointer overwrite |
| **Prevention** | Limit recursion depth | Bounds checking |
| **Detection tools** | Stack protector | Valgrind, ASAN |

## Memory Layout Examples

### Example 1: Where variables go

```c
// Global initialized - DATA segment
int initialized_global = 42;

// Global uninitialized - BSS segment
int uninitialized_global;

// Static initialized - DATA segment  
static int static_initialized = 100;

// Static uninitialized - BSS segment
static int static_uninitialized;

void function() {
    // Local variable - STACK
    int local_stack = 5;
    
    // Static inside function - DATA/BSS (only one instance)
    static int static_in_function = 10;
    
    // Dynamic allocation - HEAP
    int* heap_ptr = (int*)malloc(sizeof(int));
    *heap_ptr = 99;
    
    // String literal - TEXT segment (read-only)
    const char* str = "Hello";
    
    free(heap_ptr);
}
```

### Example 2: Memory in different scopes

```c
// Scope and lifetime demonstration
int* create_array(int size) {
    // BAD: returns pointer to stack memory (dangling pointer)
    int stack_array[100];  // Dies when function returns
    return stack_array;    // WRONG - undefined behavior
    
    // GOOD: returns heap memory
    int* heap_array = (int*)malloc(size * sizeof(int));
    return heap_array;     // OK - caller must free
}
```

## Key Interview Questions & Answers

### Q1: What's the maximum stack size?
**A:** OS dependent, typically 1-8MB per thread. Can be changed with `ulimit -s` (Linux) or compiler flags.

### Q2: Can stack and heap grow into each other?
**A:** Modern OS prevents this with guard pages. When they approach, stack overflow or allocation failure occurs.

### Q3: Why can't we allocate large arrays on stack?
**A:** Stack has limited size. Large allocations cause stack overflow, corrupting function return addresses.

### Q4: What's stack unwinding?
**A:** Process of destroying stack objects when function exits (calling destructors in C++, releasing stack space).

### Q5: How does compiler know stack frame size?
**A:** Calculates total size of all local variables + padding for alignment at compile time.

### Q6: What's the red zone in x86-64?
**A:** 128-byte area below stack pointer that can be used without adjusting RSP (optimization for leaf functions).

### Q7: Can heap memory be faster than stack?
**A:** Rarely. Only when stack causes cache thrashing (unlikely) or when using thread-local heaps.

## Common Pitfalls & Best Practices

### Stack Best Practices
- ✓ Keep recursion depth bounded (or use iteration)
- ✓ Large data (>1KB) goes to heap
- ✓ Be aware of stack size limits
- ✓ Use `alloca()` carefully (stack allocation like malloc)

### Heap Best Practices  
- ✓ Always check malloc return value
- ✓ Free what you allocate
- ✓ Set freed pointers to NULL
- ✓ Use tools (Valgrind) to detect leaks
- ✓ Consider memory pools for many small allocations

### Overflow Prevention
- ✓ Use `strncpy` instead of `strcpy`
- ✓ Always check buffer bounds
- ✓ Enable compiler warnings (`-Wall -Wextra`)
- ✓ Use Address Sanitizer during development
- ✓ Run tests with Valgrind

## Memory Layout Visualization Tools

### Linux
```bash
# View process memory map
cat /proc/<pid>/maps

# Check stack limits
ulimit -a

# Analyze binary sections
size executable
objdump -h executable
readelf -l executable
```

### Viewing memory layout at runtime
```c
// Print address of different segments
printf("Text: %p\n", main);
printf("Data: %p\n", &initialized_global);
printf("BSS:  %p\n", &uninitialized_global);
printf("Heap: %p\n", malloc(1));
printf("Stack: %p\n", &local_variable);
```

This comprehensive guide covers all the fundamentals of process memory layout that are essential for understanding memory allocators. Master these concepts before diving into allocator implementation!
