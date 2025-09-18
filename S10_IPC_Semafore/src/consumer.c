#include "shared.h"
#include <time.h>

/**
 * @brief Ăn 1 món chay từ mâm (hàm one-shot).
 *
 * @param data Con trỏ đến bộ nhớ chia sẻ.
 * @return int 1 nếu ăn thành công, 0 nếu thất bại.
 */
int eat_vegan_once(shared_data_t *data) {
    sem_wait(&data->vegan_full);
    sem_wait(&data->vegan_mutex);

    int success = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->vegan_tray[i] == VEGAN) {
            data->vegan_tray[i] = EMPTY;
            printf("[Khách hàng ăn chay] Đã ăn món chay\n");
            success = 1;
            break;
        }
    }

    sem_post(&data->vegan_mutex);
    sem_post(&data->vegan_empty);
    return success;
}

/**
 * @brief Ăn 1 món mặn từ mâm (hàm one-shot).
 *
 * @param data Con trỏ đến bộ nhớ chia sẻ.
 * @return int 1 nếu ăn thành công, 0 nếu thất bại.
 */
int eat_nonvegan_once(shared_data_t *data) {
    sem_wait(&data->non_full);
    sem_wait(&data->non_mutex);

    int success = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->nonvegan_tray[i] == NON_VEGAN) {
            data->nonvegan_tray[i] = EMPTY;
            printf("[Khách hàng ăn mặn] Đã ăn món mặn\n");
            success = 1;
            break;
        }
    }

    sem_post(&data->non_mutex);
    sem_post(&data->non_empty);
    return success;
}

/**
 * @brief Hành vi khách hàng chay (loop vô hạn).
 */
void customer_vegan(shared_data_t *data) {
    srand(getpid());
    while (1) {
        eat_vegan_once(data);
        sleep(rand() % 6 + 10);
    }
}

/**
 * @brief Hành vi khách hàng mặn (loop vô hạn).
 */
void customer_nonvegan(shared_data_t *data) {
    srand(getpid());
    while (1) {
        eat_nonvegan_once(data);
        sleep(rand() % 6 + 10);
    }
}

/**
 * @brief Hành vi khách hàng hybrid (ăn chay trước, sau đó ăn mặn).
 */
void customer_hybrid(shared_data_t *data) {
    srand(getpid());
    while (1) {
        eat_vegan_once(data);
        eat_nonvegan_once(data);
        sleep(rand() % 6 + 10);
    }
}
