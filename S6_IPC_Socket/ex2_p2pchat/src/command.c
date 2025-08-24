#include "command.h"
#include "common.h"
#include "connection.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>      // Quản lý mã lỗi hệ thống (errno, strerror)
#include <stdlib.h>
#include <arpa/inet.h>  // Hàm thao tác địa chỉ IP (inet_ntop, htons,...)
#include <unistd.h>     // Hàm hệ thống POSIX (close, read, write,...)

/* 
 * Hàm cmd_help: hiển thị danh sách các lệnh có thể dùng trong chương trình chat
 * Mục đích: giúp người dùng biết các lệnh và cú pháp sử dụng
 */
void cmd_help() {
    printf("\n**********************************************\n");
    printf("                Chat Application              \n");
    printf("**********************************************\n");
    printf("Use the commands below:\n\n");

    printf(" 1. help");
    printf("      : Display user interface options\n\n");

    printf(" 2. myip");
    printf("      : Display IP address of this app\n\n");

    printf(" 3. myport");
    printf("      : Display listening port of this app \n\n");

    printf(" 4. connect <destination_ip> <port>");
    printf("      : Connect to the app of another computer\n\n");

    printf(" 5. list");
    printf("      : List all the connections of this app\n\n");

    printf(" 6. terminate <connection_id>");
    printf("      : Terminate a connection\n\n");

    printf(" 7. send <connection_id> <message>");
    printf("      : Send a message to a connection \n\n");

    printf(" 8. exit");
    printf("      : Close all connections & terminate the app\n\n");

    printf("**********************************************\n\n");
}


/* ======================================================================================
 * cmd_connect:
 *   - Thiết lập kết nối TCP tới peer có địa chỉ (ip, port).
 *   - Kiểm tra hợp lệ: IPv4 hợp lệ, port 1..65535, không tự kết nối vào chính mình,
 *     không trùng kết nối đang active.
 *   - Nếu connect() thành công:
 *       + add_connection() để đưa vào danh sách quản lý
 *       + tạo thread nhận dữ liệu (conn_recv_thread) cho kết nối này
 *   - Mọi lỗi đều được log rõ ràng; đảm bảo đóng socket để tránh rò rỉ tài nguyên.
 *
 * Tham chiếu biến toàn cục:
 *   - local_ip, listen_port: dùng để phát hiện *self-connect*.
 *   - conn_head, conn_mutex: dùng để phát hiện *duplicate connection*.
 * ====================================================================================== */
void cmd_connect(const char *ip, const char *portstr) {
    /* 1) Kiểm tra IPv4 hợp lệ */
    if (!is_valid_ipv4(ip)) {
        safe_printf("[ERROR] Invalid IPv4 address: %s\n", ip ? ip : "(null)");
        return;
    }

    /* 2) Parse và kiểm tra port bằng strtol (an toàn hơn atoi) */
    char *endp = NULL;
    long port_l = strtol(portstr, &endp, 10);
    if (portstr == NULL || *portstr == '\0' || *endp != '\0' ||
        port_l <= 0 || port_l > 65535) {
        safe_printf("[ERROR] Invalid port: %s (expected 1..65535)\n",
                    portstr ? portstr : "(null)");
        return;
    }
    int port = (int)port_l;

    /* 3) Chặn tự kết nối (self-connect):
     *    - cùng IP và đúng port đang listen
     *    - trường hợp localhost cũng được chặn (127.0.0.1)
     */
    if ((strcmp(ip, local_ip) == 0 || strcmp(ip, "127.0.0.1") == 0) &&
        port == listen_port) {
        safe_printf("[ERROR] Attempt to connect to self is not allowed.\n");
        return;
    }

    /* 4) Kiểm tra trùng kết nối đang active (duplicate) */
    pthread_mutex_lock(&conn_mutex);
    for (PeerConn *c = conn_head; c; c = c->next) {
        if (c->active && c->port == port && strcmp(c->ip, ip) == 0) {
            pthread_mutex_unlock(&conn_mutex);
            safe_printf("[ERROR] Connection already exists to %s:%d (id=%d)\n",
                        ip, port, c->id);
            return;
        }
    }
    pthread_mutex_unlock(&conn_mutex);

    /* 5) Tạo TCP socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        safe_printf("[ERROR] socket() failed: %s\n", strerror(errno));
        return;
    }

    /* 6) Chuẩn bị sockaddr_in đích (peer) */
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    // inet_pton đã được is_valid_ipv4 xác thực; ở đây chắc chắn thành công
    if (inet_pton(AF_INET, ip, &dest.sin_addr) != 1) {
        safe_printf("[ERROR] inet_pton() unexpectedly failed for %s\n", ip);
        close(sock);
        return;
    }
    dest.sin_port = htons(port);

    /* 7) Gọi connect() để thiết lập kết nối TCP */
    if (connect(sock, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
        safe_printf("[ERROR] connect() to %s:%d failed: %s\n",
                    ip, port, strerror(errno));
        close(sock);
        return;
    }

    /* 8) Thêm vào danh sách kết nối (đã kết nối thành công) */
    PeerConn *p = add_connection(sock, &dest);
    if (!p) {
        safe_printf("[ERROR] add_connection() failed (out of memory?)\n");
        close(sock);
        return;
    }

    /* 9) Tạo thread nhận dữ liệu cho kết nối này */
    if (pthread_create(&p->thread, NULL, conn_recv_thread, p) != 0) {
        safe_printf("[ERROR] pthread_create() failed for new connection\n");
        // rollback: đóng socket và gỡ node p khỏi danh sách để không rò rỉ
        close(sock);
        pthread_mutex_lock(&conn_mutex);
        if (conn_head == p) {
            conn_head = p->next;
        } else {
            PeerConn *cur = conn_head;
            while (cur && cur->next != p) cur = cur->next;
            if (cur) cur->next = p->next;
        }
        free(p);
        pthread_mutex_unlock(&conn_mutex);
        return;
    }

    /* 10) Thành công */
    safe_printf("[INFO] Connected to %s:%d with id=%d\n", p->ip, p->port, p->id);
}

/* ================================
 *  Lệnh "list"
 *  Mục đích: Liệt kê tất cả kết nối đang hoạt động với ID, địa chỉ IP, và Port
 * ================================ */
void cmd_list() {
    // Khóa mutex để tránh xung đột khi đọc danh sách kết nối
    pthread_mutex_lock(&conn_mutex);

    // In tiêu đề bảng kết nối
    printf("ID\tIP\t\tPort\n");

    // Duyệt danh sách liên kết các kết nối
    PeerConn *cur = conn_head;
    while (cur) {
        // Chỉ in ra các kết nối đang hoạt động (active = 1)
        if (cur->active)
            printf("%d\t%s\t%d\n", cur->id, cur->ip, cur->port);
        cur = cur->next;
    }

    // Giải phóng khóa
    pthread_mutex_unlock(&conn_mutex);

    // Gom rác các kết nối đã đóng nhưng chưa được loại bỏ khỏi danh sách
    reap_connections();
}

/* ================================
 *  Lệnh "terminate <id>"
 *  Mục đích: Ngắt và xóa kết nối có ID cụ thể
 * ================================ */
void cmd_terminate(int id) {
    // Khóa mutex để tìm kết nối cần hủy
    pthread_mutex_lock(&conn_mutex);

    PeerConn *cur = conn_head, *prev = NULL;

    // Tìm kết nối theo ID
    while (cur) {
        if (cur->id == id) break;
        prev = cur;
        cur = cur->next;
    }

    // Nếu không tìm thấy thì báo lỗi
    if (!cur) {
        pthread_mutex_unlock(&conn_mutex);
        printf("[ERROR] Connection id %d not found.\n", id);
        return;
    }

    // Gửi thông báo ngắt kết nối cho peer (best-effort, không chắc thành công)
    send(cur->sockfd, "CMD:DISCONNECT", strlen("CMD:DISCONNECT"), 0);

    // Đóng socket (shutdown để cắt cả chiều đọc/ghi)
    shutdown(cur->sockfd, SHUT_RDWR);
    close(cur->sockfd);

    // Đánh dấu kết nối không còn hoạt động
    cur->active = 0;

    // Giải phóng mutex trước khi join thread (tránh deadlock)
    pthread_mutex_unlock(&conn_mutex);

    // Chờ thread nhận dữ liệu từ kết nối này kết thúc
    pthread_join(cur->thread, NULL);

    // Khóa lại mutex để loại bỏ node khỏi danh sách
    pthread_mutex_lock(&conn_mutex);

    // Xóa kết nối ra khỏi danh sách liên kết
    PeerConn *scan = conn_head, *scan_prev = NULL;
    while (scan && scan != cur) { 
        scan_prev = scan; 
        scan = scan->next; 
    }

    if (scan == cur) {
        remove_node_locked(scan_prev, scan);
        printf("[INFO] Connection id %d terminated and removed.\n", id);
    }

    pthread_mutex_unlock(&conn_mutex);
}


/* ================================
 * send command
 * Gửi tin nhắn 'msg' tới kết nối có ID = id.
 * - Kiểm soát độ dài msg
 * - Tìm kết nối trong danh sách (có khóa conn_mutex)
 * - Gửi bằng send()
 * - In kết quả
 * ================================ */
void cmd_send(int id, const char *msg) {
    // 1) Chặn tin nhắn quá dài (đảm bảo yêu cầu đề bài)
    if (strlen(msg) > MAX_MSG_LEN) {
        printf("[ERROR] Message too long (max %d chars).\n", MAX_MSG_LEN);
        return; // Không gửi
    }

    // 2) Khóa danh sách kết nối để tìm đúng peer theo id và đang active
    pthread_mutex_lock(&conn_mutex);
    PeerConn *cur = conn_head;            // bắt đầu từ đầu danh sách
    while (cur) {                         // duyệt tuần tự
        if (cur->id == id && cur->active) // tìm đúng ID và còn hoạt động
            break;
        cur = cur->next;                  // sang node tiếp theo
    }

    // 3) Nếu không thấy kết nối phù hợp -> báo lỗi
    if (!cur) {
        pthread_mutex_unlock(&conn_mutex);  // mở khóa trước khi trả về
        printf("[ERROR] Connection id %d not found or not active.\n", id);
        return;
    }

    // 4) Thực hiện gửi tin nhắn:
    //    (Ở phiên bản gốc đang giữ conn_mutex khi gọi send()
    //     → đảm bảo 'cur' không bị xóa trong lúc gửi, nhưng có thể làm block list)
    ssize_t sent = send(cur->sockfd, msg, strlen(msg), 0);

    // 5) Mở khóa danh sách sau khi gửi xong
    pthread_mutex_unlock(&conn_mutex);

    // 6) Kiểm tra kết quả gửi
    if (sent < 0) {
        printf("[ERROR] send failed: %s\n", strerror(errno));
    } else {
        // In thông tin tóm tắt: id, địa chỉ peer và nội dung đã gửi
        printf("[INFO] Sent to id=%d (%s:%d): %s\n", id, cur->ip, cur->port, msg);
    }
}

/* ============================================================
 * exit command
 * Thoát ứng dụng một cách "gọn gàng":
 *  - Báo hiệu dừng (running = 0)
 *  - Đóng socket lắng nghe để accept() thoát
 *  - Gửi CMD:DISCONNECT tới các peer, shutdown/close socket
 *  - Đợi (join) listener thread
 *  - Join tất cả recv-thread của các kết nối và free bộ nhớ
 * ============================================================ */
void cmd_exit() {
    // 1) Thông báo bắt đầu quá trình thoát
    safe_printf("[INFO] Exiting: notifying peers and closing connections...\n");

    // 2) Đặt cờ dừng cho các vòng lặp phụ thuộc 'running'
    running = 0;

    // 3) Đóng server socket để 'accept()' trong listener_thread bị phá vỡ và kết thúc
    if (server_sock >= 0) close(server_sock);

    // 4) Khóa danh sách, lần lượt đóng tất cả kết nối đang active
    pthread_mutex_lock(&conn_mutex);
    PeerConn *cur = conn_head;
    while (cur) {
        if (cur->active) {
            // Gửi lệnh ngắt kết nối cho peer (best-effort)
            send(cur->sockfd, "CMD:DISCONNECT", strlen("CMD:DISCONNECT"), 0);

            // Cắt cả 2 chiều đọc/ghi rồi đóng socket
            shutdown(cur->sockfd, SHUT_RDWR);
            close(cur->sockfd);

            // Đánh dấu không còn hoạt động; recv-thread sẽ thoát
            cur->active = 0;

            // Sang node tiếp theo
            cur = cur->next;
        } else {
            // Nếu node đã không active thì chỉ đi tiếp
            cur = cur->next;
        }
    }
    pthread_mutex_unlock(&conn_mutex);

    // 5) Join listener thread để đảm bảo thread lắng nghe đã kết thúc hẳn
    pthread_join(listener_thread_id, NULL);

    // 6) Join tất cả các thread nhận dữ liệu của từng kết nối và giải phóng bộ nhớ
    //    (Phiên bản gốc giữ conn_mutex trong lúc join → có nguy cơ deadlock trong một số thiết kế;
    //     ở đây ta làm theo code gốc nhưng lưu ý điều đó ở phần "Lưu ý" bên dưới.)
    pthread_mutex_lock(&conn_mutex);
    cur = conn_head;
    while (cur) {
        // Đợi thread của kết nối này kết thúc (tránh rò rỉ tài nguyên)
        pthread_join(cur->thread, NULL);

        // Free từng node: lưu tạm con trỏ để duyệt tiếp
        PeerConn *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    // Xóa hẳn danh sách
    conn_head = NULL;
    pthread_mutex_unlock(&conn_mutex);
}