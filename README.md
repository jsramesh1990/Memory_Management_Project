##    Memory Management in C
– Static, Stack & Dynamic (Heap) Memory

https://img.shields.io/badge/C-Programming-blue.svg
https://img.shields.io/badge/Memory-Management-orange.svg
https://img.shields.io/badge/Static-Memory-yellow.svg
https://img.shields.io/badge/Stack-Memory-green.svg
https://img.shields.io/badge/Heap-Dynamic-red.svg
https://img.shields.io/badge/Educational-Project-purple.svg
https://img.shields.io/badge/License-MIT-brightgreen.svg
https://img.shields.io/badge/Platform-Linux%2520%257C%2520macOS%2520%257C%2520WSL-lightgrey.svg
https://img.shields.io/badge/Compiler-GCC%25209+-blueviolet.svg
https://img.shields.io/badge/Beginner-Friendly-success.svg

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
