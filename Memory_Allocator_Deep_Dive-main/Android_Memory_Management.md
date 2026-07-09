# Android Memory Management

## Overview

Android memory management combines Linux kernel memory management with Dalvik/ART runtime memory management, creating a unique hybrid system. Understanding Android's memory model is crucial for developing high-performance Android applications and diagnosing memory issues.

## Android Memory Architecture

```
ANDROID MEMORY STACK
═══════════════════════════════════════════════════════════════════

                    ANDROID APPLICATION
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│  Java/Kotlin  │    │   Native C++  │    │    Render     │
│     Code      │    │     Code      │    │    Thread     │
└───────┬───────┘    └───────┬───────┘    └───────┬───────┘
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│   ART/Dalvik  │    │      JNI      │    │   Skia/OpenGL │
│     Heap      │    │    (Glue)     │    │    Buffers    │
└───────┬───────┘    └───────┬───────┘    └───────┬───────┘
        │                    │                    │
        └────────────────────┼────────────────────┘
                             │
                             ▼
              ┌─────────────────────────────┐
              │     Linux Kernel Allocator   │
              │     (kmalloc, vmalloc, etc)  │
              └─────────────────────────────┘
                             │
                             ▼
              ┌─────────────────────────────┐
              │         Physical RAM         │
              └─────────────────────────────┘

MEMORY TYPES IN ANDROID APP:
═══════════════════════════════════════════════════════════════════
Type                    Managed by           Example
───────────────────────────────────────────────────────────────────
Java Heap               Garbage Collector     new Object()
Native Heap             C++ new/delete        malloc(), new
Graphics Memory         SurfaceFlinger        Bitmap pixels
Code Memory             Dynamic loader        .so files
Stack                   OS                    Local variables
Binder Memory           Binder driver         IPC buffers
Ashmem                  Kernel                Shared memory
```

## ART/Dalvik Runtime Heap

### Garbage Collection Basics

```
GARBAGE COLLECTION CONCEPT
═══════════════════════════════════════════════════════════════════

WHAT IS GARBAGE COLLECTION?
───────────────────────────────────────────────────────────────────
Automatic memory management that identifies and reclaims memory no longer in use.

VISUAL REPRESENTATION:
───────────────────────────────────────────────────────────────────
BEFORE GC (Objects in heap):
┌─────────────────────────────────────────────────────────────────┐
│  Object A    Object B    Object C    Object D    Object E      │
│  [ROOT]      [REF A]     [REF B]     [FREE]      [REF C]       │
│                                                      │          │
│  Object F    Object G    Object H    Object I    Object J       │
│  [REF E]     [FREE]      [REF F]     [FREE]      [REF H]       │
└─────────────────────────────────────────────────────────────────┘

Objects reachable from roots (─► live): A, B, C, E, F, H, J
Objects unreachable (➡ garbage): D, G, I

AFTER GC (Mark and Sweep):
┌─────────────────────────────────────────────────────────────────┐
│  Object A    Object B    Object C    [FREE]      Object E      │
│  [ROOT]      [REF A]     [REF B]                 [REF C]       │
│                                                      │          │
│  Object F    [FREE]      Object H    [FREE]      Object J       │
│  [REF E]                 [REF F]                 [REF H]       │
└─────────────────────────────────────────────────────────────────┘

Memory compacted (or free list updated)
```

### Android Heap Generations

```
ART HEAP GENERATIONS
═══════════════════════════════════════════════════════════════════

ANDROID'S GENERATIONAL HEAP (ART):
───────────────────────────────────────────────────────────────────

┌─────────────────────────────────────────────────────────────────┐
│                        YOUNG GENERATION                         │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                     Eden Space                           │   │
│  │         (new objects allocated here)                     │   │
│  │  [A][B][C][D][E][ ][ ][ ][ ][ ]                         │   │
│  └──────────────────────────────────────────────────────────┘   │
│                         │                                       │
│                         ▼ (survive GC)                          │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                   Survivor Space                         │   │
│  │      (objects that survived minor GC)                    │   │
│  │  [A][C][E][ ][ ][ ][ ][ ][ ][ ]                         │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ (survive multiple GCs)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                       OLD GENERATION                            │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                    Tenured Space                         │   │
│  │         (long-lived objects, GC less often)              │   │
│  │  [A][C][E][G][I][K][M][O][Q][S]                         │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ (still alive)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                       LARGE OBJECT SPACE                        │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │              (bitmaps, large arrays, > 12KB)            │   │
│  │  [Bitmap][LargeArray][BigBuffer]                         │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘

GENERATION SIZES (typical):
───────────────────────────────────────────────────────────────────
• Young Gen: 1/3 of heap (default)
  - Eden: ~90% of young gen
  - Survivor: ~10% of young gen (two equal spaces)
• Old Gen: 2/3 of heap
• Large Object Space: separate region
```

### Garbage Collection Types

```
ANDROID GC TYPES COMPARED
═══════════════════════════════════════════════════════════════════

GC TYPE          When Happens              Impact    Focus
───────────────────────────────────────────────────────────────────
Minor GC         Eden full (frequent)      Low       Young generation
Major GC         Old gen full (rare)       High      Full heap
Concurrent GC    Background (continuous)   Medium    Incremental
Allocation GC    Failed allocation         High      Emergency

MINOR GC (Scavenge):
───────────────────────────────────────────────────────────────────
Before:                           After:
┌─────────────────────────┐       ┌─────────────────────────┐
│ Eden: [A][B][C][D][E]  │       │ Eden: [ ][ ][ ][ ][ ]   │
│ S0:   [ ][ ][ ][ ][ ]  │       │ S0:   [A][C][E][ ][ ]   │
│ S1:   [ ][ ][ ][ ][ ]  │       │ S1:   [ ][ ][ ][ ][ ]   │
└─────────────────────────┘       └─────────────────────────┘
         │                                      │
         └─ (A, C, E survive) ──────────────────┘

MAJOR GC (Full GC):
───────────────────────────────────────────────────────────────────
• Stops all application threads (STW - Stop The World)
• Marks all reachable objects from roots
• Sweeps unreachable objects
• May compact heap (optional)
• Can cause visible jank (>10ms)
• Avoid frequent full GCs!

CONCURRENT GC (CMS - Concurrent Mark Sweep):
───────────────────────────────────────────────────────────────────
Application runs → Mark phase (concurrent) → Application runs → Sweep
• Minimal pauses (most work concurrent)
• Used during background
• Now replaced by Concurrent Copying (CC) in newer Android
```

## JNI/NDK Memory Management

### Native vs Managed Memory

```
JNI MEMORY MANAGEMENT DUALITY
═══════════════════════════════════════════════════════════════════

JAVA/KOTLIN (Managed)              NATIVE/C++ (Unmanaged)
───────────────────────────────────────────────────────────────────

String str = new String("Hello");  char* str = malloc(6);
       │                                   │
       ▼                                   ▼
┌─────────────┐                    ┌─────────────┐
│   GC Heap   │                    │ Native Heap │
│  (managed)  │                    │ (unmanaged) │
└─────────────┘                    └─────────────┘
       │                                   │
       │                         Must call free()!
       └─ GC auto frees ─────┘  
       
CRITICAL RULE:
───────────────────────────────────────────────────────────────────
Managed memory → GC collects automatically
Native memory → YOU MUST FREE MANUALLY!

JNI MEMORY TYPES:
───────────────────────────────────────────────────────────────────
┌─────────────────────────────────────────────────────────────────┐
│  1. Local References                                           │
│     • Created in JNI function                                  │
│     • Automatically freed when function returns                │
│     • Limited capacity (512 default)                           │
│                                                                 │
│  2. Global References                                          │
│     • Explicitly created (NewGlobalRef)                        │
│     • Must be deleted (DeleteGlobalRef)                        │
│     • Survive function calls                                   │
│                                                                 │
│  3. Weak Global References                                     │
│     • NewWeakGlobalRef / DeleteWeakGlobalRef                   │
│     • Can be GC'd if no other references                       │
│                                                                 │
│  4. Native Allocations                                         │
│     • malloc/new (C++ heap)                                    │
│     • Must be freed manually                                   │
│     • Not tracked by GC                                        │
└─────────────────────────────────────────────────────────────────┘
```

### JNI Memory Leak Patterns

```
COMMON JNI LEAK PATTERNS
═══════════════════════════════════════════════════════════════════

PATTERN 1: Forgetting DeleteGlobalRef
───────────────────────────────────────────────────────────────────
// BAD - Leak!
jclass cls = env->FindClass("com/example/MyClass");
jobject obj = env->NewGlobalRef(cls);
// ... use obj
// MISSING: env->DeleteGlobalRef(obj); ← Memory leak!

// GOOD
jobject obj = env->NewGlobalRef(cls);
// ... use
env->DeleteGlobalRef(obj);  ✅

PATTERN 2: Native memory not freed
───────────────────────────────────────────────────────────────────
// BAD
JNIEXPORT void JNICALL Java_MyClass_process(JNIEnv* env, jobject thiz) {
    char* buffer = (char*)malloc(1024);  // Allocated
    // ... process
    // MISSING: free(buffer); ← Native leak!
}

// GOOD
JNIEXPORT void JNICALL Java_MyClass_process(JNIEnv* env, jobject thiz) {
    char* buffer = (char*)malloc(1024);
    // ... process
    free(buffer);  ✅
}

PATTERN 3: Too many local references (overflow)
───────────────────────────────────────────────────────────────────
// BAD - May overflow local ref table
for (int i = 0; i < 10000; i++) {
    jstring str = env->NewStringUTF("test");  // Each call creates new ref
    // Not deleted - local table fills up!
}

// GOOD - Delete or ensure capacity
for (int i = 0; i < 10000; i++) {
    jstring str = env->NewStringUTF("test");
    // ... use
    env->DeleteLocalRef(str);  ✅
    
    // Or ensure capacity:
    env->EnsureLocalCapacity(10000);
}
```

### Direct ByteBuffer (Shared Memory)

```
DIRECT BYTEBUFFER SHARING
═══════════════════════════════════════════════════════════════════

Java/Native zero-copy buffer sharing:

JAVA SIDE:
───────────────────────────────────────────────────────────────────
// Allocate direct buffer (native memory, not GC heap)
ByteBuffer buffer = ByteBuffer.allocateDirect(1024);

// Fill with data
buffer.putInt(42);
buffer.put("Hello".getBytes());

// Get native address (for JNI)
// Returns direct pointer to memory
long address = ((DirectBuffer) buffer).address();

NATIVE SIDE:
───────────────────────────────────────────────────────────────────
JNIEXPORT void JNICALL Java_MyClass_processBuffer(
    JNIEnv* env, jobject thiz, jobject buffer) {
    
    // Get direct buffer address
    void* native_ptr = env->GetDirectBufferAddress(buffer);
    jlong capacity = env->GetDirectBufferCapacity(buffer);
    
    // Directly access! No copying!
    int* data = (int*)native_ptr;
    data[0] = 100;  // Modifies Java view too!
}

MEMORY LAYOUT:
═══════════════════════════════════════════════════════════════════
┌─────────────────────────────────────────────────────────────────┐
│  Java Heap (managed)          │  Native Heap (direct buffer)   │
│  ┌─────────────────────────┐  │  ┌─────────────────────────┐   │
│  │ ByteBuffer object       │  │  │ Actual data (1024 bytes)│   │
│  │ - address -------------─┼──┼─→│ [int][bytes][...]       │   │
│  │ - capacity: 1024        │  │  │                         │   │
│  │ - limit: 1024           │  │  └─────────────────────────┘   │
│  │ - position: 0           │  │                    ↑            │
│  └─────────────────────────┘  │                    │            │
└────────────────────────────────┼────────────────────┼────────────┘
                                 │                    │
                            Java access           Native access
                            via get/put           via pointer
```

## Binder Memory Management

### Binder IPC Architecture

```
BINDER IPC MEMORY FLOW
═══════════════════════════════════════════════════════════════════

PROCESS A (Client)                    PROCESS B (Server)
────────────────────                  ────────────────────

┌─────────────────────┐               ┌─────────────────────┐
│  Java/Kotlin Code   │               │  Java/Kotlin Code   │
│        │            │               │         ▲           │
│        ▼            │               │         │           │
│  Parcel (virtual)   │               │  Parcel (virtual)   │
│  ┌────────────────┐ │               │  ┌────────────────┐ │
│  │ marshall data  │ │               │  │ unmarshall     │ │
│  └────────────────┘ │               │  │ data           │ │
└─────────┬───────────┘               │  └────────────────┘ │
          │                           │         ▲           │
          ▼                           │         │           │
┌─────────────────────────────────────────────────────────────┐
│                    Binder Driver (Kernel)                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Binder Transaction Buffer (shared mapping)          │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │ [Header][Data from Process A][Objects][Reply]  │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
          ▲                           │
          │                           ▼
┌─────────┴───────────┐               ┌─────────────────────┐
│  Native Code        │               │  Native Code        │
│  (via ioctl)        │               │  (via mmap)         │
└─────────────────────┘               └─────────────────────┘

KEY FEATURES:
═══════════════════════════════════════════════════════════════════
• Single copy IPC (kernel copies once)
• Parcel data serialized once
• File descriptors can be passed
• Shared memory zero-copy for large data
```

### Binder Buffer Management

```
BINDER BUFFER POOL
═══════════════════════════════════════════════════════════════════

PER-PROCESS BINDER BUFFERS:
───────────────────────────────────────────────────────────────────

PROCESS SPACE:
┌─────────────────────────────────────────────────────────────────┐
│  Binder Buffer Pool (mmap'd memory)                            │
│  ┌──────────┬──────────┬──────────┬──────────┬──────────┐     │
│  │ Buffer 0 │ Buffer 1 │ Buffer 2 │ Buffer 3 │ Buffer 4 │ ... │
│  │ 1MB      │ FREE     │ 512KB    │ FREE     │ 256KB    │     │
│  │ in use   │          │ in use   │          │ in use   │     │
│  └──────────┴──────────┴──────────┴──────────┴──────────┘     │
│       ▲                   ▲                   ▲                │
│       │                   │                   │                │
│   Transaction           Transaction         Transaction         │
│   #123                 #125                 #128               │
└─────────────────────────────────────────────────────────────────┘

BUFFER ALLOCATION:
───────────────────────────────────────────────────────────────────
1. Transaction needs 8KB (small)
   → Takes next free buffer (Buffer 1 in example)

2. Transaction needs 2MB (too large for free buffers)
   → Returns error (-ENOMEM)

3. Transaction #123 completes
   → Frees Buffer 0
   → Coalesces with adjacent free buffers

BINDER MEMORY LIMITS:
───────────────────────────────────────────────────────────────────
• Max size: /proc/sys/net/unix/max_dgram_qlen
• Per-process limit: binder_buffer_size (usually 1-4MB)
• System-wide limit: binder_proc->max_threads
```

## Ashmem (Anonymous Shared Memory)

### Ashmem Architecture

```
ASHMEM SHARING MECHANISM
═══════════════════════════════════════════════════════════════════

PROCESS A                         PROCESS B
───────────────────────────────────────────────────────────────────

┌─────────────────────┐           ┌─────────────────────┐
│  Open Ashmem region │           │  Open by FD         │
│  ashmem_create_region│          │  (received via      │
│  ↓                  │           │   binder)           │
│  File descriptor    │           │  ↓                  │
│  (fd)               │           │  mmap() region      │
│  ↓                  │           │  ↓                  │
│  mmap() region      │           │  Shared memory      │
│  ↓                  │           │  visible!           │
│  Write data         │           │  Read data          │
└─────────────────────┘           └─────────────────────┘
         │                                   │
         └─────────────┬─────────────────────┘
                       │
                       ▼
         ┌─────────────────────────────┐
         │      KERNEL (Ashmem)        │
         │  ┌────────────────────────┐ │
         │  │      Page Cache        │ │
         │  │  [Shared Memory Pages] │ │
         │  └────────────────────────┘ │
         │                             │
         │  Features:                  │
         │  • Reference counted        │
         │  • Can be pinned            │
         │  • Provides name for debug  │
         │  • Supports protection      │
         └─────────────────────────────┘

ASHMEM ADVANTAGES:
═══════════════════════════════════════════════════════════════════
✓ True shared memory between processes
✓ Can be larger than binder buffers
✓ Zero-copy (once mapped)
✓ Can be named (debuggable)
✓ Reference counted (auto cleanup)
✓ Supports partially unmapping
```

### Ashmem Usage Example

```
ASHMEM IMPLEMENTATION EXAMPLE
═══════════════════════════════════════════════════════════════════

JAVA/KOTLIN (MemoryFile):
───────────────────────────────────────────────────────────────────
// Create shared memory region
MemoryFile sharedMem = new MemoryFile("my_shared", 1024 * 1024); // 1MB

// Write data
byte[] data = "Hello from Process A".getBytes();
sharedMem.writeBytes(data, 0, 0, data.length);

// Get file descriptor to pass via binder
FileDescriptor fd = sharedMem.getFileDescriptor();

// In another process (receiving FD):
ParcelFileDescriptor pfd = ParcelFileDescriptor.dup(receivedFd);
MemoryFile receivedMem = new MemoryFile(pfd);
byte[] buffer = new byte[1024];
receivedMem.readBytes(buffer, 0, 0, buffer.length);

NATIVE (C++):
───────────────────────────────────────────────────────────────────
#include <linux/ashmem.h>

int create_ashmem_region(const char* name, size_t size) {
    int fd = ashmem_create_region(name, size);
    if (fd < 0) return -1;
    
    // Set protection (read/write)
    if (ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE) < 0) {
        close(fd);
        return -1;
    }
    
    // Map into process space
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        return -1;
    }
    
    return fd;  // Can pass via binder
}

// Use mapped memory directly
void* shared_ptr = ptr;
strcpy((char*)shared_ptr, "Shared data");

// Cleanup
munmap(ptr, size);
close(fd);
```

## Memory Monitoring and Tools

### Android Memory Profiling

```
MEMORY MONITORING TOOLS
═══════════════════════════════════════════════════════════════════

ANDROID STUDIO PROFILER:
───────────────────────────────────────────────────────────────────
┌─────────────────────────────────────────────────────────────────┐
│  Memory Profiler                                               │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Memory Usage (MB)                                       │  │
│  │  100 ┤                           ┌───┐                   │  │
│  │   80 ┤                     ┌─────┘   └─────┐             │  │
│  │   60 ┤           ┌─────────┘               └───┐         │  │
│  │   40 ┤     ┌─────┘                             └───┐     │  │
│  │   20 ┤─────┘                                       └───┐ │  │
│  │    0 └──────────────────────────────────────────────────┘  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                  │
│  Live display of:                                              │
│  • Java Heap (green)                                           │
│  • Native Heap (blue)                                          │
│  • Graphics (red)                                              │
│  • Stack (purple)                                              │
│  • Code (yellow)                                               │
└─────────────────────────────────────────────────────────────────┘

ADB COMMANDS:
───────────────────────────────────────────────────────────────────
# Dump heap info
adb shell dumpsys meminfo <package>

# Force GC
adb shell run-as <package> kill -10 <pid>

# Track native allocations
adb shell setprop libc.debug.malloc 1

# Show memory maps
adb shell cat /proc/<pid>/maps

DDMS/Android Debug Monitor:
───────────────────────────────────────────────────────────────────
• Heap dump (HPROF file)
• Allocation tracking
• GC triggering
• Memory analysis (MAT, LeakCanary)
```

### Interpreting dumpsys meminfo

```
DUMPSYS MEMINFO OUTPUT EXPLAINED
═══════════════════════════════════════════════════════════════════

$ adb shell dumpsys meminfo com.example.app

OUTPUT:
───────────────────────────────────────────────────────────────────
Applications Memory Usage (kB):
Uptime: 1234567 Realtime: 1234567

** MEMINFO in pid 12345 [com.example.app] **
                   Pss  Private  Private  SwapPss     Heap     Heap
                 Total    Dirty    Clean    Dirty     Size    Alloc     Free
                ------   ------   ------   ------   ------   ------   ------
  Native Heap    12345    12000        0        0    20000    15000     5000
  Dalvik Heap     5678     5000        0        0     8000     6000     2000
        Stack      123      123        0        0
       Ashmem       12        0       12        0
      Gfx dev     1024     1024        0        0
    Other dev        4        0        4        0
     .so mmap      456      0       456        0
    .apk mmap       89      0        89        0
    .dex mmap     3456      0      3456        0
    Other mmap      45       4        0        0
      Unknown      345      345        0        0
        TOTAL    23456    18496     4017        0    28000    21000     7000

INTERPRETATION:
───────────────────────────────────────────────────────────────────
• Native Heap: C/C++ allocations (malloc/new)
• Dalvik Heap: Java/Kotlin objects (GC managed)
• Pss (Proportional Set Size): Fair share of shared memory
• Private Dirty: Unique, non-swappable memory
• Heap Size: Total heap capacity
• Heap Alloc: Currently allocated in heap
• Heap Free: Available in heap

HEALTH INDICATORS:
───────────────────────────────────────────────────────────────────
✅ Healthy: Free > 10% of heap size
⚠️ Warning: Free < 5% of heap size
❌ Critical: Frequent GCs, allocation failures
```

### Leak Detection Tools

```
ANDROID MEMORY LEAK DETECTION
═══════════════════════════════════════════════════════════════════

LEAKCANARY (Java/Kotlin):
───────────────────────────────────────────────────────────────────
// Add to build.gradle
dependencies {
    debugImplementation 'com.squareup.leakcanary:leakcanary-android:2.x'
}

// Automatic detection in debug builds
// Shows leak traces with heap dumps

SAMPLE LEAK REPORT:
───────────────────────────────────────────────────────────────────
┬───
│ GC Root: Local variable in native code
│
├─ com.example.MyActivity instance
│    Leaking: YES (ObjectWatcher was watching this)
│    ↓ MyActivity.mListener
│
├─ com.example.MyListener instance
│    Leaking: UNKNOWN
│    ↓ MyListener.mCallback
│
╰→ com.example.MyActivity instance (same as above)

NATIVE MEMORY LEAK DETECTION:
───────────────────────────────────────────────────────────────────
// Using Valgrind (emulator)
valgrind --leak-check=full --log-file=valgrind.log ./app_process

// Using heaptrack
heaptrack ./my_native_binary

// Using simple tracing
#define malloc(x) malloc_trace(x, __FILE__, __LINE__)
#define free(x) free_trace(x, __FILE__, __LINE__)
```

## Best Practices

### Memory Optimization Guidelines

```
ANDROID MEMORY BEST PRACTICES
═══════════════════════════════════════════════════════════════════

DO'S ✅:
───────────────────────────────────────────────────────────────────
• Use Android's memory profiler regularly
• Release references in onPause()/onDestroy()
• Use WeakReference for caches/listeners
• Reuse objects with Object Pools
• Use try-with-resources for AutoCloseable
• Optimize bitmaps (use inSampleSize)
• Monitor memory with Debug.getNativeHeapAllocatedSize()
• Use android:largeHeap only when necessary
• Profile with dumpsys meminfo

DON'TS ❌:
───────────────────────────────────────────────────────────────────
• Don't hold Activity references in static fields
• Don't forget to unregister listeners
• Don't use finalize() (slow, unpredictable)
• Don't ignore onTrimMemory() callbacks
• Don't keep large data in memory unnecessarily
• Don't create objects in tight loops
• Don't ignore JNI local reference limits
• Don't hold onto Context after Activity dies

JNI SPECIFIC:
───────────────────────────────────────────────────────────────────
✅ DeleteLocalRef when done
✅ Use GetStringUTFChars with ReleaseStringUTFChars
✅ Use GlobalRef carefully and delete
✅ Check for exceptions after JNI calls
✅ Use EnsureLocalCapacity for many references

BITMAP OPTIMIZATION:
───────────────────────────────────────────────────────────────────
// Bad
BitmapFactory.decodeResource(res, R.drawable.large);  // Full size!

// Good
BitmapFactory.Options options = new BitmapFactory.Options();
options.inJustDecodeBounds = true;
BitmapFactory.decodeResource(res, R.id.image, options);

// Calculate sample size
options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight);
options.inJustDecodeBounds = false;
Bitmap bitmap = BitmapFactory.decodeResource(res, R.id.image, options);
```

### onTrimMemory() Callbacks

```
ONTRIMEMORY LEVELS
═══════════════════════════════════════════════════════════════════

IMPLEMENTATION:
───────────────────────────────────────────────────────────────────
@Override
public void onTrimMemory(int level) {
    switch (level) {
        case TRIM_MEMORY_RUNNING_MODERATE:
            // App running, moderate memory pressure
            // Clear caches, non-essential data
            break;
            
        case TRIM_MEMORY_RUNNING_LOW:
            // App running, low memory pressure
            // Clear larger caches, bitmaps
            break;
            
        case TRIM_MEMORY_RUNNING_CRITICAL:
            // App running, critical memory
            // Release everything non-essential
            break;
            
        case TRIM_MEMORY_UI_HIDDEN:
            // App going to background
            // Release UI resources, bitmaps
            break;
            
        case TRIM_MEMORY_BACKGROUND:
            // App in background, device low memory
            // Release all caches
            break;
    }
}

MEMORY LEVELS HIERARCHY:
───────────────────────────────────────────────────────────────────
CRITICAL (Lowest memory)
    ↑
TRIM_MEMORY_RUNNING_CRITICAL
    ↑
TRIM_MEMORY_RUNNING_LOW
    ↑
TRIM_MEMORY_RUNNING_MODERATE
    ↑
TRIM_MEMORY_UI_HIDDEN
    ↑
TRIM_MEMORY_BACKGROUND
    ↑
TRIM_MEMORY_COMPLETE
    ↑
No pressure
```

## Memory Pressure Handling

```
LOW MEMORY KILLER (LMK)
═══════════════════════════════════════════════════════════════════

ANDROID OEM ADAPTATION:
───────────────────────────────────────────────────────────────────
Each manufacturer customizes LMK levels:

System-wide memory thresholds:
┌─────────────────────────────────────────────────────────────────┐
│  Memory Level    Action                                        │
│  ─────────────────────────────────────────────────────────────  │
│  > 100MB free    Normal operation                              │
│   50-100MB free  Start killing cached processes                │
│   20-50MB free   Kill background services                      │
│   10-20MB free   Kill visible apps (last resort)               │
│   < 10MB free    Kernel panic / reboot                         │
└─────────────────────────────────────────────────────────────────┘

PROCESS ADJUSTMENT LEVELS:
───────────────────────────────────────────────────────────────────
Level          Priority        Example
───────────────────────────────────────────────────────────────────
0 (highest)    Native          Kernel, system_server
1              Persistent      Phone, Bluetooth
2              Foreground      Current activity
3              Visible         Visible but not foreground
4              Perceptible     Playing music
5              Service         Started service
6              Home            Launcher
7              Previous        Previous app
8-15           Cached          Background apps

LMK Decision:
───────────────────────────────────────────────────────────────────
• Kills processes with lowest oom_adj first
• Considers RSS (memory usage)
• May kill more to free memory
• Processes can handle onLowMemory() callback
```

## Interview Questions & Answers

### Q1: What's the difference between Android's GC and Java GC?

```
ANSWER:
═══════════════════════════════════════════════════════════════════

Similarities:
- Both use generational garbage collection
- Both mark-and-sweep or mark-and-copy
- Both have stop-the-world pauses

Android ART differences:
1. Concurrent Copying (CC) collector (from Android 8+)
   - Compaction during GC (reduces fragmentation)
   - Shorter pause times (<2ms typical)

2. Memory footprint tuning
   - Designed for mobile (RAM constraints)
   - Aggressive compaction
   - Heap sizes: 128MB-512MB (vs 2GB+ on servers)

3. Low memory killer integration
   - GC decisions consider system pressure
   - Can trigger LMK if needed

4. Heap behavior differences:
   - ART uses Large Object Space (LOS) for bitmaps
   - Dalvik had different GC strategies
   - ART has generational GC with two survivors
```

### Q2: How do you detect memory leak in Android?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Tools & Techniques:

1. LeakCanary (automatic)
   dependencies { debugImplementation 'com.squareup.leakcanary:leakcanary-android:2.x' }
   // Automatic detection in debug builds

2. Android Studio Profiler
   - Capture heap dump
   - Use Analyzer Tasks
   - Look for references to destroyed Activity

3. Manual heap dump analysis
   adb shell am dumpheap <pid> /data/local/tmp/heap.hprof
   adb pull /data/local/tmp/heap.hprof
   # Open in MAT (Eclipse Memory Analyzer)

4. Monitor logs
   adb logcat | grep -i "leak\|memory"

5. Common leak patterns:
   - Static Context references
   - Inner classes (Handler, AsyncTask) holding Activity
   - Unregistered listeners
   - Singleton with Activity reference
   - WebView not destroyed

Prevention:
- Use WeakReference for callbacks
- Clear references in onDestroy()
- Use ViewModel (survives config changes)
- Application Context for singletons
```

### Q3: Explain Android's low memory killer.

```
ANSWER:
───────────────────────────────────────────────────────────────────

LMK (Low Memory Killer) is a kernel driver that kills processes to free memory.

How it works:
───────────────────────────────────────────────────────────────────
1. Kernel monitors free memory
2. When memory below threshold, activate LMK
3. LMK selects process based on oom_score_adj
4. Sends SIGKILL to selected process
5. Process removed, memory freed

Process priorities (oom_score_adj):
0: System processes (don't kill)
1: Persistent (phone, Bluetooth)
2: Foreground (current app)
3: Visible app
4: Perceptible (music playback)
5: Service started
6: Home app
7: Previous app
8-15: Background cached apps (kill first)

Configuration (in /sys/module/lowmemorykiller/parameters/):
minfree: 18432,23040,27648,32256,55296,80640 (in pages)
adj: 0,100,200,300,900,906

Application response:
- Implement onTrimMemory()
- Free caches when level > TRIM_MEMORY_BACKGROUND
- Don't rely on LMK for cleanup
```

### Q4: What's the difference between Ashmem and Binder?

```
ANSWER:
───────────────────────────────────────────────────────────────────

ASHMEM (Anonymous Shared Memory) vs BINDER
───────────────────────────────────────────────────────────────────

ASHMEM:
- Pure shared memory (like POSIX shm)
- Multiple processes map same physical pages
- Direct read/write access
- No serialization needed
- Best for: Large buffers, bitmaps, bulk data
- Example: SurfaceFlinger graphics buffers

BINDER:
- IPC mechanism (request-reply)
- Data serialized to Parcel
- Single copy (driver copies once)
- Supports RPC, object references, FDs
- Best for: Frequent small messages, RPC calls
- Example: ServiceManager, ActivityManager

Size limits:
- Ashmem: Limited by RAM configuration
- Binder: 1MB default (adjustable)

Use together:
- Ashmem for large data
- Binder to pass the file descriptor
- Zero-copy sharing!

When to use which:
- Small data (< 1MB) → Binder
- Large data (> 1MB) → Ashmem + Binder FD
- Streaming data → Socket + Ashmem
```

### Q5: How does JNI memory management work?

```
ANSWER:
───────────────────────────────────────────────────────────────────

JNI separates managed (Java) from unmanaged (Native) memory.

Managed memory (Java heap):
- Automatically garbage collected
- Cannot be directly accessed from native
- Access via JNI functions (GetXXXField, etc.)

Native memory:
- Allocated with malloc/new
- Must be manually freed
- Persists across JNI calls

JNI Reference Types:
┌─────────────────────────────────────────────────────────────────┐
│ 1. Local Reference                                             │
│    - Created in JNI function                                   │
│    - Auto-freed on return                                      │
│    - Limit: 512 per thread                                     │
│                                                                │
│ 2. Global Reference                                            │
│    - NewGlobalRef() / DeleteGlobalRef()                        │
│    - Survives function calls                                   │
│    - Must be deleted explicitly                                │
│                                                                │
│ 3. Weak Global Reference                                       │
│    - Doesn't prevent GC                                        │
│    - Check with IsSameObject()                                 │
│                                                                │
│ 4. Critical Reference (GetPrimitiveArrayCritical)              │
│    - Direct pointer to array                                   │
│    - Must release quickly                                      │
│    - Blocks GC!                                                │
└─────────────────────────────────────────────────────────────────┘

Common mistakes:
- Forgetting DeleteGlobalRef (native leak)
- Not checking for exceptions after JNI
- Holding global ref to Activity (prevents GC)
- Getting array elements without releasing
- Critical sections that are too long

Best practices:
- Always pair Get/Release
- Use PushLocalFrame for many refs
- Check exceptions after every JNI call
- Don't cache objects across threads
```

### Q6: What causes OutOfMemoryError in Android?

```
ANSWER:
───────────────────────────────────────────────────────────────────

Common causes:

1. Bitmaps (most common)
   - Large images decoded at full size
   - Not recycling bitmaps
   - Solution: Use inSampleSize, Glide/Coil

2. Activity leaks
   - Static references to Activity
   - Handler inner classes
   - Anonymous listeners
   - Solution: WeakReference, remove callbacks

3. Large data structures
   - Loading entire database
   - Caching everything
   - Solution: Pagination, Disk caching

4. Multiple large allocations
   - Constructor creating large objects
   - Creating arrays in loops
   - Solution: Reuse objects, allocate once

5. Fragmentation (especially on older Android)
   - Lots of small allocations
   - Large contiguous allocation fails
   - Solution: Pre-allocate, use pools

6. Memory limit per app
   - Varies by device (typically 256-512MB)
   - Large heap request: android:largeHeap
   - Solution: Be efficient, don't request large heap

Detection:
- Monitor with Memory Profiler
- Track allocation rate
- Heap dump analysis
- LeakCanary for leaks

Prevention:
- Use memory-efficient data structures (SparseArray vs HashMap)
- Release references in onPause/onDestroy
- Use ViewModel (handles configuration changes)
- Clear ImageView bitmaps earlier
```

### Q7: What's the difference between PSS, RSS, and USS?

```
ANSWER:
───────────────────────────────────────────────────────────────────

RSS (Resident Set Size):
───────────────────────────────────────────────────────────────────
- Total physical memory used by process
- Includes shared libraries (counted for each process)
- Overestimates usage for shared memory
- Example: 2 processes share 10MB library → RSS shows 20MB total

PSS (Proportional Set Size):
───────────────────────────────────────────────────────────────────
- Fair share of shared memory
- Shared memory divided by number of processes
- More accurate for memory accounting
- Example: 2 processes share 10MB library → 5MB each in PSS

USS (Unique Set Size):
───────────────────────────────────────────────────────────────────
- Memory unique to process (not shared)
- Memory freed if process dies
- Best for memory leak detection

Visual example:
┌─────────────────────────────────────────────────────────────────┐
│ Process A:                                                     │
│   Private: 30MB                                                │
│   Shared with B: 20MB (10MB in PSS)                           │
│   RSS = 50MB, PSS = 40MB, USS = 30MB                          │
│                                                                │
│ Process B:                                                     │
│   Private: 20MB                                                │
│   Shared with A: 20MB (10MB in PSS)                           │
│   RSS = 40MB, PSS = 30MB, USS = 20MB                          │
│                                                                │
│ System total:                                                  │
│   RSS total = 90MB (double-counts shared)                     │
│   PSS total = 70MB (actual physical usage)                    │
└─────────────────────────────────────────────────────────────────┘

When to use:
- RSS: Process comparison (approximate)
- PSS: Memory accounting (accurate)
- USS: Leak detection (private memory growth)
```

This comprehensive guide covers all Android memory management aspects with detailed architecture, practical examples, and interview-focused explanations perfect for Android/Linux system interviews!
