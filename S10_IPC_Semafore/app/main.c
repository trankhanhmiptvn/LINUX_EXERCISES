/**
 * @file main.c
 * @brief Tiến trình chính: khởi tạo bộ nhớ chia sẻ, tạo tiến trình con (chef & customer),
 *        in trạng thái mâm thức ăn định kỳ, cleanup sau 60 giây hoặc khi nhấn Ctrl+C.
 */

#include "shared.h"
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

/// Khai báo các hàm trong chef.c và customer.c
extern void chef_vegan(shared_data_t *data);
extern void chef_nonvegan(shared_data_t *data);
extern void customer_vegan(shared_data_t *data);
extern void customer_nonvegan(shared_data_t *data);
extern void customer_hybrid(shared_data_t *data);

/// Lưu PID các tiến trình con
pid_t children[5];
int child_count = 0;

/// Con trỏ đến vùng nhớ chia sẻ
shared_data_t *g_data = NULL;

/**
 * @brief Dọn dẹp tiến trình con và shared memory.
 */
void cleanup_all() {
    printf("\n[Main] Cleaning up resources...\n");

    // Gửi tín hiệu dừng cho tất cả tiến trình con
    for (int i = 0; i < child_count; i++) {
        kill(children[i], SIGTERM);
    }

    // Thu hoạch (reap) các tiến trình con để tránh zombie
    for (int i = 0; i < child_count; i++) {
        waitpid(children[i], NULL, 0);
    }

    // Dọn dẹp shared memory
    if (g_data != NULL) {
        cleanup_shared_memory();
    }
}

/**
 * @brief Handler cho tín hiệu SIGINT (Ctrl+C).
 */
void handle_sigint(int sig) {
    (void)sig; // tránh warning unused
    printf("\n[Main] Nhận tín hiệu Ctrl+C (SIGINT)\n");
    cleanup_all();
    exit(0);
}

/**
 * @brief Hàm main - điểm bắt đầu của chương trình.
 */
int main() {
    g_data = init_shared_memory();
    time_t start = time(NULL);

    // Đăng ký handler cho Ctrl+C (chỉ tiến trình cha xử lý)
    signal(SIGINT, handle_sigint);

    // ===== Fork tiến trình con (Producers) =====
    if ((children[child_count++] = fork()) == 0) {
        signal(SIGINT, SIG_IGN);  // Con bỏ qua Ctrl+C
        chef_nonvegan(g_data);
    }
    if ((children[child_count++] = fork()) == 0) {
        signal(SIGINT, SIG_IGN);
        chef_vegan(g_data);
    }

    // ===== Fork tiến trình con (Consumers) =====
    if ((children[child_count++] = fork()) == 0) {
        signal(SIGINT, SIG_IGN);
        customer_nonvegan(g_data);
    }
    if ((children[child_count++] = fork()) == 0) {
        signal(SIGINT, SIG_IGN);
        customer_vegan(g_data);
    }
    if ((children[child_count++] = fork()) == 0) {
        signal(SIGINT, SIG_IGN);
        customer_hybrid(g_data);
    }

    // ===== Tiến trình cha =====
    while (1) {
        sleep(10);
        print_tray_status(g_data);

        if (time(NULL) - start >= 60) {
            printf("[Main] Simulation finished after 60 seconds.\n");
            cleanup_all();
            break;
        }
    }

    return 0;
}
