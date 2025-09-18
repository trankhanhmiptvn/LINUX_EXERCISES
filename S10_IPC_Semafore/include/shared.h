#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRAY_SIZE 10

/// Loại món ăn
typedef enum {
    EMPTY = 0,
    VEGAN = 1,
    NON_VEGAN = 2
} food_type_t;

/// Bộ nhớ chia sẻ cho 2 mâm thức ăn
typedef struct {
    int vegan_tray[TRAY_SIZE];     ///< mâm đồ chay
    int nonvegan_tray[TRAY_SIZE];  ///< mâm đồ mặn

    // semaphores cho mâm chay
    sem_t vegan_mutex;
    sem_t vegan_empty;
    sem_t vegan_full;

    // semaphores cho mâm mặn
    sem_t non_mutex;
    sem_t non_empty;
    sem_t non_full;
} shared_data_t;

// ===== prototypes =====
shared_data_t *init_shared_memory();
void cleanup_shared_memory();
void print_tray_status(shared_data_t *data);

#endif
