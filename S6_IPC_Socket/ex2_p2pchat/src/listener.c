#include "common.h"
#include "listener.h"
#include "connection.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>      // Quản lý mã lỗi hệ thống (errno, strerror)
#include <arpa/inet.h>
#include <stdlib.h>

int server_sock = -1;
int listen_port = 0;
char local_ip[INET_ADDRSTRLEN] = "0.0.0.0";
pthread_t listener_thread_id;
int running = 1;

/* 
 * Thread "listener" (luồng lắng nghe kết nối mới).
 * - Chạy vòng lặp gọi accept() trên socket server.
 * - Khi có client (peer) mới kết nối, tạo PeerConn để quản lý.
 * - Gắn vào danh sách kết nối toàn cục.
 * - Sinh ra một thread "conn_recv_thread" cho kết nối đó để xử lý nhận dữ liệu.
 * 
 * Kết thúc khi biến toàn cục "running" = false (server shutdown).
 */
void* listener_thread(void *arg) {
    // Socket server (đã bind() và listen()) được truyền từ main thread
    int srv = *(int*)arg;

    // Vòng lặp chính: chờ và nhận kết nối mới
    while (running) {
        struct sockaddr_in peer;   // thông tin địa chỉ của peer kết nối vào
        socklen_t plen = sizeof(peer);

        // accept(): chờ một kết nối mới từ client
        int newfd = accept(srv, (struct sockaddr*)&peer, &plen);

        // Xử lý khi accept() thất bại
        if (newfd < 0) {
            if (!running) break; // Nếu server đang shutdown thì thoát vòng lặp
            safe_printf("[ERROR] accept failed: %s\n", strerror(errno));
            continue; // Tiếp tục vòng lặp để chờ accept() lần sau
        }

        /* Thêm peer mới vào danh sách kết nối toàn cục */
        PeerConn *p = add_connection(newfd, &peer);
        if (!p) {
            // Nếu tạo PeerConn thất bại (hết RAM, lỗi init,…)
            safe_printf("[ERROR] add_connection failed, closing socket\n");
            close(newfd);
            continue; // Bỏ qua kết nối này
        }

        /* Tạo thread để nhận dữ liệu từ peer */
        if (pthread_create(&p->thread, NULL, conn_recv_thread, p) != 0) {
            // Nếu tạo thread thất bại
            safe_printf("[ERROR] pthread_create failed for new connection\n");
            close(newfd);

            // Xóa node p khỏi danh sách (vì ta vừa chèn vào trước đó)
            pthread_mutex_lock(&conn_mutex);
            if (conn_head == p) {
                // Nếu p đang là node đầu tiên
                conn_head = p->next;
            } else {
                // Nếu p không phải head: tìm node trước nó rồi bỏ qua
                PeerConn *cur = conn_head;
                while (cur && cur->next != p) cur = cur->next;
                if (cur) cur->next = p->next;
            }
            free(p); // giải phóng PeerConn
            pthread_mutex_unlock(&conn_mutex);

            continue; // quay lại vòng lặp accept()
        }

        // Nếu mọi thứ OK: báo đã nhận kết nối mới thành công
        safe_printf("[INFO] Accepted connection id=%d from %s:%d\n", 
                    p->id, p->ip, p->port);
    }

    // Khi thoát khỏi vòng lặp (server shutdown), thread listener kết thúc
    return NULL;
}
