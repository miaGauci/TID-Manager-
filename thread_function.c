#include "task1.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void *thread_function(void *arg) {
    int *sleep_times = (int *)arg;
    int min_sleep = sleep_times[0];     // Minimum random sleep time
    int max_sleep = sleep_times[1]; // Maximum random sleep time
    free(arg);

    printf("Thread %lu: Requesting TID\n", pthread_self()); // Debug print
    int tid = allocate_tid();
    if (tid == -1) {
        printf("Thread %lu: Unable to allocate TID.\n", pthread_self());
        return NULL;
    }

    printf("Thread %lu: Allocated TID: %d\n", pthread_self(), tid);

    // Generate random sleep time between min_sleep and max_sleep
    int sleep_time = min_sleep + rand() % (max_sleep - min_sleep + 1);
    printf("Thread %lu: Sleeping for %d seconds\n", pthread_self(), sleep_time);
    sleep(sleep_time);

    // Release the TID before exiting the thread
    printf("Thread %lu: Releasing TID: %d\n", pthread_self(), tid);
    release_tid(tid);
    printf("Thread %lu: Released TID: %d\n", pthread_self(), tid);

    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = 100; // Default number of threads
    int min_sleep = 1;     // Default minimum random sleep time
    int max_sleep = 10;    // Default maximum random sleep time

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                num_threads = atoi(argv[i + 1]);
                if (num_threads <= 0) {
                    fprintf(stderr, "Error: Number of threads must be positive.\n");
                    return -1;
                }
                i++; // Skip the next argument
            } else {
                fprintf(stderr, "Error: Missing value for -n.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                min_sleep = atoi(argv[i + 1]);
                if (min_sleep < 0) {
                    fprintf(stderr, "Error: Minimum sleep time must be non-negative.\n");
                    return -1;
                }
                i++; // Skip the next argument
            } else {
                fprintf(stderr, "Error: Missing value for -l.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 < argc) {
                max_sleep = atoi(argv[i + 1]);
                if (max_sleep < 0 || max_sleep < min_sleep) {
                    fprintf(stderr, "Error: Maximum sleep time must be non-negative and greater than or equal to minimum sleep time.\n");
                    return -1;
                }
                i++; // Skip the next argument
            } else {
                fprintf(stderr, "Error: Missing value for -h.\n");
                return -1;
            }
        } else {
            fprintf(stderr, "Error: Unknown argument %s\n", argv[i]);
            return -1;
        }
    }

    printf("Number of threads: %d\n", num_threads); // Debug print
    printf("Sleep range: %d to %d seconds\n", min_sleep, max_sleep); // Debug print

    srand(time(NULL));

    // Initialize the TID manager data structures
    printf("Initializing TID manager...\n"); // Debug print
    int allocation_result = allocate_map();
    if (allocation_result != 1) {
        fprintf(stderr, "Failed to initialize the data structure representing TIDs.\n");
        return -1;
    }

    // Create threads based on the specified number
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        int *sleep_times = (int *)malloc(2 * sizeof(int));
        if (sleep_times == NULL) {
            perror("Failed to allocate memory");
            // Free already allocated threads
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
            }
            exit(EXIT_FAILURE);
        }
        sleep_times[0] = min_sleep;
        sleep_times[1] = max_sleep;
        if (pthread_create(&threads[i], NULL, thread_function, (void *)sleep_times) != 0) {
            perror("Failed to create thread");
            free(sleep_times);
            // Free already allocated threads
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
            }
            exit(EXIT_FAILURE);
        }
        printf("Created thread %d\n", i); // Debug print
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            exit(EXIT_FAILURE);
        }
        printf("Joined thread %d\n", i); // Debug print
    }

    printf("All threads have finished execution.\n"); // Debug print
    return 0;
}
