# File Systems in C - Complete Guide

## Table of Contents
1. [Definition](#1-definition)
2. [Types of File Systems](#2-types-of-file-systems)
3. [Basic Syntax & Functions](#3-basic-syntax--functions)
4. [Working Flow](#4-working-flow)
5. [Block Diagrams](#5-block-diagrams)
6. [How to Modify File Systems](#6-how-to-modify-file-systems)
7. [Complete Code Examples](#7-complete-code-examples)
8. [Error Handling](#8-error-handling)
9. [Advanced Operations](#9-advanced-operations)
10. [Best Practices](#10-best-practices)

---

## 1. Definition

A **File System** is a method and data structure that an operating system uses to control how data is stored, organized, and retrieved on a storage device. In C programming, file systems are accessed through standard I/O libraries that provide functions to create, read, write, and manipulate files.

### Key Concepts
- **File Descriptor**: Integer handle to access a file
- **Stream**: High-level interface for file operations
- **Inode**: Data structure storing file metadata
- **Directory**: Special file containing file names and their inode numbers

---

## 2. Types of File Systems

### 2.1 Based on Access Method

| Type | Description | Example Functions |
|------|-------------|-------------------|
| **Buffered I/O** | Uses memory buffer for efficiency | `fopen()`, `fread()`, `fwrite()` |
| **Unbuffered I/O** | Direct system calls | `open()`, `read()`, `write()` |
| **Memory Mapped** | Maps file to memory address | `mmap()`, `munmap()` |

### 2.2 Common File System Types

```
Linux/Unix:     ext2, ext3, ext4, XFS, Btrfs, ZFS
Windows:        NTFS, FAT32, exFAT
Embedded:       JFFS2, YAFFS, UBIFS, SquashFS
Network:        NFS, CIFS/SMB
```

---

## 3. Basic Syntax & Functions

### 3.1 Header Files Required

```c
#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Exit, memory allocation
#include <string.h>     // String operations
#include <fcntl.h>      // File control options
#include <unistd.h>     // POSIX system calls
#include <sys/stat.h>   // File status
#include <dirent.h>     // Directory operations
#include <errno.h>      // Error handling
```

### 3.2 Core Functions Cheat Sheet

| Operation | Buffered (stdio) | Unbuffered (POSIX) |
|-----------|-----------------|-------------------|
| **Open** | `fopen(path, mode)` | `open(path, flags, mode)` |
| **Read** | `fread(buffer, size, count, stream)` | `read(fd, buffer, count)` |
| **Write** | `fwrite(buffer, size, count, stream)` | `write(fd, buffer, count)` |
| **Close** | `fclose(stream)` | `close(fd)` |
| **Seek** | `fseek(stream, offset, whence)` | `lseek(fd, offset, whence)` |
| **Delete** | `remove(path)` | `unlink(path)` |

### 3.3 File Opening Modes

```c
// Text modes
"r"   // Read only (file must exist)
"w"   // Write only (creates/truncates)
"a"   // Append only (creates if needed)
"r+"  // Read/Write (file must exist)
"w+"  // Read/Write (creates/truncates)
"a+"  // Read/Append (creates if needed)

// Binary modes (append 'b')
"rb", "wb", "ab", "r+b", "w+b", "a+b"
```

---

## 4. Working Flow

### 4.1 Basic File Operation Flowchart

```
    START
      |
      v
+-------------+
|  Open File  |
|  (fopen)    |
+-------------+
      |
      v
+-------------+
|  Check if   |----No---->[Error Handling]
|  Successful |
+-------------+
      | Yes
      v
+-------------+
|  Perform    |
|  Operation  |
| (read/write)|
+-------------+
      |
      v
+-------------+
|  Check for  |----Yes---->[Retry/Handle]
|  Errors     |
+-------------+
      | No
      v
+-------------+
|  Close File |
|  (fclose)   |
+-------------+
      |
      v
     END
```

### 4.2 Detailed Read/Write Flow

```
WRITE OPERATION:                    READ OPERATION:
----------------                    ---------------
[User Buffer]                       [File on Disk]
      |                                    |
      v                                    v
[fwrite() call]                     [fopen() call]
      |                                    |
      v                                    v
[stdio buffer]                      [File stream opened]
      |                                    |
      v                                    v
[System call: write()]              [fread() call]
      |                                    |
      v                                    v
[Kernel buffer cache]               [Kernel reads from disk]
      |                                    |
      v                                    v
[Device driver]                     [Data to stdio buffer]
      |                                    |
      v                                    v
[Physical disk write]               [User buffer gets data]
```

### 4.3 Complete Working Example Flow

```c
// Step-by-step flow with code
#include <stdio.h>

int main() {
    // Step 1: Declare file pointer
    FILE *fp;
    
    // Step 2: Open file
    fp = fopen("example.txt", "w+");
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }
    
    // Step 3: Write data
    fprintf(fp, "Hello, File System!\n");
    fputs("Second line\n", fp);
    
    // Step 4: Reset position for reading
    rewind(fp);
    
    // Step 5: Read and display
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("Read: %s", buffer);
    }
    
    // Step 6: Close file
    fclose(fp);
    
    return 0;
}
```

---

## 5. Block Diagrams

### 5.1 Layered Architecture of File System

```
+--------------------------------------------------+
|                  User Application               |
+--------------------------------------------------+
|                  System Calls                   |
|    (open, read, write, close, lseek, stat)      |
+--------------------------------------------------+
|                Virtual File System (VFS)        |
|         (Common interface for all FS types)     |
+--------------------------------------------------+
|                                                   |
|   +-----------+  +-----------+  +-----------+    |
|   |   ext4    |  |   FAT32   |  |   NFS     |    |
|   |  Driver   |  |  Driver   |  |  Driver   |    |
|   +-----------+  +-----------+  +-----------+    |
|                                                   |
+--------------------------------------------------+
|                Block I/O Layer                   |
|         (Buffer cache, I/O scheduler)            |
+--------------------------------------------------+
|                Device Drivers                    |
|         (SATA, NVMe, USB, SD card)               |
+--------------------------------------------------+
|                  Physical Disk                   |
+--------------------------------------------------+
```

### 5.2 Inode Structure Diagram

```
+-------------------------------+
|           INODE               |
+-------------------------------+
| Mode (permissions)            |
| Owner UID/GID                 |
| File size                     |
| Timestamps:                   |
|   - Access time               |
|   - Modify time              |
|   - Change time              |
| Link count                    |
| Block pointers:               |
|   +------------------------+  |
|   | Direct blocks (12)     |--+--> [Data Block]
|   +------------------------+  |
|   | Single indirect        |--+--> [Indirect Block] --> [Data Blocks]
|   +------------------------+  |
|   | Double indirect        |--+--> [Double Indirect] --> [Indirect] --> [Data]
|   +------------------------+  |
|   | Triple indirect        |--+--> [Triple Indirect] --> ... --> [Data]
|   +------------------------+  |
+-------------------------------+
```

### 5.3 Directory Structure and File Storage

```
Directory Entry Layout:
+===========+============+============+============+
| File Name | Inode Num  | File Type  |  Size      |
+===========+============+============+============+
| "file1"   | 1024       | Regular    | 4096       |
+-----------+------------+------------+------------+
| "file2"   | 2048       | Directory  | 1024       |
+-----------+------------+------------+------------+
| "link1"   | 1024       | Symlink    | 10         |
+-----------+------------+------------+------------+

Physical Layout on Disk:
+----------+----------+----------+----------+
| Super    | Inode    | Data     | Inode    |
| Block    | Table    | Blocks   | Bitmap   |
+----------+----------+----------+----------+
```

---

## 6. How to Modify File Systems

### 6.1 Modifying File Content

#### Method 1: In-place Modification
```c
void modify_file_inplace(const char *filename, long position, 
                         const char *new_data, size_t data_size) {
    FILE *fp = fopen(filename, "r+b");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    // Seek to desired position
    fseek(fp, position, SEEK_SET);
    
    // Write new data
    fwrite(new_data, 1, data_size, fp);
    
    fclose(fp);
}
```

#### Method 2: Copy-Modify-Rename (Safer)
```c
int modify_file_safe(const char *filename, modify_callback callback) {
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", filename);
    
    FILE *src = fopen(filename, "r");
    FILE *dst = fopen(temp_file, "w");
    
    if (!src || !dst) {
        perror("fopen");
        return -1;
    }
    
    // Apply modifications through callback
    callback(src, dst);
    
    fclose(src);
    fclose(dst);
    
    // Replace original with modified version
    remove(filename);
    rename(temp_file, filename);
    
    return 0;
}
```

### 6.2 Modifying File Metadata

```c
#include <sys/stat.h>
#include <utime.h>

// Change file permissions
void modify_permissions(const char *filename, mode_t new_perms) {
    if (chmod(filename, new_perms) == -1) {
        perror("chmod");
    }
}

// Change file ownership
void modify_owner(const char *filename, uid_t owner, gid_t group) {
    if (chown(filename, owner, group) == -1) {
        perror("chown");
    }
}

// Modify timestamps
void modify_timestamps(const char *filename, time_t access_time, 
                       time_t mod_time) {
    struct utimbuf times = {access_time, mod_time};
    if (utime(filename, &times) == -1) {
        perror("utime");
    }
}
```

### 6.3 Extending File Size

```c
void extend_file(const char *filename, off_t new_size) {
    int fd = open(filename, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return;
    }
    
    // Extend file size (creates holes on Unix)
    if (ftruncate(fd, new_size) == -1) {
        perror("ftruncate");
    }
    
    close(fd);
}

// Fill extended space with zeros
void extend_and_zero_fill(const char *filename, off_t current_size, 
                          off_t new_size) {
    FILE *fp = fopen(filename, "r+b");
    if (!fp) return;
    
    fseek(fp, 0, SEEK_END);
    
    // Write zeros to fill the extended space
    char zero = 0;
    for (off_t i = current_size; i < new_size; i++) {
        fwrite(&zero, 1, 1, fp);
    }
    
    fclose(fp);
}
```

### 6.4 Implementing Custom File System Operations

```c
// Custom file system structure
typedef struct {
    char name[256];
    mode_t permissions;
    off_t size;
    time_t created;
    time_t modified;
    void *data_blocks;
} custom_file_t;

// Custom open operation
custom_file_t* custom_open(const char *path, const char *mode) {
    custom_file_t *file = malloc(sizeof(custom_file_t));
    // Implementation specific logic
    return file;
}

// Custom read operation
size_t custom_read(custom_file_t *file, void *buffer, size_t size) {
    // Custom read logic
    return 0;
}

// Custom write operation
size_t custom_write(custom_file_t *file, const void *buffer, size_t size) {
    // Custom write logic with modification tracking
    return 0;
}
```

---

## 7. Complete Code Examples

### 7.1 Simple File Copy Utility

```c
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int copy_file(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    if (!src) {
        fprintf(stderr, "Cannot open source file: %s\n", source);
        return -1;
    }
    
    FILE *dst = fopen(destination, "wb");
    if (!dst) {
        fprintf(stderr, "Cannot create destination file: %s\n", destination);
        fclose(src);
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0) {
        fwrite(buffer, 1, bytes_read, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    printf("File copied successfully: %s -> %s\n", source, destination);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }
    
    return copy_file(argv[1], argv[2]);
}
```

### 7.2 Directory Traversal and File Search

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void search_files(const char *dir_path, const char *pattern) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return;
    }
    
    struct dirent *entry;
    char full_path[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directory
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", 
                 dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively search subdirectory
                search_files(full_path, pattern);
            } else if (S_ISREG(st.st_mode)) {
                // Check if filename contains pattern
                if (strstr(entry->d_name, pattern)) {
                    printf("Found: %s (size: %ld bytes)\n", 
                           full_path, st.st_size);
                }
            }
        }
    }
    
    closedir(dir);
}

int main() {
    search_files("/home/user/documents", ".txt");
    return 0;
}
```

### 7.3 Real-time File Monitor

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

typedef struct {
    char filename[256];
    time_t last_modify;
    off_t last_size;
} file_monitor_t;

void monitor_init(file_monitor_t *monitor, const char *filename) {
    strcpy(monitor->filename, filename);
    struct stat st;
    if (stat(filename, &st) == 0) {
        monitor->last_modify = st.st_mtime;
        monitor->last_size = st.st_size;
    }
}

int check_changes(file_monitor_t *monitor) {
    struct stat st;
    if (stat(monitor->filename, &st) != 0) {
        printf("File no longer exists!\n");
        return -1;
    }
    
    int changed = 0;
    
    if (st.st_mtime != monitor->last_modify) {
        printf("File modified at %s", ctime(&st.st_mtime));
        monitor->last_modify = st.st_mtime;
        changed = 1;
    }
    
    if (st.st_size != monitor->last_size) {
        printf("Size changed: %ld -> %ld bytes\n", 
               monitor->last_size, st.st_size);
        monitor->last_size = st.st_size;
        changed = 1;
    }
    
    return changed;
}

int main() {
    file_monitor_t monitor;
    monitor_init(&monitor, "important.dat");
    
    printf("Monitoring file: %s\n", monitor.filename);
    
    while (1) {
        if (check_changes(&monitor)) {
            printf("Change detected!\n");
        }
        sleep(1);
    }
    
    return 0;
}
```

---

## 8. Error Handling

### 8.1 Common Errors and Handling

```c
#include <errno.h>
#include <string.h>

void safe_file_operation(const char *filename) {
    FILE *fp = fopen(filename, "r");
    
    if (fp == NULL) {
        switch (errno) {
            case ENOENT:
                fprintf(stderr, "Error: File '%s' does not exist\n", 
                        filename);
                break;
            case EACCES:
                fprintf(stderr, "Error: Permission denied for '%s'\n", 
                        filename);
                break;
            case EMFILE:
                fprintf(stderr, "Error: Too many open files\n");
                break;
            default:
                fprintf(stderr, "Error opening file: %s\n", 
                        strerror(errno));
        }
        return;
    }
    
    // Proceed with file operations
    fclose(fp);
}
```

### 8.2 Robust Read with Retry

```c
#define MAX_RETRIES 3
#define RETRY_DELAY_MS 100

ssize_t robust_read(int fd, void *buffer, size_t count) {
    ssize_t bytes_read = 0;
    int retries = 0;
    char *buf = (char *)buffer;
    
    while (bytes_read < count && retries < MAX_RETRIES) {
        ssize_t result = read(fd, buf + bytes_read, count - bytes_read);
        
        if (result == -1) {
            if (errno == EINTR) {
                // Interrupted, retry immediately
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Resource temporarily unavailable
                usleep(RETRY_DELAY_MS * 1000);
                retries++;
                continue;
            } else {
                perror("read");
                return -1;
            }
        } else if (result == 0) {
            // EOF
            break;
        }
        
        bytes_read += result;
        retries = 0; // Reset retries on successful read
    }
    
    return bytes_read;
}
```

---

## 9. Advanced Operations

### 9.1 Memory-Mapped File I/O

```c
#include <sys/mman.h>

void memory_mapped_example(const char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return;
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return;
    }
    
    // Map file to memory
    char *mapped = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }
    
    // Modify file through memory
    sprintf(mapped, "Modified at %ld", time(NULL));
    
    // Synchronize changes
    msync(mapped, st.st_size, MS_SYNC);
    
    // Cleanup
    munmap(mapped, st.st_size);
    close(fd);
}
```

### 9.2 Locking for Concurrent Access

```c
#include <fcntl.h>

void lock_file(int fd, short type) {
    struct flock fl = {
        .l_type = type,     // F_RDLCK, F_WRLCK, F_UNLCK
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,         // Lock entire file
        .l_pid = getpid()
    };
    
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("fcntl lock");
    }
}

void concurrent_write_example(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return;
    }
    
    // Acquire exclusive lock
    lock_file(fd, F_WRLCK);
    
    // Critical section
    write(fd, "Protected data\n", 15);
    
    // Release lock
    lock_file(fd, F_UNLCK);
    
    close(fd);
}
```

### 9.3 File System Statistics

```c
#include <sys/statvfs.h>

void get_filesystem_stats(const char *path) {
    struct statvfs stat;
    
    if (statvfs(path, &stat) == -1) {
        perror("statvfs");
        return;
    }
    
    printf("File System Statistics for %s:\n", path);
    printf("  Block size: %lu bytes\n", stat.f_bsize);
    printf("  Total blocks: %lu\n", stat.f_blocks);
    printf("  Free blocks: %lu\n", stat.f_bfree);
    printf("  Available blocks: %lu\n", stat.f_bavail);
    printf("  Total inodes: %lu\n", stat.f_files);
    printf("  Free inodes: %lu\n", stat.f_ffree);
    
    unsigned long long total_space = stat.f_blocks * stat.f_bsize;
    unsigned long long free_space = stat.f_bavail * stat.f_bsize;
    unsigned long long used_space = total_space - free_space;
    
    printf("  Total space: %.2f GB\n", total_space / (1024.0 * 1024 * 1024));
    printf("  Used space: %.2f GB\n", used_space / (1024.0 * 1024 * 1024));
    printf("  Free space: %.2f GB\n", free_space / (1024.0 * 1024 * 1024));
    printf("  Usage: %.1f%%\n", (used_space * 100.0) / total_space);
}
```

---

## 10. Best Practices

### 10.1 Coding Guidelines

```c
// DO's and DON'Ts

// ✅ DO: Always check return values
FILE *fp = fopen("file.txt", "r");
if (fp == NULL) {
    handle_error();
}

// ❌ DON'T: Ignore return values
fopen("file.txt", "r");  // Bad! No error check

// ✅ DO: Use meaningful buffer sizes
#define BUFFER_SIZE 8192  // Good for most file operations
char buffer[BUFFER_SIZE];

// ❌ DON'T: Hardcode small buffers
char buffer[64];  // Too small for efficient I/O

// ✅ DO: Close files in reverse order of opening
FILE *fp1 = fopen("a.txt", "r");
FILE *fp2 = fopen("b.txt", "r");
// ... operations ...
fclose(fp2);
fclose(fp1);

// ✅ DO: Use temporary files for atomic updates
// Create temp, write, then rename
snprintf(temp_path, sizeof(temp_path), "%s.tmp.%d", 
         original_path, getpid());
```

### 10.2 Performance Optimization Checklist

| Technique | Benefit | Implementation |
|-----------|---------|----------------|
| **Buffering** | Reduces syscalls | `setvbuf(fp, NULL, _IOFBF, 65536)` |
| **Block I/O** | Faster large reads | Use 4K-64K buffers |
| **Memory mapping** | Zero-copy access | `mmap()` for random access |
| **Async I/O** | Non-blocking | `aio_read()` / `aio_write()` |
| **Direct I/O** | Bypass cache | `O_DIRECT` flag (Linux) |

### 10.3 Security Best Practices

```c
// Secure file creation
void secure_create(const char *filename) {
    // Create with restrictive permissions
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd == -1) {
        perror("secure_create");
        return;
    }
    
    // Ensure umask doesn't weaken permissions
    fchmod(fd, 0600);
    
    close(fd);
}

// Avoid TOCTOU (Time-of-check Time-of-use) bugs
// ❌ BAD:
if (access("file.txt", R_OK) == 0) {
    fopen("file.txt", "r");  // Race condition!
}

// ✅ GOOD:
FILE *fp = fopen("file.txt", "r");
if (fp == NULL) {
    // Handle error directly
}
```

---

## 11. Quick Reference Card

```c
// Common File Operations Quick Reference

// Open and read entire file
char* read_entire_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    
    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    
    fclose(fp);
    return buffer;
}

// Write string to file
int write_string(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;
    
    int result = fprintf(fp, "%s", content);
    fclose(fp);
    return result;
}

// Check if file exists
int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

// Get file size
off_t file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size;
}

// Append to file
int append_to_file(const char *path, const char *data) {
    FILE *fp = fopen(path, "a");
    if (!fp) return -1;
    
    int result = fputs(data, fp);
    fclose(fp);
    return result;
}
```

---

## 12. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2024-01-15 | System Team | Initial release |
| 1.1 | 2024-02-01 | System Team | Added memory-mapped I/O |
| 1.2 | 2024-02-15 | System Team | Added error handling best practices |

