# memory-corruption.md

---

#  Introduction

In embedded systems, **memory corruption** happens when a program accidentally modifies memory that it is not supposed to access.

Since most microcontrollers do not have a Memory Management Unit (MMU), memory corruption can directly lead to:

* system crashes
* unpredictable behavior
* hardware register damage
* real-time failures
* device reboot (watchdog reset)

---

#  Definition

Memory corruption is a condition where **invalid memory access overwrites valid memory regions**, causing data loss or system instability.

---

#  Memory Corruption Flow

```text id="flow1"
Invalid pointer / buffer overflow
            |
            v
Writes outside allowed memory
            |
            v
Overwrites stack / heap / registers
            |
            v
Program behavior becomes unpredictable
            |
            v
System crash or reboot
```

---

#  Memory Layout (Where corruption happens)

```text id="mem1"
+--------------------------+
| Flash (Code)             |
|--------------------------|
| .text                    |
| .rodata                  |
+--------------------------+
| RAM                      |
|--------------------------|
| .data                    |
| .bss                     |
| Heap  ← corruption risk  |
| Stack ← corruption risk  |
+--------------------------+
```

---

#  Types of Memory Corruption

---

# 1. Buffer Overflow

##  Definition

Writing beyond allocated array size.

## Example

```c id="ex1"
int arr[5];

arr[10] = 100; //  memory corruption
```

---

## Flow

```text id="flow2"
Array boundary exceeded
        |
        v
Overwrites adjacent memory
        |
        v
Data corruption or crash
```

---

# 2. Stack Corruption

## Example

```c id="ex2"
void func() {
    int buffer[10];
    buffer[100] = 5; //  stack corruption
}
```

---

## Impact

* return address overwritten
* function jump failure
* crash or undefined behavior

---

# 3. Heap Corruption

## Example

```c id="ex3"
int *p = malloc(10 * sizeof(int));

p[20] = 5; //  heap overflow
```

---

## Impact

* heap metadata corruption
* malloc/free failure
* memory leaks

---

# 4. Wild Pointer Corruption

## Example

```c id="ex4"
int *ptr;
*ptr = 10; //  wild pointer
```

---

## Impact

* random memory overwrite
* hardware register damage
* system crash

---

# 5. Use-after-free Corruption

## Example

```c id="ex5"
int *p = malloc(10);

free(p);

p[0] = 5; //  memory corruption
```

---

#  Embedded System Impact

```text id="flow3"
Memory overwritten
        |
        v
Peripheral registers corrupted
        |
        v
Interrupt behavior changes
        |
        v
System becomes unstable
        |
        v
Watchdog reset / freeze
```

---

#  Real-Time Embedded Example

## UART Buffer Corruption

```c id="ex6"
char buffer[10];

void receive() {
    buffer[20] = 'A'; //  overwrites memory
}
```

---

## Result:

* UART may stop working
* other variables get corrupted

---

#  Why Embedded Systems are More Vulnerable

* No MMU (no protection)
* Direct memory access
* Hardware registers mapped in memory
* Real-time constraints
* No OS safety layer (bare-metal systems)

---

#  Corruption Visualization

```text id="diag1"
Before:
[ stack ][ heap ][ variables ]

After overflow:
[ stack ][ XXX corrupted XXX ][ variables ]
```

---

#  Corruption Propagation Flow

```text id="flow4"
One invalid write
        |
        v
Adjacent memory overwritten
        |
        v
Critical variable corrupted
        |
        v
Control logic fails
        |
        v
System crash
```

---

#  How to Prevent Memory Corruption

---

## 1. Array bounds checking

```c id="safe1"
if (i < 10) {
    arr[i] = 5;
}
```

---

## 2. Initialize pointers

```c id="safe2"
int *ptr = NULL;
```

---

## 3. Avoid unsafe indexing

```c id="safe3"
Use defined buffer sizes
```

---

## 4. Use static analysis tools

* MISRA C checks
* compiler warnings
* runtime sanitizers

---

## 5. Use safe memory patterns

```c id="safe4"
memcpy_s, bounded buffers
```

---

#  Common Mistakes

---

## 1. Buffer overflow

```c id="m1"
arr[100] = 1; // 
```

---

## 2. Pointer misuse

```c id="m2"
*ptr = 10; //  if ptr invalid
```

---

## 3. Wrong loop limits

```c id="m3"
for(i=0;i<=size;i++) //  off-by-one error
```

---

