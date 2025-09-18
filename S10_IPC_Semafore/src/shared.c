#include "shared.h"

#define SHM_NAME "/restaurant_shm"

static int shm_fd = -1;

shared_data_t *init_shared_memory() {
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }

    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        perror("ftruncate"); exit(1);
    }

    shared_data_t *data = mmap(NULL, sizeof(shared_data_t),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); exit(1); }

    // Khởi tạo dữ liệu ban đầu
    memset(data->vegan_tray, 0, sizeof(data->vegan_tray));
    memset(data->nonvegan_tray, 0, sizeof(data->nonvegan_tray));

    // Khởi tạo semaphores
    sem_init(&data->vegan_mutex, 1, 1);
    sem_init(&data->vegan_empty, 1, TRAY_SIZE);
    sem_init(&data->vegan_full, 1, 0);

    sem_init(&data->non_mutex, 1, 1);
    sem_init(&data->non_empty, 1, TRAY_SIZE);
    sem_init(&data->non_full, 1, 0);

    return data;
}

void cleanup_shared_memory() {
    shm_unlink(SHM_NAME);
}

void print_tray_status(shared_data_t *data) {
    int vegan_count = 0, non_count = 0;
    for (int i=0; i<TRAY_SIZE; i++) {
        if (data->vegan_tray[i] != EMPTY) vegan_count++;
        if (data->nonvegan_tray[i] != EMPTY) non_count++;
    }
    printf("[Trạng thái] Mâm đồ chay: %d/%d, Mâm đồ mặn: %d/%d\n",
           vegan_count, TRAY_SIZE, non_count, TRAY_SIZE);
}
