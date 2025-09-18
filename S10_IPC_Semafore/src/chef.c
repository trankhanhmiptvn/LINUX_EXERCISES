#include "shared.h"
#include <time.h>

/// Danh sách món chay
static const char *vegan_dishes[] = {
   "Gỏi cuốn chay",
   "Đậu hũ sốt cà chua"
};

/// Danh sách món mặn
static const char *nonvegan_dishes[] = {
    "Phở bò tái nạm",
    "Sườn nướng mật ong"
};

/**
 * @brief Đầu bếp chay (Chef Portecelli) thêm món ăn vào mâm chay.
 *
 * @param data Con trỏ đến vùng nhớ chia sẻ chứa mâm thức ăn và semaphore.
 */
void chef_vegan(shared_data_t *data) {
    srand(getpid());
    while (1) {
        const char *dish = vegan_dishes[rand() % 2];
        sleep(rand() % 5 + 1);

        sem_wait(&data->vegan_empty);   ///< Chờ nếu mâm đầy
        sem_wait(&data->vegan_mutex);   ///< Khóa mâm để tránh race condition

        for (int i = 0; i < TRAY_SIZE; i++) {
            if (data->vegan_tray[i] == EMPTY) {
                data->vegan_tray[i] = VEGAN;
                printf("[Đầu bếp chay] Thêm món chay: %s\n", dish);
                break;
            }
        }

        sem_post(&data->vegan_mutex);   ///< Mở khóa
        sem_post(&data->vegan_full);    ///< Tăng số lượng món có sẵn
    }
}

/**
 * @brief Đầu bếp mặn (Chef Donatello) thêm món ăn vào mâm mặn.
 *
 * @param data Con trỏ đến vùng nhớ chia sẻ chứa mâm thức ăn và semaphore.
 */
void chef_nonvegan(shared_data_t *data) {
    srand(getpid());
    while (1) {
        const char *dish = nonvegan_dishes[rand() % 2];
        sleep(rand() % 5 + 1);

        sem_wait(&data->non_empty);
        sem_wait(&data->non_mutex);

        for (int i = 0; i < TRAY_SIZE; i++) {
            if (data->nonvegan_tray[i] == EMPTY) {
                data->nonvegan_tray[i] = NON_VEGAN;
                printf("[Đầu bếp mặn] Thêm món mặn: %s\n", dish);
                break;
            }
        }

        sem_post(&data->non_mutex);
        sem_post(&data->non_full);
    }
}

int add_vegan_dish_once(shared_data_t *data, const char *dish_name) {
    sem_wait(&data->vegan_empty);
    sem_wait(&data->vegan_mutex);

    int success = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->vegan_tray[i] == EMPTY) {
            data->vegan_tray[i] = VEGAN;
            printf("[Đầu bếp chay] Thêm món chay: %s\n", dish_name);
            success = 1;
            break;
        }
    }

    sem_post(&data->vegan_mutex);
    sem_post(&data->vegan_full);
    return success;
}

int add_nonvegan_dish_once(shared_data_t *data, const char *dish_name) {
    sem_wait(&data->non_empty);
    sem_wait(&data->non_mutex);

    int success = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->nonvegan_tray[i] == EMPTY) {
            data->nonvegan_tray[i] = NON_VEGAN;
            printf("[Đầu bếp mặn] Thêm món mặn: %s\n", dish_name);
            success = 1;
            break;
        }
    }

    sem_post(&data->non_mutex);
    sem_post(&data->non_empty);
    return success;
}
