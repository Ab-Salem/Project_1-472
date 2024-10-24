%%writefile fileManager.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/resource.h>  // for getrusage()

#define MAX_WORD_LENGTH 1000
#define MAX_FILENAME_LENGTH 256
#define MAX_FILES 10

// structure to hold results from each process
typedef struct {
    int word_count;
    char filename[MAX_FILENAME_LENGTH];
} SharedData;

// structure to hold file chunk information for threads
typedef struct {
    long start;
    long end;
    char* filename;
    char* target_word;
    int* local_count;
    pthread_mutex_t* mtx;
} FileChunk;

// convert string to lowercase
void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// function to count words in a specific chunk of file
void* count_words_in_chunk(void* arg) {
    FileChunk* chunk = (FileChunk*)arg;
    FILE* file = fopen(chunk->filename, "r");
    if (!file) {
        perror("failed to open file in thread");
        return NULL;
    }

    // seek to chunk start
    fseek(file, chunk->start, SEEK_SET);
    
    char word[MAX_WORD_LENGTH];
    int count = 0;
    long current_pos;

    // read words until chunk end
    while ((current_pos = ftell(file)) < chunk->end && fscanf(file, "%s", word) == 1) {
        to_lower(word);
        if (strcmp(word, chunk->target_word) == 0) {
            count++;
        }
    }

    // update shared counter with mutex protection
    pthread_mutex_lock(chunk->mtx);
    *chunk->local_count += count;
    pthread_mutex_unlock(chunk->mtx);

    fclose(file);
    return NULL;
}

// function to count words using a single thread
int count_words_single_thread(const char* filename, const char* target_word) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("failed to open file");
        return -1;
    }
    
    char word[MAX_WORD_LENGTH];
    int count = 0;

    while (fscanf(file, "%s", word) == 1) {
        to_lower(word);
        if (strcmp(word, target_word) == 0) {
            count++;
        }
    }

    fclose(file);
    return count;
}

// function to process a single file using multiple threads
int process_file(const char* filename, const char* target_word, int num_threads) {
    // get file size
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("failed to open file");
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fclose(file);

    // initialize thread-related variables
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    FileChunk* chunks = malloc(num_threads * sizeof(FileChunk));
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    int total_count = 0;

    // create threads to process file chunks
    long chunk_size = file_size / num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        chunks[i].start = i * chunk_size;
        chunks[i].end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;
        chunks[i].filename = strdup(filename);
        chunks[i].target_word = strdup(target_word);
        chunks[i].local_count = &total_count;
        chunks[i].mtx = &mtx;

        if (pthread_create(&threads[i], NULL, count_words_in_chunk, &chunks[i]) != 0) {
            perror("failed to create thread");
            return -1;
        }
    }

    // wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        free(chunks[i].filename);
        free(chunks[i].target_word);
    }

    // cleanup
    free(threads);
    free(chunks);
    pthread_mutex_destroy(&mtx);

    return total_count;
}

void print_performance_metrics(const struct rusage* usage) {
    printf("user cpu time used: %ld.%06ld sec\n", usage->ru_utime.tv_sec, usage->ru_utime.tv_usec);
    printf("system cpu time used: %ld.%06ld sec\n", usage->ru_stime.tv_sec, usage->ru_stime.tv_usec);
    printf("peak amount of memory that the process occupied in ram (memory): %ld kb\n", usage->ru_maxrss);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "usage: %s <target_word> <num_threads> <single_threaded: 0/1> <file1> [file2] ...\n", argv[0]);
        return 1;
    }

    char* target_word = argv[1];
    int num_threads = atoi(argv[2]);
    int single_threaded = atoi(argv[3]);  // new argument to choose between single-threaded or multi-threaded mode
    int num_files = argc - 4;

    if (num_files > MAX_FILES) {
        fprintf(stderr, "too many files. maximum is %d\n", MAX_FILES);
        return 1;
    }

    // output mode at the beginning
    if (single_threaded) {
        printf("running in single-threaded mode.\n");
    } else {
        printf("running in multi-threaded mode with %d threads per file.\n", num_threads);
    }

    // create shared memory for inter-process communication
    int shm_fd = shm_open("/word_count_results", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("failed to create shared memory");
        return 1;
    }

    // set size for shared memory
    if (ftruncate(shm_fd, num_files * sizeof(SharedData)) == -1) {
        perror("failed to set shared memory size");
        return 1;
    }

    SharedData* shared_results = (SharedData*)mmap(
        NULL,
        num_files * sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    // record start time
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // create child processes for each file
    pid_t* child_pids = malloc(num_files * sizeof(pid_t));
    
    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) { // child process
            to_lower(target_word);  // convert target word to lowercase
            int count;

            if (single_threaded) {
                count = count_words_single_thread(argv[i + 4], target_word);  // use single-threaded mode
            } else {
                count = process_file(argv[i + 4], target_word, num_threads);  // use multi-threaded mode
            }
            
            // store results in shared memory
            shared_results[i].word_count = count;
            strncpy(shared_results[i].filename, argv[i + 4], MAX_FILENAME_LENGTH - 1);
            
            exit(0);
        } else {
            child_pids[i] = pid;
        }
    }

    // wait for all child processes to complete
    for (int i = 0; i < num_files; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
    }

    // record end time and calculate duration
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double duration = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    // measure performance metrics using getrusage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    // print results
    printf("\nresults:\n");
    for (int i = 0; i < num_files; i++) {
        printf("file: %s - word count: %d\n",
               shared_results[i].filename,
               shared_results[i].word_count);
    }
    
    printf("\ntotal processing time: %.3f seconds\n", duration);
    print_performance_metrics(&usage);

    // cleanup
    free(child_pids);
    munmap(shared_results, num_files * sizeof(SharedData));
    shm_unlink("/word_count_results");

    return 0;
}
