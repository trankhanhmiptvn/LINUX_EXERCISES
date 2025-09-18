#include "shared.h"
#include <assert.h>

extern int add_vegan_dish_once(shared_data_t *data, const char *dish_name);
extern int add_nonvegan_dish_once(shared_data_t *data, const char *dish_name);

int main() {
    printf("Running chef unit tests...\n");
    shared_data_t *data = init_shared_memory();

    // Test thêm món chay
    int r1 = add_vegan_dish_once(data, "Test Vegan Dish");
    assert(r1 == 1);  // phải thành công
    int found = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->vegan_tray[i] == VEGAN) { found = 1; break; }
    }
    assert(found == 1);

    // Test thêm món mặn
    int r2 = add_nonvegan_dish_once(data, "Test Non-Vegan Dish");
    assert(r2 == 1);
    found = 0;
    for (int i = 0; i < TRAY_SIZE; i++) {
        if (data->nonvegan_tray[i] == NON_VEGAN) { found = 1; break; }
    }
    assert(found == 1);

    printf("✅ All chef unit tests passed!\n");

    cleanup_shared_memory();
    return 0;
}
