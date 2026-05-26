# memory
---

#  Introduction

In embedded systems, program memory is divided into different **memory sections** depending on the type of data stored and its lifetime.

These memory sections are managed by:

* compiler
* linker
* microcontroller architecture

Understanding memory layout is critical for:

* firmware development
* debugging crashes
* optimizing RAM usage
* driver development
* real-time system design

---

#  Definition

Memory sections are logical divisions of program memory that store:

* code
* global variables
* static variables
* dynamic memory
* stack data

---

#  Embedded System Memory Layout

```text id="mem1"
+----------------------+
| Flash / ROM         |  → Program Code (.text)
|----------------------|
| Initialized Data    |  → .data section
|----------------------|
| Uninitialized Data  |  → .bss section
|----------------------|
| Heap                |  → dynamic memory (malloc)
|----------------------|
| Stack               |  → local variables, function calls
+----------------------+
```

---

# ⚙ Memory Sections Explained

---

# 1. Text Section (.text)

##  Definition

Stores compiled program instructions (code).

## Characteristics

* Read-only
* Stored in Flash/ROM
* Executed by CPU

## Example

```c id="t1"
void main() {
    int a = 10;
}
```

Code of `main()` goes into `.text`.

---

#  Flow

```text id="flow1"
Source Code
   |
   v
Compiler
   |
   v
Machine Code → .text (Flash memory)
```

---

# 2. Data Section (.data)

##  Definition

Stores **initialized global and static variables**.

## Example

```c id="d1"
int a = 10;   // initialized global variable
static int b = 20;
```

## Characteristics

* Stored in RAM
* Copied from Flash during startup
* Read-write

---

#  Flow

```text id="flow2"
Flash (initial value)
        |
        v
Startup Code copies to RAM
        |
        v
.data section in RAM
```

---

# 3. BSS Section (.bss)

##  Definition

Stores **uninitialized global and static variables**.

## Example

```c id="b1"
int x;        // uninitialized global
static int y; // uninitialized static
```

## Characteristics

* Stored in RAM
* Automatically initialized to 0
* Does NOT occupy Flash storage

---

#  Flow

```text id="flow3"
Program Start
     |
     v
BSS cleared to zero by startup code
     |
     v
Variables initialized to 0
```

---

# 4. Heap

##  Definition

Heap is used for **dynamic memory allocation**.

## Example

```c id="h1"
int *ptr = (int*)malloc(10 * sizeof(int));
```

## Characteristics

* Grows upward
* Managed manually (`malloc/free`)
* Risk of fragmentation

---

#  Heap Flow

```text id="flow4"
malloc request
     |
     v
Heap allocates memory block
     |
     v
Pointer returns address
     |
     v
free() releases memory
```

---

#  Embedded Note

Heap is **rarely used in real-time embedded systems** due to:

* fragmentation
* unpredictable timing
* memory leaks

---

# 5. Stack

##  Definition

Stack is used for:

* function calls
* local variables
* return addresses

## Example

```c id="s1"
void func() {
    int x = 10; // stored in stack
}
```

## Characteristics

* LIFO (Last In First Out)
* Grows downward (in most MCUs)
* Automatically managed

---

#  Stack Flow

```text id="flow5"
Function Call
     |
     v
Stack Frame Created
     |
     v
Local variables stored
     |
     v
Function ends → Stack cleared
```

---

#  Full Memory Layout Diagram

```text id="mem2"
+------------------------+
| Flash (ROM)          |
|------------------------|
| .text (code)         |
| .rodata (const data) |
+------------------------+
| RAM                  |
|------------------------|
| .data (init globals) |
| .bss (zero globals)  |
| Heap (malloc)        |
| Stack (local vars)   |
+------------------------+
```

---

#  Embedded System Memory Flow

```text id="flow6"
Power ON
   |
   v
Bootloader runs (.text)
   |
   v
.data copied Flash → RAM
   |
   v
.bss cleared to 0
   |
   v
main() starts
   |
   v
while(1) loop executes
   |
   v
Stack + Heap used during runtime
```

---

#  Real-Time Example

```c id="ex1"
int globalVar = 10;   // .data
int uninitVar;        // .bss

void func() {
    int localVar = 5; // stack
}

int main() {
    int *p = malloc(10); // heap
    func();
    return 0;
}
```

---

#  Comparison Table

| Section | Location | Lifetime      | Managed By   |
| ------- | -------- | ------------- | ------------ |
| .text   | Flash    | Permanent     | Compiler     |
| .data   | RAM      | Program life  | Startup code |
| .bss    | RAM      | Program life  | Startup code |
| Heap    | RAM      | Dynamic       | Programmer   |
| Stack   | RAM      | Function life | Compiler     |

---

# ⚠ Common Problems

---

## 1. Stack Overflow

```text id="err1"
Too many function calls or large local variables
→ system crash
```

---

## 2. Heap Fragmentation

```text id="err2"
Repeated malloc/free → memory broken into pieces
```

---

## 3. Uninitialized Variables

```c id="err3"
int x; // garbage value if not in .bss handling properly
```

---
