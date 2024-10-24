# File Word Counter Using Multiprocessing and Multithreading

## Description of the Project
This program efficiently counts occurrences of a desired target_word in 7+ files using both multiprocessing and multithreading. It takes advantage of parallel processing to enhance performance, especially with large files, by creating separate processes for each file and utilizing multiple threads within each process to speed up the word counting operation.

## How Multiprocessing and Multithreading are Used
- **Multiprocessing**: The project uses the **fork()** system call to create a new child process for each file specified in the command line arguments. Each child process operates independently.
  
- **Multithreading**: Within each child process, the application creates multiple threads using POSIX threads (pthreads) to divide the workload of counting words in the file into smaller chunks. Each thread processes a different section of the file simultaneously, reducing the overall time required.

## Advantages and Disadvantages of Each Approach

### Advantages:
- **Multiprocessing**:
  - Each process has its own memory space, which avoids issues related to shared memory corruption.
  - If one process crashes, it does not affect the others.
  - Allows true parallelism on multi-core system.

- **Multithreading**:
  - Lower memory overhead compared to processes, as threads share the same memory space of the parent process.
  - Faster context switching between threads compared to processes, leading to improved performance for CPU-bound tasks.

### Disadvantages:
- **Multiprocessing**:
  - Higher overhead due to memory allocation for separate process spaces.
  - Communication between processes is more complex (requires IPC mechanisms).
  
- **Multithreading**:
  - Risk of data corruption if multiple threads access shared data without proper synchronization.
  - Debugging is more complex due to potential race conditions and deadlocks.

## Structure of the Code
The project consists of several components:

1. **Main Function**: Handles command-line arguments and initializes shared memory for inter-process communication (IPC).
2. **Process Creation**: Uses **fork()** to create a child process for each file.
3. **Thread Management**: Uses pthreads to create multiple threads within each process.
4. **IPC Mechanism**: Utilizes shared memory to communicate word counts back to the parent process.
5. **Error Handling**: Implements error checks for process creation, IPC setup, and file handling. Utilizes **Mutex** to prevent synchronization issues.

### Diagrams

#### 1. Process and Thread Creation


## Implementation Required by Project Requirements

### Process Management
- Each file is handled by a separate child process created with `fork()`. This ensures isolation and allows concurrent processing of multiple files.

### IPC Mechanism
- The application uses shared memory created via `shm_open()` and `mmap()` to store the results (word counts) from each child process, allowing the parent process to retrieve the results after all child processes have finished executing.

### Threading
- Inside each child process, multiple threads are created using pthreads. The file is divided into chunks, and each thread counts occurrences of the target word in its assigned chunk. Mutex locks are used to protect access to shared variables (like the total word count).

### Error Handling
- The code checks for errors during file opening, process creation, thread creation, and shared memory operations, ensuring the program handles failures gracefully.

## Performance Evaluation

### Instructions to Run the System
1. Compile the code using:

