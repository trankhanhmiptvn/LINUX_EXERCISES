/**
 * @file test_shared.c
 * @brief Unit test cho thư viện shared.c
 *
 * Kiểm tra:
 *  - Khởi tạo và truy cập bộ nhớ chia sẻ
 *  - Thao tác trên mâm (tray) chay & mặn
 *  - Semaphore hoạt động cơ bản
 */

#include "shared.h"
#include <assert.h>

int main() {
    printf("===== RUNNING UNIT TEST: shared.c =====\n");

    // 1. Init shared memory
    shared_data_t *data = init_shared_memory();
    assert(data != NULL);
    printf("[TEST] init_shared_memory() OK\n");

    // 2. Kiểm tra mâm ban đầu rỗng
    for (int i = 0; i < TRAY_SIZE; i++) {
        assert(data->vegan_tray[i] == EMPTY);
        assert(data->nonvegan_tray[i] == EMPTY);
    }
    printf("[TEST] Trays are empty after init OK\n");

    // 3. Thêm món vào mâm chay
    data->vegan_tray[0] = VEGAN;
    data->vegan_tray[1] = VEGAN;

    // Thêm món vào mâm mặn
    data->nonvegan_tray[0] = NON_VEGAN;

    print_tray_status(data);
    printf("[TEST] Added items to trays OK\n");

    // 4. Test semaphore hoạt động cơ bản
    int sval;
    sem_getvalue(&data->vegan_empty, &sval);
    assert(sval == TRAY_SIZE);  // vẫn là giá trị ban đầu
    printf("[TEST] Semaphore vegan_empty initialized to %d OK\n", sval);

    // 5. Cleanup shared memory
    cleanup_shared_memory();
    printf("[TEST] cleanup_shared_memory() OK\n");

    printf("===== ALL UNIT TESTS PASSED =====\n");
    return 0;
}
