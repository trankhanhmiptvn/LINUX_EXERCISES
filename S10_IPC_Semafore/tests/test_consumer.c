#include "shared.h"
#include <assert.h>

extern int eat_vegan_once(shared_data_t *data);
extern int eat_nonvegan_once(shared_data_t *data);

int main() {
    printf("Running customer unit tests...\n");
    shared_data_t *data = init_shared_memory();

    // Chuẩn bị dữ liệu: đặt 1 món chay và 1 món mặn vào mâm
    data->vegan_tray[0] = VEGAN;
    data->nonvegan_tray[0] = NON_VEGAN;

    // Update semaphores tương ứng
    sem_trywait(&data->vegan_empty); 
    sem_post(&data->vegan_full);

    sem_trywait(&data->non_empty);
    sem_post(&data->non_full);

    // Test ăn món chay
    int r1 = eat_vegan_once(data);
    assert(r1 == 1);  // phải thành công
    assert(data->vegan_tray[0] == EMPTY);

    // Test ăn món mặn
    int r2 = eat_nonvegan_once(data);
    assert(r2 == 1);  
    assert(data->nonvegan_tray[0] == EMPTY);

    printf("All customer unit tests passed!\n");

    cleanup_shared_memory();
    return 0;
}
