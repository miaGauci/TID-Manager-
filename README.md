# TID Manager Project
## Overview
This project implements a Thread ID (TID) Manager that manages thread identifiers in a multithreaded environment. The system allocates and releases TIDs in a thread-safe manner using mutexes for synchronization. It also includes a simulation where multiple threads request TIDs, perform tasks, and release the TIDs after completing their work.

Code Explanation
1. TIDManager Structure
The TIDManager structure holds:

A bitmap for tracking available TIDs, where each bit represents one TID.
A mutex to ensure synchronization between threads when accessing or modifying the bitmap.
2. Main Functions
1. allocate_map()
This function initializes the TID manager:

It sets up a mutex to ensure thread-safe operations.
It initializes the bitmap where all TIDs above a certain threshold (MIN_TID) are marked as available.
c
Copy code
int allocate_map() {
    printf("Initializing mutex...\n");
    if (pthread_mutex_init(&tid_manager.mutex, NULL) != 0) {
        perror("Mutex init failed");
        return -1;
    }
    printf("Mutex initialized.\n");

    for (int i = 0; i < BITMAP_SIZE; i++) {
        tid_manager.bitmap[i] = 0xFF;
    }
    printf("Bitmap initialized.\n");

    for (int i = 0; i < (MIN_TID / 8); i++) {
        tid_manager.bitmap[i] = 0x00;
    }
    printf("Marked TIDs less than MIN_TID as unavailable.\n");

    return 1;
}
2. allocate_tid()
This function attempts to allocate a TID:

It locks the mutex to prevent other threads from interfering while it checks for an available TID.
It uses bitwise operations to find the first free TID and marks it as unavailable.
It unlocks the mutex after successfully allocating the TID.
c
Copy code
int allocate_tid() {
    pthread_t kernel_thread_id = pthread_self();
    while (pthread_mutex_trylock(&tid_manager.mutex) != 0) {
        usleep((unsigned int)(kernel_thread_id % 100) * 10);
    }
    printf("Attempting to allocate TID...\n");

    for (int i = 0; i < BITMAP_SIZE; i++) {
        unsigned char mask = 0x80;
        for (int j = 0; j < 8; j++) {
            if ((tid_manager.bitmap[i] & mask) == 0) {
                tid_manager.bitmap[i] |= mask;
                pthread_mutex_unlock(&tid_manager.mutex);
                int tid = MIN_TID + i * 8 + j;
                printf("Allocated TID: %d\n", tid);
                return tid;
            }
            mask >>= 1;
        }
    }

    pthread_mutex_unlock(&tid_manager.mutex);
    printf("No available TIDs\n");
    return -1;
}
3. release_tid(int tid)
This function releases a TID:

It locks the mutex, marks the specified TID as available, and unlocks the mutex afterward.
It checks that the TID is within a valid range before releasing it.
c
Copy code
void release_tid(int tid) {
    pthread_t kernel_thread_id = pthread_self();
    while (pthread_mutex_trylock(&tid_manager.mutex) != 0) {
        usleep((unsigned int)(kernel_thread_id % 100) * 10);
    }

    if (tid < MIN_TID || tid > MAX_TID) {
        pthread_mutex_unlock(&tid_manager.mutex);
        printf("TID %d out of range\n", tid);
        return;
    }

    int byte_index = (tid - MIN_TID) / 8;
    int bit_index = (tid - MIN_TID) % 8;
    unsigned char mask = 1 << (7 - bit_index);

    tid_manager.bitmap[byte_index] &= ~mask;

    pthread_mutex_unlock(&tid_manager.mutex);
    printf("Released TID: %d\n", tid);
}
3. Thread Function
Each thread:

Requests a TID using allocate_tid().
Sleeps for a random period between specified minimum and maximum times.
Releases the TID using release_tid().
c
Copy code
void *thread_function(void *arg) {
    int *sleep_times = (int *)arg;
    int min_sleep = sleep_times[0];
    int max_sleep = sleep_times[1];
    free(arg);

    printf("Thread %lu: Requesting TID\n", pthread_self());
    int tid = allocate_tid();
    if (tid == -1) {
        printf("Thread %lu: Unable to allocate TID.\n", pthread_self());
        return NULL;
    }

    printf("Thread %lu: Allocated TID: %d\n", pthread_self(), tid);

    int sleep_time = min_sleep + rand() % (max_sleep - min_sleep + 1);
    printf("Thread %lu: Sleeping for %d seconds\n", pthread_self(), sleep_time);
    sleep(sleep_time);

    printf("Thread %lu: Releasing TID: %d\n", pthread_self(), tid);
    release_tid(tid);
    printf("Thread %lu: Released TID: %d\n", pthread_self(), tid);

    return NULL;
}
How to Run
To run the program with 5 threads and a random sleep time between 2 and 5 seconds:

bash
Copy code
./tid_manager -n 5 -l 2 -h 5
