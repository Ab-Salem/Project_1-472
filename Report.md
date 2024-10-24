# Word Counter Using **Multiprocessing** and **Multithreading**

## Description
This program efficiently counts occurrences of a desired target_word in 7+ files using both multiprocessing and multithreading. It takes advantage of parallel processing to enhance performance, especially with large files, by creating separate processes for each file and utilizing multiple threads within each process to speed up the word counting operation.

## How Multiprocessing and Multithreading are Used
- **Multiprocessing**: The project uses the **fork()** system call to create a new child process for each file specified in the command line arguments. Each child process operates independently.
  
- **Multithreading**: Within each child process, the application creates multiple threads using pthreads to divide the workload of counting words in the file into smaller chunks. Each thread processes a different section of the file simultaneously, reducing the overall time required.

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
- **Process and Thread Creation**  
![image](https://github.com/user-attachments/assets/c4e34ae6-4d5f-4fdc-bde3-5381a9fcdbb6)  
- **IPC (Shared Memory)**  
![image](https://github.com/user-attachments/assets/99ab5f85-2582-49f2-8927-2870978a9303)


## Implementation 
### Process Management
- Each file is handled by a separate child process created with **fork()**. This ensures isolation and allows concurrent processing of multiple files.

### IPC Mechanism
- The application uses shared memory created via **shm_open()** and **mmap()** to store the results (word counts) from each child process, allowing the parent process to retrieve the results after all child processes have finished executing.

### Threading
- Inside each child process, multiple threads are created using pthreads. The file is divided into chunks, and each thread counts occurrences of the target word in its assigned chunk. Mutex locks are used to protect access to shared variables (like the total word count).

### Error Handling
- The code checks for errors during file opening, process creation, thread creation, and shared memory operations, ensuring the program handles failures gracefully.

### Performance Evaluation
- Single Thread Mode:
  ![image](https://github.com/user-attachments/assets/3e60545b-1390-45f9-9d6a-b960e5bc0deb)  
- Multi Threaded Mode:
  ![image](https://github.com/user-attachments/assets/37da585e-ea37-45c9-94dd-65287dcca4a9)
  - **Analysis**:
    The performance results show that the single-threaded mode took 0.010 seconds, while the multi-threaded mode took 0.020 seconds, which is unexpected as I would expect multi-threading to have faster processing times. This outcome may indicate that the overhead associated with thread management and context switching in the multi-threaded implementation outweighed the benefits of parallel processing for the specific file sizes used in the tests. Furthermore, the files being processed were relatively small and the target word count was low. The latency incurred from creating and synchronizing threads may have resulted in slower overall performance compared to the single-threaded approach, which executes sequentially without this overhead. These results suggest that for certain tasks or datasets, a single-threaded approach can be more efficient.
  

### Instructions to Run the System
**see Instructions.md**

