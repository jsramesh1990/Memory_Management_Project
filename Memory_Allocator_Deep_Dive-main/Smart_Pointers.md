# Smart Pointers

## Overview

Smart pointers are C++ RAII wrappers around raw pointers that automate memory management. They eliminate manual `delete` calls, prevent memory leaks, and provide exception safety. Introduced in C++11, they are now fundamental to modern C++ programming.

## The Three Smart Pointers

```
SMART POINTER FAMILY TREE
═══════════════════════════════════════════════════════════════════

                    ┌─────────────────┐
                    │   Smart Pointer │
                    │     (abstract)  │
                    └────────┬────────┘
                             │
            ┌────────────────┼────────────────┐
            │                │                │
            ▼                ▼                ▼
    ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
    │  unique_ptr  │  │  shared_ptr  │  │   weak_ptr   │
    ├──────────────┤  ├──────────────┤  ├──────────────┤
    │ Exclusive    │  │ Shared       │  │ Weak         │
    │ ownership    │  │ ownership    │  │ reference    │
    ├──────────────┤  ├──────────────┤  ├──────────────┤
    │ Move-only    │  │ Copyable     │  │ Copyable     │
    ├──────────────┤  ├──────────────┤  ├──────────────┤
    │ Zero overhead│  │ Has control  │  │ Must lock()  │
    │ (compared to │  │ block        │  │ to access    │
    │ raw pointer) │  │              │  │              │
    └──────────────┘  └──────────────┘  └──────────────┘
```

## 1. unique_ptr - Exclusive Ownership

### Concept Visualization

```
UNIQUE_PTR OWNERSHIP MODEL
═══════════════════════════════════════════════════════════════════

Single owner at any time:

    ┌─────────────┐         ┌─────────────┐
    │  unique_ptr │         │   Heap      │
    │    p1       │────────→│   Object    │
    └─────────────┘         │   42        │
                            └─────────────┘
    
    p2 = std::move(p1);  // Transfer ownership
    
    ┌─────────────┐         ┌─────────────┐
    │  unique_ptr │         │   Heap      │
    │    p1       │  (null) │   Object    │
    └─────────────┘         │   42        │
                            └─────────────┘
                                 ↑
    ┌─────────────┐              │
    │  unique_ptr │──────────────┘
    │    p2       │
    └─────────────┘

NO COPYING ALLOWED:
    unique_ptr<int> p1(new int(5));
    unique_ptr<int> p2 = p1;   // ❌ Compiler error!
```

### Implementation Internals

```cpp
// Simplified unique_ptr implementation
template<typename T>
class my_unique_ptr {
private:
    T* ptr;  // Raw pointer - ONLY member!
    
public:
    // Constructor
    explicit my_unique_ptr(T* p = nullptr) : ptr(p) {}
    
    // Destructor - automatically deletes
    ~my_unique_ptr() { delete ptr; }
    
    // NO copy constructor (delete)
    my_unique_ptr(const my_unique_ptr&) = delete;
    
    // NO copy assignment (delete)
    my_unique_ptr& operator=(const my_unique_ptr&) = delete;
    
    // Move constructor - transfers ownership
    my_unique_ptr(my_unique_ptr&& other) noexcept 
        : ptr(other.ptr) {
        other.ptr = nullptr;  // Source becomes null
    }
    
    // Move assignment
    my_unique_ptr& operator=(my_unique_ptr&& other) noexcept {
        if (this != &other) {
            delete ptr;           // Delete current
            ptr = other.ptr;      // Transfer ownership
            other.ptr = nullptr;
        }
        return *this;
    }
    
    // Access operators
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
    
    // Release ownership (return raw pointer)
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    
    // Reset to new pointer
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
};
```

### Memory Layout - unique_ptr

```
UNIQUE_PTR MEMORY LAYOUT
═══════════════════════════════════════════════════════════════════

Stack (sizeof = 8 bytes on 64-bit)
┌─────────────────────────────────────────────────────────────────┐
│  unique_ptr<int> p                                             │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  ptr (T*) ──────────────────────────────┐               │    │
│  └─────────────────────────────────────────┼───────────────┘    │
│  (ONLY the raw pointer - zero overhead!)    │                   │
└─────────────────────────────────────────────┼───────────────────┘
                                              │
                                              ▼
Heap                                          │
┌─────────────────────────────────────────────────────────────────┐
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  int value: 42                                          │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘

TOTAL OVERHEAD: 0 bytes (same as raw pointer!)
```

### unique_ptr Usage Examples

```cpp
// Creation
auto p1 = std::make_unique<int>(42);     // Preferred (exception safe)
std::unique_ptr<int> p2(new int(42));    // Okay but not preferred

// Transfer ownership
auto p3 = std::move(p1);                  // p1 becomes null
p2 = std::move(p3);                       // p2 now owns, p3 null

// Release ownership (careful!)
int* raw = p2.release();                  // p2 releases ownership
delete raw;                               // Must manually delete!

// Reset
p2.reset();                               // Deletes and sets null
p2.reset(new int(100));                   // Deletes old, owns new

// Custom deleter
auto file_deleter = [](FILE* f) { if(f) fclose(f); };
std::unique_ptr<FILE, decltype(file_deleter)> 
    file_ptr(fopen("test.txt", "r"), file_deleter);
```

## 2. shared_ptr - Shared Ownership

### Concept Visualization

```
SHARED_PTR OWNERSHIP MODEL
═══════════════════════════════════════════════════════════════════

Multiple owners, reference counting:

    ┌─────────────┐
    │  shared_ptr │         ┌─────────────────────────┐
    │    p1       │────────→│    CONTROL BLOCK        │
    └─────────────┘         │  ┌───────────────────┐  │
                            │  │ strong_count = 3  │  │
    ┌─────────────┐         │  │ weak_count   = 0  │  │
    │  shared_ptr │────────→│  │ deleter           │  │
    │    p2       │         │  │ allocator         │  │
    └─────────────┘         │  └───────────────────┘  │
                            └───────────┬─────────────┘
    ┌─────────────┐                     │
    │  shared_ptr │                     ▼
    │    p3       │────────→    ┌─────────────┐
    └─────────────┘              │   Heap      │
                                 │   Object    │
                                 │   42        │
                                 └─────────────┘

When strong_count reaches 0 → object destroyed
When both counts 0 → control block destroyed
```

### shared_ptr Internals - Deep Dive

```cpp
// Simplified shared_ptr internals
template<typename T>
class my_shared_ptr {
private:
    T* ptr;                    // Raw pointer to managed object
    control_block* ctrl;       // Pointer to control block
    
    struct control_block {
        size_t strong_count;   // Number of shared_ptrs
        size_t weak_count;     // Number of weak_ptrs
        void (*deleter)(T*);   // Custom deleter
        // ... possibly allocator, etc.
    };
    
public:
    // Constructor - creates control block
    explicit my_shared_ptr(T* p = nullptr) 
        : ptr(p), ctrl(new control_block{1, 0, default_deleter}) {}
    
    // Copy constructor - increments reference count
    my_shared_ptr(const my_shared_ptr& other) 
        : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->strong_count++;
    }
    
    // Destructor - decrements and cleans up
    ~my_shared_ptr() {
        if (ctrl && --ctrl->strong_count == 0) {
            ctrl->deleter(ptr);      // Delete object
            if (ctrl->weak_count == 0) {
                delete ctrl;         // Delete control block
            }
        }
    }
    
    // Assignment
    my_shared_ptr& operator=(const my_shared_ptr& other) {
        if (this != &other) {
            // Decrement current
            this->~my_shared_ptr();
            // Copy other
            ptr = other.ptr;
            ctrl = other.ctrl;
            if (ctrl) ctrl->strong_count++;
        }
        return *this;
    }
    
    size_t use_count() const { 
        return ctrl ? ctrl->strong_count : 0; 
    }
};
```

### Complete shared_ptr Memory Layout

```
SHARED_PTR MEMORY LAYOUT DETAIL
═══════════════════════════════════════════════════════════════════

Stack (sizeof = 16 bytes on 64-bit)
┌─────────────────────────────────────────────────────────────────┐
│  shared_ptr<int> p                                             │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  ptr (T*) ──────────────────────────┐                   │    │
│  ├─────────────────────────────────────┼───────────────────┤    │
│  │  ctrl (control_block*) ─────────────┼───────┐           │    │
│  └─────────────────────────────────────┼───────┼───────────┘    │
└─────────────────────────────────────────┼───────┼───────────────┘
                                          │       │
                                          ▼       ▼
Heap - Control Block                      │       │
┌─────────────────────────────────────────────────────────────────┐
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  strong_count = 3 (how many shared_ptrs)                │    │
│  │  weak_count   = 1 (how many weak_ptrs)                  │    │
│  │  deleter = &default_delete                               │    │
│  │  allocator                                               │    │
│  └─────────────────────────────────────────────────────────┘    │
│                              │                                  │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Managed Object: int value = 42                         │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘

TOTAL OVERHEAD: 
- Per shared_ptr: 8 bytes (control block pointer)
- Per control block: 24-32 bytes + object size
- Control block shared among all copies
```

### Reference Count Location - Critical Interview Question

```
WHERE IS THE REFERENCE COUNT STORED?
═══════════════════════════════════════════════════════════════════

OPTION 1: With the object (intrusive pointers)
┌─────────────────────────────────────────────────────────────────┐
│  Heap:                                                          │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  ┌──────────────────────┐                               │    │
│  │  │  reference_count     │  ← Inside the object itself  │    │
│  │  ├──────────────────────┤                               │    │
│  │  │  actual object data  │                               │    │
│  │  └──────────────────────┘                               │    │
│  └─────────────────────────────────────────────────────────┘    │
│  Pro: Single allocation, better cache locality                   │
│  Con: Requires intrusive design (enabled_shared_from_this)      │
└─────────────────────────────────────────────────────────────────┘

OPTION 2: Separate control block (shared_ptr's approach) ⭐
┌─────────────────────────────────────────────────────────────────┐
│  Two separate heap allocations:                                 │
│                                                                  │
│  ┌─────────────────────┐        ┌─────────────────────────┐     │
│  │   CONTROL BLOCK     │        │   MANAGED OBJECT        │     │
│  │  ┌───────────────┐  │        │  ┌───────────────────┐  │     │
│  │  │ strong_count  │  │        │  │ actual data       │  │     │
│  │  │ weak_count    │  │        │  │                   │  │     │
│  │  │ deleter       │  │        │  └───────────────────┘  │     │
│  │  │ allocator     │  │        │                         │     │
│  │  └───────────────┘  │        └─────────────────────────┘     │
│  └─────────────────────┘                                        │
│                                                                  │
│  Pro: Works with any type (non-intrusive)                        │
│  Con: Two allocations, more overhead                             │
└─────────────────────────────────────────────────────────────────┘

OPTION 3: Make shared (optimization)
┌─────────────────────────────────────────────────────────────────┐
│  Single allocation (control block + object together):           │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  ┌───────────────────┬─────────────────────────────┐    │    │
│  │  │  CONTROL BLOCK    │     MANAGED OBJECT          │    │    │
│  │  │  strong_count     │                             │    │    │
│  │  │  weak_count       │                             │    │    │
│  │  └───────────────────┴─────────────────────────────┘    │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
│  Used when: auto p = std::make_shared<T>(args...);               │
│  Pro: Single allocation, better performance                      │
│  Con: Control block lives as long as object (weak_ptr issues)   │
└─────────────────────────────────────────────────────────────────┘
```

### make_shared Optimization - Why It's Better

```cpp
// WITHOUT make_shared (two allocations)
shared_ptr<int> p1(new int(42));
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Stack         │     │   Heap          │     │   Heap          │
│  ┌───────────┐  │     │  ┌───────────┐  │     │  ┌───────────┐  │
│  │ ptr ──────┼──┼────→│  │ object    │  │     │  │ control   │  │
│  │ ctrl ─────┼──┼──┐  │  │ 42        │  │     │  │ block     │  │
│  └───────────┘  │  │  └───────────┘  │     │  └───────────┘  │
└─────────────────┘  │  └─────────────────┘     └─────────────────┘
                     └───────→ 2 separate allocations!

// WITH make_shared (single allocation)
auto p2 = make_shared<int>(42);
┌─────────────────┐     ┌─────────────────────────────────────────┐
│   Stack         │     │   Heap (single allocation)              │
│  ┌───────────┐  │     │  ┌───────────────────┬───────────────┐  │
│  │ ptr ──────┼──┼────→│  │   control block   │   object 42   │  │
│  │ ctrl ─────┼──┼──┐  │  │   strong_count=1  │               │  │
│  └───────────┘  │  │  │   weak_count=0      │               │  │
└─────────────────┘  │  └───────────────────┴───────────────┘  │
                     └───────→ 1 allocation (contiguous memory)   │
                                                                  │
Benefits:                                                         │
✓ Faster (one allocation instead of two)                          │
✓ Better cache locality                                           │
✓ Less memory fragmentation                                       │
✗ Control block not freed until weak_count=0 even if object gone  │
```

### shared_ptr Usage Examples

```cpp
// Creation - ALWAYS prefer make_shared
auto p1 = std::make_shared<int>(42);           // Preferred
std::shared_ptr<int> p2(new int(42));           // Okay (2 allocations)

// Reference counting in action
auto sp1 = std::make_shared<int>(100);
std::cout << sp1.use_count();                   // 1

auto sp2 = sp1;                                 // Copy
std::cout << sp1.use_count();                   // 2
std::cout << sp2.use_count();                   // 2

sp2.reset();                                    // Decrement count
std::cout << sp1.use_count();                   // 1

// Custom deleter
auto deleter = [](int* p) { 
    std::cout << "Deleting int: " << *p << "\n";
    delete p; 
};
std::shared_ptr<int> sp3(new int(999), deleter);

// Aliasing constructor (share ownership but point to different member)
struct MyStruct { int a; int b; };
auto sp4 = std::make_shared<MyStruct>();
std::shared_ptr<int> sp5(sp4, &sp4->b);  // Manages sp4, points to sp4->b
```

## 3. weak_ptr - Breaking Cycles

### The Problem: Circular References

```
CIRCULAR REFERENCE PROBLEM
═══════════════════════════════════════════════════════════════════

Situation: Object A points to B, B points to A

    ┌─────────────────────────────────────────────────────────────┐
    │  shared_ptr<A> a ──→ ┌─────────┐         ┌─────────┐        │
    │  shared_ptr<B> b ──→ │   A     │         │   B     │        │
    │                      │ ─────── │←────────│ ─────── │        │
    │                      │ sp_to_B │         │ sp_to_A │        │
    │                      └─────────┘         └─────────┘        │
    │                         ↑                   ↑               │
    │                         └───────────────────┘               │
    │                                                             │
    │  Both strong_counts = 1 (from a and b)                     │
    │  PLUS each points to other (extra strong reference!)       │
    │                                                             │
    │  Result: strong_count never reaches 0 → MEMORY LEAK!       │
    └─────────────────────────────────────────────────────────────┘
```

### Solution: weak_ptr

```
WEAK_PTR BREAKING CYCLES
═══════════════════════════════════════════════════════════════════

Use weak_ptr for one direction:

    ┌─────────────────────────────────────────────────────────────┐
    │  shared_ptr<A> a ──→ ┌─────────┐         ┌─────────┐        │
    │                      │   A     │         │   B     │        │
    │                      │ ─────── │─weak───→│ ─────── │        │
    │                      │ weak_to_B│         │ sp_to_A │        │
    │                      └─────────┘         └─────────┘        │
    │                         ↑                   ↑               │
    │                         └───────────────────┘               │
    │                                                             │
    │  strong_count (B) = 1 (from A's weak_ptr doesn't count)   │
    │  strong_count (A) = 1 (from B's shared_ptr)                │
    │  When a goes out of scope:                                 │
    │    A's strong_count becomes 0 → A destroyed               │
    │    B's strong_count becomes 0 → B destroyed               │
    │    ✓ Clean destruction!                                    │
    └─────────────────────────────────────────────────────────────┘
```

### weak_ptr Internals

```cpp
// Simplified weak_ptr implementation
template<typename T>
class my_weak_ptr {
private:
    T* ptr;                    // Raw pointer (may be dangling)
    control_block* ctrl;       // Shared control block
    
public:
    // Construct from shared_ptr (doesn't increment strong_count)
    my_weak_ptr(const my_shared_ptr<T>& sp) 
        : ptr(sp.ptr), ctrl(sp.ctrl) {
        if (ctrl) ctrl->weak_count++;
    }
    
    // Lock to get shared_ptr (if object still alive)
    my_shared_ptr<T> lock() const {
        if (ctrl && ctrl->strong_count > 0) {
            return my_shared_ptr<T>(*this);  // Creates shared_ptr
        }
        return my_shared_ptr<T>();           // Empty
    }
    
    // Check if object still exists
    bool expired() const {
        return !ctrl || ctrl->strong_count == 0;
    }
    
    ~my_weak_ptr() {
        if (ctrl && --ctrl->weak_count == 0 && ctrl->strong_count == 0) {
            delete ctrl;  // Delete control block when last to leave
        }
    }
};
```

## Critical Interview Questions & Answers

### Q1: What's the difference between shared_ptr and unique_ptr?

```
UNIQUE_PTR                    SHARED_PTR
═══════════════════════════════════════════════════════════════════
Single ownership             Multiple ownership
Cannot be copied             Can be copied
Move-only                    Copyable
Zero overhead (8 bytes)      Overhead (16 bytes + control block)
No reference counting        Reference counting
Faster operations            Slower (atomic operations)
Cannot be converted to weak  Works with weak_ptr
Make_unique (C++14)          Make_shared (C++11)
Use when ownership clear     Use when sharing needed
```

### Q2: Where is the reference count stored? (Deep dive)

```cpp
// Demonstration showing separate control block
int main() {
    auto p1 = std::make_shared<int>(42);
    auto p2 = p1;
    auto p3 = p1;
    
    // The count is NOT in the object
    // The count is NOT in the shared_ptr
    // The count is in a SEPARATE control block on the heap
    
    // Memory layout:
    // Stack: p1 (ptr, ctrl) → Heap: Control Block (count=3) → Object (42)
    // Stack: p2 (ptr, ctrl) ──┘
    // Stack: p3 (ptr, ctrl) ──┘
    
    std::cout << p1.use_count();  // 3 - retrieves from control block
}
```

### Q3: What happens in make_shared vs new + shared_ptr?

```cpp
// Option 1: Two allocations
shared_ptr<int> p1(new int(42));
// Allocation 1: control block
// Allocation 2: int object
// Two separate heap allocations → slower, more fragmentation

// Option 2: One allocation (make_shared)
auto p2 = make_shared<int>(42);
// Single allocation: control block + object contiguously
// Faster, better cache locality

// BUT: weak_ptr with make_shared keeps control block alive longer
weak_ptr<int> wp = p2;
p2.reset();           // Object destroyed
// But control block remains (weak_count > 0) until wp dies
// With separate allocations, control block could be freed earlier
```

### Q4: Can unique_ptr be converted to shared_ptr?

```cpp
unique_ptr<int> up = make_unique<int>(42);
shared_ptr<int> sp = std::move(up);  // ✓ Yes, transfers ownership
// up is now null

// But NOT other way around:
shared_ptr<int> sp2 = make_shared<int>(100);
unique_ptr<int> up2 = std::move(sp2);  // ❌ Compiler error!
// shared_ptr cannot be converted to unique_ptr (would break sharing)
```

### Q5: What's enable_shared_from_this?

```cpp
// Problem: Getting shared_ptr from inside class
class Bad {
public:
    void dangerous() {
        shared_ptr<Bad> this_sp(this);  // Creates NEW control block!
        // Two independent control blocks for same object → DOUBLE DELETE!
    }
};

// Solution: enable_shared_from_this
class Good : public std::enable_shared_from_this<Good> {
public:
    void safe() {
        auto this_sp = shared_from_this();  // Returns existing shared_ptr
        // Works with the original control block
    }
};

int main() {
    auto p = make_shared<Good>();
    p->safe();  // ✓ Safe - uses same control block
}
```

## Smart Pointer Comparison Matrix

| Feature | unique_ptr | shared_ptr | weak_ptr |
|---------|------------|------------|----------|
| **Ownership** | Exclusive | Shared | No ownership |
| **Memory overhead** | 8 bytes | 16 bytes | 16 bytes |
| **Control block** | No | Yes (24-32 bytes) | Yes (shared) |
| **Copyable** | ❌ (move-only) | ✓ | ✓ |
| **Reference count** | N/A | Yes (atomic) | No |
| **Null check** | ✓ | ✓ | via lock() |
| **Custom deleter** | ✓ (type-erased) | ✓ | N/A |
| **Use case** | Clear single owner | Shared resources | Breaking cycles |
| **Performance** | Fastest | Moderate | N/A |
| **Thread safety** | N/A | Reference count atomic | lock() atomic |

## Common Pitfalls and Solutions

### Pitfall 1: Cyclic References

```cpp
// BAD: Memory leak!
struct Node {
    std::shared_ptr<Node> next;
    ~Node() { std::cout << "Deleted\n"; }
};

auto n1 = make_shared<Node>();
auto n2 = make_shared<Node>();
n1->next = n2;
n2->next = n1;  // Cycle! Never deleted

// GOOD: Use weak_ptr
struct NodeSafe {
    std::shared_ptr<NodeSafe> next;
    std::weak_ptr<NodeSafe> prev;  // One direction weak
    ~NodeSafe() { std::cout << "Deleted\n"; }
};
```

### Pitfall 2: Using get() Incorrectly

```cpp
shared_ptr<int> sp = make_shared<int>(42);
int* raw = sp.get();
// DON'T delete raw pointer!
delete raw;  // ❌ Double delete when sp dies

// DON'T create new shared_ptr from raw
shared_ptr<int> sp2(raw);  // ❌ Two control blocks!

// DO: Just use sp normally
*sp = 100;  // ✓ Safe
```

### Pitfall 3: Shared from this without inheritance

```cpp
class Bad {
public:
    void func() {
        auto self = shared_from_this();  // ❌ Error! Not derived
    }
};

class Good : public enable_shared_from_this<Good> {
public:
    void func() {
        auto self = shared_from_this();  // ✓ Works
    }
};

int main() {
    Good g;  // On stack!
    g.func();  // ❌ Runtime error (not managed by shared_ptr)
    
    auto g2 = make_shared<Good>();
    g2->func();  // ✓ Works
}
```

## Best Practices Summary

### DO's ✅

```cpp
// 1. Prefer make_unique/make_shared over raw new
auto p = std::make_unique<MyClass>(args...);  // Exception safe

// 2. Use unique_ptr for exclusive ownership
std::unique_ptr<MyClass> factory() {
    return std::make_unique<MyClass>();
}

// 3. Use shared_ptr only when truly shared
class Cache {
    std::shared_ptr<Data> getData() { return cached_data; }
};

// 4. Use weak_ptr to break cycles
class Parent {
    std::vector<std::shared_ptr<Child>> children;
};
class Child {
    std::weak_ptr<Parent> parent;  // Weak back-reference
};

// 5. Use .get() only for legacy APIs that need raw pointers
void legacy_api(int* p);
auto sp = std::make_unique<int>(42);
legacy_api(sp.get());  // ✓ OK, but don't store or delete

// 6. Check weak_ptr before use
std::weak_ptr<int> wp;
if (auto sp = wp.lock()) {
    // Use sp safely
}
```

### DON'Ts ❌

```cpp
// 1. Don't use raw new with smart pointers (use make functions)
std::shared_ptr<int> p(new int(42));  // ❌ Two allocations

// 2. Don't create cycles with shared_ptr
struct A { std::shared_ptr<A> next; };  // ❌ Danger!

// 3. Don't use shared_ptr when unique_ptr works
class Widget {
    std::shared_ptr<Gadget> g;  // ❌ Overkill if exclusive
};

// 4. Don't return raw pointers from factories
MyClass* factory() { return new MyClass(); }  // ❌ Who deletes?

// 5. Don't cast smart pointers without proper rules
auto sp = std::make_shared<Derived>();
std::shared_ptr<Base> bp = sp;  // ✓ OK (implicit)
// auto dp = std::dynamic_pointer_cast<Derived>(bp);  // ✓ OK

// 6. Don't store raw pointers from smart pointers long-term
auto sp = std::make_shared<int>(42);
int* raw = sp.get();  // ❌ Dangerous - sp might die
```

## Performance Characteristics

```
PERFORMANCE COMPARISON (relative to raw pointer)
═══════════════════════════════════════════════════════════════════

Operation              raw ptr  unique_ptr  shared_ptr
──────────────────────────────────────────────────────────────────
Create/Destroy         1x       1-2x        3-5x
Copy                   N/A      N/A (move)  2-3x (+atomic)
Dereference            1x       1x          1x
Memory overhead        0 bytes  8 bytes     16-32 bytes
Context switching      1x       1x          1.5x (atomic ops)

SHARED_PTR overhead breakdown:
┌─────────────────────────────────────────────────────────────────┐
│  Per shared_ptr: 16 bytes (ptr + ctrl)                         │
│  Per control block: 24-32 bytes                                │
│  Per copy: atomic increment (5-10 ns on modern CPU)           │
│  Per destroy: atomic decrement + possible delete (10-20 ns)   │
└─────────────────────────────────────────────────────────────────┘
```

## Thread Safety Guarantees

```
SHARED_PTR THREAD SAFETY
═══════════════════════════════════════════════════════════════════

✓ Reference counting is thread-safe (atomic operations)
✓ Different shared_ptrs to same object can be used in different threads
✓ const methods (get(), use_count()) are thread-safe

✗ Object content is NOT automatically thread-safe
✗ Resetting one shared_ptr while another uses it is race condition

┌─────────────────────────────────────────────────────────────────┐
│  Thread 1:          │  Thread 2:                               │
│  auto sp2 = sp1;    │  auto sp3 = sp1;  // ✓ Safe (copy)      │
│  *sp2 = 10;         │  *sp3 = 20;       // ✗ Race on object!  │
└─────────────────────────────────────────────────────────────────┘

Solution: Use mutex to protect object content
```

This comprehensive guide covers everything about smart pointers with detailed internals, memory layouts, and interview-focused explanations!
