#ifndef TASK1_H
#define TASK1_H

#include <pthread.h>

// Constants for TID range
#define MIN_TID 300
#define MAX_TID 5000

// Bitmap size calculation assuming 1 bit per TID
#define BITMAP_SIZE ((MAX_TID - MIN_TID + 1) / 8 + 1)

// Structure to manage TIDs
typedef struct {
    unsigned char bitmap[BITMAP_SIZE];
    pthread_mutex_t mutex;
} TIDManager;

extern TIDManager tid_manager;

void initialize_tid_manager();
int allocate_tid();
void release_tid(int tid);
int allocate_map();

#endif // TASK1_H
