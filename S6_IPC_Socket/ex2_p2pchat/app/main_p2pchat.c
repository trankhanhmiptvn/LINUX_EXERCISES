/* p2pchat.c
   Compile: gcc -o p2pchat p2pchat.c -pthread
   Usage: ./p2pchat <listening_port>
   -> Đây là chương trình chat ngang hàng (peer-to-peer chat), 
      sử dụng socket và pthread để hỗ trợ nhiều kết nối song song.
*/

#include <stdio.h>      // Thư viện nhập/xuất chuẩn (printf, perror,...)
#include <stdlib.h>     // Thư viện hàm tiện ích (malloc, exit,...)
#include <string.h>     // Xử lý chuỗi (strcpy, memset,...)

// #include <stdarg.h>     // Hỗ trợ hàm có số lượng tham số biến đổi (vd: printf tuỳ chỉnh)


// #include <sys/socket.h> // Hàm socket API cơ bản (socket, bind, listen, accept,...)
// #include <sys/types.h>  // Định nghĩa kiểu dữ liệu cho socket (socklen_t, size_t,...)
#include "common.h"
#include "utils.h"
#include "connection.h"
#include "listener.h"
#include "command.h"


/* ================================
 * Main function: Parse and handle user commands
 * ================================
 */
int main(int argc, char *argv[]) {
    /* Kiểm tra số lượng tham số dòng lệnh.
     * Chương trình yêu cầu đúng 1 tham số: cổng lắng nghe.
     * Ví dụ: ./p2p 5000
     */
    if (argc != 2) {
        printf("Usage: %s <listening_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Chuyển tham số cổng (chuỗi) sang số nguyên */
    listen_port = atoi(argv[1]);

    /* Kiểm tra tính hợp lệ của cổng (1–65535) */
    if (listen_port <= 0 || listen_port > 65535) {
        printf("Invalid port.\n");
        exit(EXIT_FAILURE);
    }

    /* Lấy địa chỉ IP cục bộ của máy (nếu thất bại thì fallback = 127.0.0.1) */
    if (get_local_ip(local_ip, sizeof(local_ip)) != 0) {
        strncpy(local_ip, "127.0.0.1", sizeof(local_ip));
    }
    printf("[INFO] Local IP: %s, Listening port: %d\n", local_ip, listen_port);

    /* ========================
     * Tạo socket server (TCP)
     * ========================
     */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { 
        perror("socket"); 
        exit(EXIT_FAILURE); 
    }

    /* Cho phép tái sử dụng địa chỉ (tránh lỗi "Address already in use") */
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Cấu hình địa chỉ server (IPv4, ANY địa chỉ, port do user cung cấp) */
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;              // IPv4
    serv.sin_addr.s_addr = INADDR_ANY;      // Lắng nghe trên tất cả interface
    serv.sin_port = htons(listen_port);     // Chuyển port sang network byte order

    /* Bind socket với địa chỉ và cổng */
    if (bind(server_sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) { 
        perror("bind"); 
        exit(EXIT_FAILURE); 
    }

    /* Chuyển socket sang chế độ "listen" để chấp nhận kết nối từ client */
    if (listen(server_sock, BACKLOG) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }

    /* ========================
     * Tạo thread listener
     * ========================
     * Thread này sẽ chạy hàm listener_thread()
     * và xử lý việc accept() kết nối đến từ peer.
     */
    if (pthread_create(&listener_thread_id, NULL, listener_thread, &server_sock) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    cmd_help();

    /* ========================
     * Vòng lặp chính: đọc command từ người dùng
     * ========================
     */
    char line[512];
    while (running) {
        printf("p2p> ");  // prompt

        /* Đọc 1 dòng từ stdin (nếu EOF/Ctrl+D thì thoát) */
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        /* Xóa ký tự xuống dòng '\n' nếu có */
        char *nl = strchr(line, '\n'); 
        if (nl) *nl = '\0';

        /* Bỏ qua input rỗng */
        if (strlen(line) == 0) continue;

        /* Gom rác: loại bỏ các connection đã chết */
        reap_connections();

        /* Phân tích command (tách token đầu tiên) */
        char *cmd = strtok(line, " ");
        if (!cmd) continue;

        /* ========================
         * Xử lý từng loại command
         * ========================
         */
        if (strcmp(cmd, "help") == 0) {
            cmd_help();

        } else if (strcmp(cmd, "myip") == 0) {
            printf("%s\n", local_ip);

        } else if (strcmp(cmd, "myport") == 0) {
            printf("%d\n", listen_port);

        } else if (strcmp(cmd, "connect") == 0) {
            char *ip = strtok(NULL, " ");
            char *port = strtok(NULL, " ");
            if (!ip || !port) { 
                printf("[ERROR] Usage: connect <ip> <port>\n"); 
                continue; 
            }
            cmd_connect(ip, port);

        } else if (strcmp(cmd, "list") == 0) {
            cmd_list();

        } else if (strcmp(cmd, "terminate") == 0) {
            char *idstr = strtok(NULL, " ");
            if (!idstr) { 
                printf("[ERROR] Usage: terminate <id>\n"); 
                continue; 
            }
            int id = atoi(idstr);
            if (id <= 0) { 
                printf("[ERROR] Invalid id\n"); 
                continue; 
            }
            cmd_terminate(id);

        } else if (strcmp(cmd, "send") == 0) {
            char *idstr = strtok(NULL, " ");
            char *msg = strtok(NULL, ""); // lấy phần còn lại làm message
            if (!idstr || !msg) { 
                printf("[ERROR] Usage: send <id> <message>\n"); 
                continue; 
            }
            int id = atoi(idstr);
            if (id <= 0) { 
                printf("[ERROR] Invalid id\n"); 
                continue; 
            }
            cmd_send(id, msg);

        } else if (strcmp(cmd, "exit") == 0) {
            /* Gọi hàm cmd_exit() để đóng toàn bộ kết nối, giải phóng tài nguyên */
            cmd_exit();
            printf("[INFO] Goodbye.\n");
            return 0; // kết thúc chương trình

        } else {
            printf("[ERROR] Unknown command. Type 'help'\n");
        }
    }
    
    return 0;
}
