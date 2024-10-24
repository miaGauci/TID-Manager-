#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // for usleep
#include "task1.h"

TIDManager tid_manager;

int allocate_map() {
    printf("Initializing mutex...\n"); // Debug print
    if (pthread_mutex_init(&tid_manager.mutex, NULL) != 0) {
        perror("Mutex init failed");
        return -1; // Mutex initialization failed
    }
    printf("Mutex initialized.\n"); // Debug print

    for (int i = 0; i < BITMAP_SIZE; i++) {
        tid_manager.bitmap[i] = 0xFF; // Initialize all bits as available (1)
    }
    printf("Bitmap initialized.\n"); // Debug print

    // Mark TIDs less than MIN_TID as unavailable
    for (int i = 0; i < (MIN_TID / 8); i++) {
        tid_manager.bitmap[i] = 0x00;
    }
    printf("Marked TIDs less than MIN_TID as unavailable.\n"); // Debug print

    printf("TID map allocated successfully.\n"); // Debug print
    return 1; // Indicate success
}

int allocate_tid() {
    pthread_t kernel_thread_id = pthread_self();
    while (pthread_mutex_trylock(&tid_manager.mutex) != 0) {
        // Backoff based on thread ID
        usleep((unsigned int)(kernel_thread_id % 100) * 10); // backoff delay based on thread ID
    }
    printf("Attempting to allocate TID...\n"); // Debug print

    for (int i = 0; i < BITMAP_SIZE; i++) {
        unsigned char mask = 0x80; // Start from the most significant bit
        for (int j = 0; j < 8; j++) {
            if ((tid_manager.bitmap[i] & mask) == 0) { // Check if TID is available
                tid_manager.bitmap[i] |= mask;           // Mark TID as unavailable
                pthread_mutex_unlock(&tid_manager.mutex); // Unlock mutex
                int tid = MIN_TID + i * 8 + j;
                printf("Allocated TID: %d\n", tid); // Debug print
                return tid;                          // Return TID considering MIN_TID offset
            }
            mask >>= 1; // Move to the next bit
        }
    }

    pthread_mutex_unlock(&tid_manager.mutex); // Unlock mutex
    printf("No available TIDs\n"); // Debug print
    return -1; // No available TIDs
}

void release_tid(int tid) {
    pthread_t kernel_thread_id = pthread_self();
    while (pthread_mutex_trylock(&tid_manager.mutex) != 0) {
        // Backoff based on thread ID
        usleep((unsigned int)(kernel_thread_id % 100) * 10); // backoff delay based on thread ID
    }

    // Release a TID
    if (tid < MIN_TID || tid > MAX_TID) {
        pthread_mutex_unlock(&tid_manager.mutex); // Unlock mutex
        printf("TID %d out of range\n", tid); // Debug print
        return; // TID out of range
    }

    int byte_index = (tid - MIN_TID) / 8;
    int bit_index = (tid - MIN_TID) % 8;
    unsigned char mask = 1 << (7 - bit_index);

    tid_manager.bitmap[byte_index] &= ~mask; // Mark TID as available

    pthread_mutex_unlock(&tid_manager.mutex); // Unlock mutex
    printf("Released TID: %d\n", tid); // Debug print
}
