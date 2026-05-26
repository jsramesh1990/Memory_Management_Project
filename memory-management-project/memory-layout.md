# memory-layout

---

#  Introduction

In embedded systems, **memory layout** defines how a program is organized in physical memory (Flash, RAM, registers).

Every embedded application runs on a microcontroller where memory is limited and strictly structured.

Understanding memory layout is essential for:

* firmware development
* debugging crashes
* optimizing RAM usage
* writing drivers
* RTOS development
* linker script understanding

---

#  Definition

Memory layout is the **arrangement of program code, variables, stack, heap, and hardware registers in memory** during execution.

---

#  Embedded System Memory Architecture

```text id="mem1"
+-----------------------------+
| Flash / ROM                 |
|-----------------------------|
| .text  (Code)               |
| .rodata (Const data)        |
+-----------------------------+
| RAM                         |
|-----------------------------|
| .data  (Init globals)       |
| .bss   (Zero globals)       |
| Heap   (Dynamic memory)     |
| Stack  (Function calls)     |
+-----------------------------+
| Hardware Registers          |
+-----------------------------+
```

---

#  Memory Sections Overview

---

# 1. Text Section (.text)

##  Definition

Stores executable program instructions.

## Characteristics

* Read-only
* Stored in Flash/ROM
* Contains all functions

## Example

```c id="t1"
void main() {
    int a = 10;
}
```

---

## Flow

```text id="flow1"
Source code → Compiler → Machine code → Flash (.text)
```

---

# 2. Read-Only Data (.rodata)

##  Definition

Stores constant values like strings and const variables.

## Example

```c id="r1"
const int x = 100;
printf("Hello");
```

---

## Characteristics

* Stored in Flash
* Read-only
* Cannot be modified

---

# 3. Data Section (.data)

##  Definition

Stores **initialized global and static variables**.

## Example

```c id="d1"
int a = 10;
static int b = 20;
```

---

## Flow

```text id="flow2"
Flash (initial value)
      |
      v
Copied to RAM at startup
      |
      v
Used during execution
```

---

# 4. BSS Section (.bss)

##  Definition

Stores **uninitialized global and static variables** (initialized to 0).

## Example

```c id="b1"
int x;
static int y;
```

---

## Characteristics

* Stored in RAM
* Automatically zero-initialized
* Does not consume Flash space

---

## Flow

```text id="flow3"
Startup code
    |
    v
BSS cleared to zero
```

---

# 5. Heap

##  Definition

Used for **dynamic memory allocation** (`malloc`, `free`).

## Example

```c id="h1"
int *ptr = (int*)malloc(10 * sizeof(int));
```

---

## Characteristics

* Grows upward
* Programmer-managed
* Risk of fragmentation

---

## Flow

```text id="flow4"
malloc → heap allocates memory → returns pointer
free   → memory released
```

---

#  Embedded Note

Heap is often avoided in real-time systems because:

* unpredictable timing
* fragmentation
* memory leaks

---

# 6. Stack

## 📖 Definition

Used for:

* function calls
* local variables
* return addresses

## Example

```c id="s1"
void func() {
    int x = 10; // stored in stack
}
```

---

## Characteristics

* LIFO structure
* Automatically managed
* Fast access
* Limited size

---

## Flow

```text id="flow5"
Function call
    |
    v
Stack frame created
    |
    v
Local variables stored
    |
    v
Function return → stack cleared
```

---

#  Embedded System Full Execution Flow

```text id="flow6"
Power ON
   |
   v
Bootloader executes (.text)
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
while(1) loop runs
   |
   v
Stack + Heap used during runtime
```

---

#  Complete Memory Map

```text id="mem2"
+---------------------------+
| Flash / ROM             |
|---------------------------|
| .text (code)            |
| .rodata (constants)     |
+---------------------------+
| RAM                     |
|---------------------------|
| .data                   |
| .bss                    |
| Heap (dynamic memory)   |
| Stack (function calls)  |
+---------------------------+
| Peripheral Registers    |
+---------------------------+
```

---

#  Real-Time Embedded Example

```c id="ex1"
int globalVar = 10;   // .data
int uninitVar;        // .bss

const int maxVal = 100; // .rodata

void func() {
    int local = 5;    // stack
}

int main() {
    int *p = malloc(10); // heap
    func();
    return 0;
}
```

---

#  Comparison Table

| Section | Memory | Lifetime          | Managed By   |
| ------- | ------ | ----------------- | ------------ |
| .text   | Flash  | Program lifetime  | Compiler     |
| .rodata | Flash  | Program lifetime  | Compiler     |
| .data   | RAM    | Program lifetime  | Startup code |
| .bss    | RAM    | Program lifetime  | Startup code |
| Heap    | RAM    | Dynamic           | Programmer   |
| Stack   | RAM    | Function lifetime | Compiler     |

---

#  Common Issues

---

## 1. Stack Overflow

```text id="err1"
Too many function calls or large arrays → crash
```

---

## 2. Heap Fragmentation

```text id="err2"
Repeated malloc/free → broken memory blocks
```

---

## 3. Memory Corruption

```text id="err3"
Wrong pointer usage → overwrite stack/heap
```

---


