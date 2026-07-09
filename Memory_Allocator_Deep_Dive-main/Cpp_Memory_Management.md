# C++ Memory Management

## Overview

C++ introduces object-oriented memory management with `new` and `delete` operators that go beyond simple memory allocation. Unlike C's `malloc/free`, C++ operators integrate with constructors and destructors, enabling the powerful RAII (Resource Acquisition Is Initialization) paradigm.

## Core Operators Comparison

```
C APPROACH                      C++ APPROACH
═══════════════════════════════════════════════════════════════════

malloc(sizeof(int))             new int
├─ Allocates raw memory         ├─ Allocates memory
└─ Returns void*                ├─ Calls constructor
                                └─ Returns typed pointer*

free(ptr)                       delete ptr
├─ Only frees memory            ├─ Calls destructor
└─ No object cleanup            └─ Frees memory

malloc(sizeof(int) * 10)        new int[10]
├─ Allocates raw array          ├─ Allocates memory for 10 ints
└─ Manual size tracking         ├─ Calls default constructors
                                └─ Tracks array size internally

free(arr)                       delete[] arr
├─ Frees memory only            ├─ Calls destructor for each element
└─ No element cleanup           └─ Frees memory
```

## Memory Layout Visualization

### Single Object Allocation

```
new int (single object)
═══════════════════════════════════════════════════════════════════

Memoria dinámica (Heap)
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                     HEAP MEMORY                          │    │
│  │  ┌───────────────┐                                      │    │
│  │  │  int value    │  ← 4 bytes (actual data)             │    │
│  │  │     42        │                                      │    │
│  │  └───────────────┘                                      │    │
│  │       ↑                                                 │    │
│  │       └── p points here                                 │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

Stack (local variable)          Heap (allocated memory)
┌─────────────────────┐         ┌─────────────────────┐
│    p (int*)         │────────→│     42 (int)        │
│   0x7FFF1234        │         │   0x5000ABCD        │
└─────────────────────┘         └─────────────────────┘
```

### Array Allocation with Internal Tracking

```
new int[5] (array allocation)
═══════════════════════════════════════════════════════════════════

Allocated memory layout (implementation dependent):

┌─────────────────────────────────────────────────────────────────┐
│  ┌──────────────┬──────────────────────────────────────────┐    │
│  │ Array Header │         Element Data                     │    │
│  │ (hidden)     │                                          │    │
│  ├──────────────┼────┬────┬────┬────┬────────────────────┤    │
│  │ count = 5    │[0] │[1] │[2] │[3] │[4]                 │    │
│  │              │ 10 │ 20 │ 30 │ 40 │ 50                 │    │
│  └──────────────┴────┴────┴────┴────┴────────────────────┘    │
│       ↑                                                        │
│       └── Internal metadata (not accessible to programmer)     │
│                                                                 │
│  User pointer (arr) points here →                              │
│  ════════════════════════════════════════════════════════      │
│  delete[] uses this metadata to know how many destructors      │
│  to call!                                                      │
└─────────────────────────────────────────────────────────────────┘

CRITICAL: delete[] reads the hidden count to call proper number
          of destructors. Using wrong delete causes leaks!
```

## Object Creation Flow

### Complete new/delete Lifecycle

```
new MyClass() CALLED
═══════════════════════════════════════════════════════════════════

Phase 1: Memory Allocation
┌─────────────────────────────────────────────────────────────────┐
│  operator new(sizeof(MyClass))                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ Allocate memory from heap (like malloc)                  │    │
│  │ Returns: void* raw_memory                                │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
Phase 2: Constructor Call
┌─────────────────────────────────────────────────────────────────┐
│  MyClass::MyClass(raw_memory)  ← Constructor executes           │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ • Initialize member variables                           │    │
│  │ • Allocate internal resources (if any)                  │    │
│  │ • Set up virtual table pointers                         │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
Phase 3: Return Typed Pointer
┌─────────────────────────────────────────────────────────────────┐
│  MyClass* ptr = static_cast<MyClass*>(raw_memory)               │
│                                                                  │
│  Result: Fully constructed object ready to use!                 │
└─────────────────────────────────────────────────────────────────┘


delete ptr CALLED
═══════════════════════════════════════════════════════════════════

Phase 1: Destructor Call
┌─────────────────────────────────────────────────────────────────┐
│  ptr->~MyClass()  ← Destructor executes                         │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ • Clean up member resources                             │    │
│  │ • Release allocated memory (delete members)             │    │
│  │ • Final cleanup                                         │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
Phase 2: Memory Deallocation
┌─────────────────────────────────────────────────────────────────┐
│  operator delete(ptr)  ← Free memory (like free)                │
│                                                                  │
│  Memory returned to heap for reuse                              │
└─────────────────────────────────────────────────────────────────┘
```

### Array Construction/Destruction Flow

```
new MyClass[3] CALLED
═══════════════════════════════════════════════════════════════════

Memory Layout with Array Cookie:
┌────────┬─────────┬─────────┬─────────┐
│ Cookie │ Obj[0]  │ Obj[1]  │ Obj[2]  │
│ count=3│         │         │         │
└────────┴─────────┴─────────┴─────────┘

Construction Order (0 → n-1):
Step 1: ┌─────────────────────────┐
        │ MyClass::MyClass(&obj[0]) │ ← Constructor for element 0
        └─────────────────────────┘
Step 2: ┌─────────────────────────┐
        │ MyClass::MyClass(&obj[1]) │ ← Constructor for element 1
        └─────────────────────────┘
Step 3: ┌─────────────────────────┐
        │ MyClass::MyClass(&obj[2]) │ ← Constructor for element 2
        └─────────────────────────┘


delete[] arr CALLED
═══════════════════════════════════════════════════════════════════

Read cookie: count = 3

Destruction Order (n-1 → 0):
Step 1: ┌──────────────────────────┐
        │ obj[2].~MyClass()        │ ← Destructor for element 2
        └──────────────────────────┘
Step 2: ┌──────────────────────────┐
        │ obj[1].~MyClass()        │ ← Destructor for element 1
        └──────────────────────────┘
Step 3: ┌──────────────────────────┐
        │ obj[0].~MyClass()        │ ← Destructor for element 0
        └──────────────────────────┘

Finally: operator delete[](arr) ← Free entire block including cookie
```

## malloc/free vs new/delete - Deep Comparison

### Visual Side-by-Side

```
MALLOC/FREE                      NEW/DELETE
═══════════════════════════════════════════════════════════════════

// Allocation                   // Allocation
┌─────────────────┐             ┌─────────────────┐
│ void* p =       │             │ MyClass* p =    │
│   malloc(size); │             │   new MyClass();│
└─────────────────┘             └─────────────────┘
        │                               │
        ▼                               ▼
┌─────────────────┐             ┌─────────────────┐
│ • Raw memory    │             │ • Allocates      │
│ • No type safety│             │ • Calls ctor ⭐  │
│ • void* return  │             │ • Returns typed* │
└─────────────────┘             └─────────────────┘

┌─────────────────┐             ┌─────────────────┐
│ // Use memory   │             │ // Use object   │
│ int* i =        │             │ p->method();    │
│   (int*)p;      │             │                 │
│ *i = 42;        │             │                 │
└─────────────────┘             └─────────────────┘

┌─────────────────┐             ┌─────────────────┐
│ // Deallocation │             │ // Deallocation │
│ free(p);        │             │ delete p;       │
└─────────────────┘             └─────────────────┘
        │                               │
        ▼                               ▼
┌─────────────────┐             ┌─────────────────┐
│ • Frees memory  │             │ • Calls dtor ⭐ │
│ • No cleanup    │             │ • Frees memory  │
└─────────────────┘             └─────────────────┘
```

### Complete Comparison Table

| Aspect | malloc/free | new/delete |
|--------|-------------|------------|
| **Type Safety** | Returns `void*`, need cast | Returns typed pointer |
| **Size Calculation** | Manual `sizeof(type)` | Automatic from type |
| **Initialization** | None (raw memory) | Constructor called |
| **Cleanup** | None (memory only) | Destructor called |
| **Error Handling** | Returns NULL | Throws `std::bad_alloc` |
| **Overridable** | No (but `mmap` etc.) | Yes (operator new/delete) |
| **Array Allocation** | Manual size tracking | Internal cookie tracking |
| **Memory Overhead** | Minimal (8-16 bytes) | May have cookie overhead |
| **Exception Safety** | Not applicable | Constructor can throw |
| **Required when** | C code, realloc needed | C++ objects, RAII |

## Critical C++ Concept: RAII

### What is RAII?

**Resource Acquisition Is Initialization** - A C++ idiom where resource management is tied to object lifetime.

```
RAII PRINCIPLE VISUALIZATION
═══════════════════════════════════════════════════════════════════

                    CONSTRUCTION
                         │
                         ▼
              ┌─────────────────────┐
              │   Object Created    │
              │   Resource Acquired │ ← File opened, mutex locked,
              └─────────────────────┘   memory allocated, etc.
                         │
                         ▼
              ┌─────────────────────┐
              │   Object LIVES      │
              │   Resource Held     │ ← Resource guaranteed valid
              └─────────────────────┘
                         │
                         ▼
                   (goes out of scope)
                         │
                         ▼
                    DESTRUCTION
                         │
                         ▼
              ┌─────────────────────┐
              │   Object Destroyed  │
              │   Resource Released │ ← File closed, mutex unlocked,
              └─────────────────────┘   memory freed automatically
```

### RAII Example - Buffer Management

```cpp
class Buffer {
private:
    int* ptr;
    size_t size;
    
public:
    // Constructor - ACQUIRES resource
    Buffer(size_t n) : size(n) {
        ptr = new int[n];          // Resource acquisition
        std::cout << "Buffer created with " << n << " elements\n";
    }
    
    // Destructor - RELEASES resource
    ~Buffer() {
        delete[] ptr;              // Resource release
        std::cout << "Buffer destroyed, memory freed\n";
    }
    
    // Prevent copying (Rule of Three/Five)
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    
    // Move constructor
    Buffer(Buffer&& other) noexcept 
        : ptr(other.ptr), size(other.size) {
        other.ptr = nullptr;
        other.size = 0;
    }
    
    int& operator[](size_t index) { return ptr[index]; }
};

void useBuffer() {
    Buffer buf(100);        // ⭐ CONSTRUCTOR: allocates memory
    buf[0] = 42;            // Use the buffer
    // ... do work ...
}   // ⭐ DESTRUCTOR: automatically frees memory!
    // No manual delete needed - LEAK-FREE!
```

### RAII Scope Visualization

```
void function() {
    ┌─────────────────────────────────────────────────────────────┐
    │  Buffer buf(100);    // Constructor called                  │
    │  ┌───────────────────────────────────────────────────────┐  │
    │  │  if (condition) {                                     │  │
    │  │      Buffer buf2(50);  // Constructor called          │  │
    │  │      // ...                                            │  │
    │  │  }   // ⭐ buf2 destructor called (auto cleanup)       │  │
    │  │                                                       │  │
    │  │  throw std::runtime_error("error");                   │  │
    │  │  // ⭐ buf destructor STILL called! (stack unwinding)  │  │
    │  └───────────────────────────────────────────────────────┘  │
    │  // ...                                                     │
    }   // ⭐ buf destructor called (auto cleanup)
    
    ⚡ EXCEPTION SAFE: Destructors run even when exceptions occur!
}
```

### RAII vs Manual Management

```cpp
// WITHOUT RAII (C-style) - BAD
void bad_example() {
    int* data = new int[1000];
    
    if (some_condition) {
        delete[] data;       // Must remember to free!
        return;
    }
    
    // ... more code ...
    
    if (error_occurs) {
        // MISSING delete[] - MEMORY LEAK!
        throw std::runtime_error("error");
    }
    
    delete[] data;           // Must remember at every exit point!
}
// Memory leak risk: 3 potential exit points, only 2 deletes!


// WITH RAII (C++ style) - GOOD
void good_example() {
    Buffer data(1000);       // RAII wrapper
    
    if (some_condition) {
        return;              // ⭐ Automatic cleanup!
    }
    
    // ... more code ...
    
    if (error_occurs) {
        throw std::runtime_error("error");  // ⭐ Automatic cleanup!
    }
    
    // ⭐ Automatic cleanup!
}
// No memory leak: Destructor runs on ANY exit path!
```

## new/delete vs malloc/free - When to Use What

### Decision Flowchart

```
START
  │
  ▼
┌─────────────────────────┐
│ Are you allocating      │
│ C++ objects?            │
└─────────────────────────┘
  │            │
 YES           NO
  │            │
  ▼            ▼
┌──────────┐  ┌──────────────────────┐
│ Use new/ │  │ Use malloc (for C    │
│ delete   │  │ compatibility only)  │
└──────────┘  └──────────────────────┘
  │
  ▼
┌─────────────────────────┐
│ Does the object need    │
│ constructor/destructor? │
└─────────────────────────┘
  │            │
 YES           NO
  │            │
  ▼            ▼
┌──────────┐  ┌──────────────────────────┐
│ MUST use │  │ Can use new or placement │
│ new      │  │ new (but why not RAII?)  │
└──────────┘  └──────────────────────────┘
```

### Common Pitfalls and Solutions

```
PITFALL 1: Mismatched delete
═══════════════════════════════════════════════════════════════════

int* p1 = new int;        // Single object
delete[] p1;              // WRONG! Undefined behavior

int* p2 = new int[10];    // Array
delete p2;                // WRONG! Only destroys first element

SOLUTION: Always match:
    new    → delete
    new[]  → delete[]


PITFALL 2: Forgetting virtual destructor
═══════════════════════════════════════════════════════════════════

class Base {
public:
    ~Base() { }  // ❌ Non-virtual destructor
};

class Derived : public Base {
    int* data;
public:
    Derived() : data(new int[1000]) { }
    ~Derived() { delete[] data; }  // This won't be called!
};

Base* obj = new Derived();
delete obj;  // ⚠️ Only Base destructor called → MEMORY LEAK!

SOLUTION: Add virtual destructor
class Base {
public:
    virtual ~Base() { }  // ✓ Virtual destructor
};


PITFALL 3: Constructor throws exception
═══════════════════════════════════════════════════════════════════

class Dangerous {
    int* p1;
    int* p2;
public:
    Dangerous() : p1(new int[100]), p2(new int[1000000000]) {
        // If second allocation throws, p1 is leaked!
    }
};

SOLUTION: Use RAII for members
class Safe {
    Buffer p1;  // RAII wrapper
    Buffer p2;
public:
    Safe(size_t s1, size_t s2) : p1(s1), p2(s2) {
        // If p2 constructor throws, p1 automatically cleans up!
    }
};
```

## Advanced C++ Memory Management

### Placement New

```cpp
// Allocate memory pool
char memory_pool[sizeof(MyClass) * 10];

// Construct object in pre-allocated memory
MyClass* obj = new(memory_pool) MyClass();

// Must manually call destructor
obj->~MyClass();

// Memory not freed (pool remains)
```

### Overriding new/delete for a Class

```cpp
class Tracked {
    static size_t allocations;
public:
    void* operator new(size_t size) {
        allocations++;
        std::cout << "Allocating " << size << " bytes\n";
        return ::operator new(size);
    }
    
    void operator delete(void* ptr) {
        allocations--;
        std::cout << "Deallocating\n";
        ::operator delete(ptr);
    }
    
    static size_t get_allocation_count() { return allocations; }
};

size_t Tracked::allocations = 0;
```

### Smart Pointers (Modern C++)

```
RAW POINTERS (C++98)           SMART POINTERS (C++11+)
═══════════════════════════════════════════════════════════════════

int* raw = new int(42);
delete raw;  ← Manual!         unique_ptr<int> smart = 
                                 make_unique<int>(42);
                               ⭐ Auto deletes!


void dangerous() {
    MyClass* p = new MyClass();
    throw error();  // LEAK!
}

void safe() {
    auto p = make_unique<MyClass>();
    throw error();  // ⭐ Auto cleanup!
}


SHARED OWNERSHIP (Reference counting):
auto sp1 = make_shared<MyClass>();
auto sp2 = sp1;  // Shared ownership
// Deletes when last reference dies
```

## Best Practices Summary

### DO's ✅

```cpp
// 1. Use RAII wrappers
std::vector<int> v(100);     // instead of new int[100]
std::unique_ptr<MyClass> p = std::make_unique<MyClass>();

// 2. Match new with delete, new[] with delete[]
int* arr = new int[100];
delete[] arr;  // NOT delete arr

// 3. Use virtual destructor in base classes
class Base {
public:
    virtual ~Base() = default;
};

// 4. Prefer smart pointers over raw
auto ptr = std::make_unique<MyClass>();

// 5. Use nullptr instead of NULL
int* p = nullptr;  // Type-safe!
```

### DON'Ts ❌

```cpp
// 1. Don't mix malloc/free with new/delete
void* p = malloc(100);
delete p;  // UNDEFINED BEHAVIOR!

// 2. Don't delete void*
void* p = new MyClass();
delete p;  // Destructor NOT called!

// 3. Don't forget array delete
int* arr = new int[100];
delete arr;  // Only first element destructor called (if class)

// 4. Don't return pointers to local objects (stack)
MyClass* bad() {
    MyClass obj;        // Stack object
    return &obj;        // Dangling pointer!
}

// 5. Don't manually call destructors (except placement new)
MyClass* p = new MyClass();
p->~MyClass();  // BAD! Double destructor when delete called
```

## Memory Leak Detection Patterns

```
LEAK DETECTION WITH RAII
═══════════════════════════════════════════════════════════════════

Leak detector using RAII:
┌─────────────────────────────────────────────────────────────────┐
│  class LeakDetector {                                           │
│      static size_t allocations;                                 │
│  public:                                                        │
│      void* operator new(size_t size) {                          │
│          allocations++;                                         │
│          return malloc(size);                                   │
│      }                                                          │
│      void operator delete(void* ptr) {                          │
│          allocations--;                                         │
│          free(ptr);                                             │
│      }                                                          │
│      static size_t leaks() { return allocations; }              │
│  };                                                             │
│                                                                 │
│  // At program end:                                             │
│  assert(LeakDetector::leaks() == 0);  // No leaks!              │
└─────────────────────────────────────────────────────────────────┘
```

## Summary Table: When to Use What

| Scenario | Recommended Approach | Why |
|----------|---------------------|-----|
| Local array | `std::array` or stack array | No heap allocation |
| Dynamic array | `std::vector` | RAII, automatic sizing |
| Single object | `std::unique_ptr` | Automatic cleanup |
| Shared ownership | `std::shared_ptr` | Reference counting |
| C compatibility | `malloc/free` | Only when necessary |
| Embedded systems | Custom allocator + placement new | Control over memory |
| Performance critical | Pool allocator | Reduce fragmentation |
| Polymorphic base | Virtual destructor + unique_ptr | Correct cleanup |

This comprehensive guide covers all aspects of C++ memory management with visual diagrams perfect for interview preparation and understanding the critical differences between C and C++ approaches!
