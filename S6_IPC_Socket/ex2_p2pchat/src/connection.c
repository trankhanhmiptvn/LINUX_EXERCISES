#include "utils.h"
#include "connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>      // Quản lý mã lỗi hệ thống (errno, strerror)
#include <arpa/inet.h>

PeerConn *conn_head = NULL;
pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_conn_id = 1;

/* 
   Hàm: add_connection
   Mục đích: Thêm một kết nối mới (PeerConn) vào danh sách toàn cục conn_head.
   Điều kiện: socket đã kết nối thành công và peer_addr chứa thông tin đối phương.
   Trả về: con trỏ tới PeerConn vừa tạo, hoặc NULL nếu lỗi malloc.
*/
PeerConn* add_connection(int sockfd, struct sockaddr_in *peer_addr) {
    // Cấp phát bộ nhớ cho một PeerConn mới
    PeerConn *p = malloc(sizeof(PeerConn));
    if (!p) return NULL;  // nếu malloc lỗi, thoát ngay

    // Gán ID duy nhất cho kết nối.
    // __sync_fetch_and_add là atomic operation (thread-safe),
    // đảm bảo nhiều thread tăng next_conn_id không bị xung đột.
    p->id = __sync_fetch_and_add(&next_conn_id, 1);

    // Lưu socket file descriptor của kết nối
    p->sockfd = sockfd;

    // Chuyển địa chỉ IP dạng nhị phân -> chuỗi “x.x.x.x” lưu vào p->ip
    inet_ntop(AF_INET, &peer_addr->sin_addr, p->ip, sizeof(p->ip));

    // Lưu port của peer (network byte order -> host byte order)
    p->port = ntohs(peer_addr->sin_port);

    // Đặt trạng thái ban đầu
    p->active = 1;         // kết nối đang hoạt động
    p->should_remove = 0;  // chưa cần xóa
    p->next = NULL;        // chưa nối với ai trong list

    // Khóa mutex để bảo vệ danh sách conn_head (global shared list)
    pthread_mutex_lock(&conn_mutex);

    // Thêm vào đầu danh sách liên kết (insert head)
    p->next = conn_head;
    conn_head = p;

    // Mở khóa mutex
    pthread_mutex_unlock(&conn_mutex);

    // Trả về con trỏ đến kết nối vừa thêm
    return p;
}

/* 
   Tìm kết nối trong danh sách theo ID.
   Lưu ý: caller (hàm gọi) phải giữ conn_mutex 
   hoặc chỉ dùng con trỏ copy cẩn thận để tránh race condition.
*/
PeerConn* find_connection_by_id(int id) {
    PeerConn *cur = conn_head;   // bắt đầu từ đầu danh sách
    while (cur) {                // duyệt tuần tự linked list
        if (cur->id == id)       // nếu ID khớp thì trả về con trỏ kết nối
            return cur;
        cur = cur->next;         // duyệt tiếp
    }
    return NULL;                 // không tìm thấy
}

/* 
 * Thread function: lắng nghe và nhận dữ liệu từ một peer (một kết nối TCP).
 * - Mỗi kết nối sẽ có một thread riêng để xử lý việc nhận dữ liệu.
 * - Thread này chạy vòng lặp cho đến khi peer ngắt kết nối, gửi lệnh disconnect,
 *   hoặc có lỗi xảy ra.
 * - Khi kết thúc, thread đánh dấu peer này là "inactive" và "cần xóa".
 */
void* conn_recv_thread(void *arg) {
    // Ép kiểu con trỏ arg thành PeerConn (được truyền khi tạo thread).
    // PeerConn chứa thông tin về kết nối: socket, ip, port, trạng thái,...
    PeerConn *conn = (PeerConn*)arg;

    char buf[BUFSIZE];    // buffer tạm để chứa dữ liệu nhận về
    ssize_t n;            // số byte thực sự đọc được từ recv()

    // Vòng lặp chính: liên tục nhận dữ liệu từ peer
    while (1) {
        // Gọi recv() để đọc dữ liệu từ socket của peer.
        // Tham số: conn->sockfd = socket TCP, buf = bộ đệm,
        // sizeof(buf)-1 để chừa chỗ cho '\0'.
        n = recv(conn->sockfd, buf, sizeof(buf)-1, 0);

        // Trường hợp 1: nhận được dữ liệu hợp lệ (n > 0)
        if (n > 0) {
            buf[n] = '\0';  // Đảm bảo buffer kết thúc bằng ký tự null (chuỗi an toàn)

            // Kiểm tra xem tin nhắn có phải lệnh đặc biệt "CMD:DISCONNECT" không
            if (strncmp(buf, "CMD:DISCONNECT", 14) == 0) {
                safe_printf("\n[INFO] Peer %s:%d requested disconnect.\n", 
                            conn->ip, conn->port);
                break;  // thoát khỏi vòng lặp => ngắt kết nối
            } else {
                // Nếu chỉ là tin nhắn thông thường, in nó ra màn hình
                safe_printf("\nMessage from %s:%d: %s\n", 
                            conn->ip, conn->port, buf);
            }
        } 
        // Trường hợp 2: peer đã đóng kết nối (n == 0)
        else if (n == 0) {
            // Điều này nghĩa là peer đã gửi gói FIN và kết thúc TCP session
            safe_printf("\n[INFO] Peer %s:%d closed the connection.\n", 
                        conn->ip, conn->port);
            break;  // thoát vòng lặp
        } 
        // Trường hợp 3: xảy ra lỗi khi đọc dữ liệu (n < 0)
        else {
            if (errno == EINTR) {
                // Nếu lỗi do bị ngắt bởi tín hiệu (signal interrupt),
                // thì không coi là lỗi thật, tiếp tục vòng lặp để thử recv() lại.
                continue;
            }
            // Các lỗi khác: in ra thông báo và thoát.
            safe_printf("\n[ERROR] recv() from %s:%d failed (%s)\n", 
                        conn->ip, conn->port, strerror(errno));
            break;
        }
    }

    /*
     * Khi vòng lặp kết thúc (ngắt kết nối, lỗi, hoặc disconnect command),
     * cần đánh dấu peer này là không còn hoạt động.
     * Không free ngay ở đây để tránh race-condition,
     * chỉ gắn cờ should_remove để thread quản lý khác xử lý cleanup sau.
     */
    pthread_mutex_lock(&conn_mutex);
    conn->active = 0;         // peer này đã không còn hoạt động
    conn->should_remove = 1;  // yêu cầu main thread hoặc cleanup thread xóa nó khỏi danh sách
    pthread_mutex_unlock(&conn_mutex);

    return NULL;  // Thread kết thúc
}

/*
   Xóa một node (PeerConn) ra khỏi danh sách và giải phóng bộ nhớ.
   Yêu cầu: caller đã giữ conn_mutex trước khi gọi.
   prev: node đứng trước (nếu có)
   cur: node cần xóa
*/
void remove_node_locked(PeerConn *prev, PeerConn *cur) {
    if (!cur) return;  // nếu node cần xóa rỗng thì thoát

    if (prev) 
        prev->next = cur->next;   // nếu không phải head, nối prev với cur->next
    else 
        conn_head = cur->next;    // nếu là head thì cập nhật conn_head

    free(cur);  // giải phóng bộ nhớ node đã xóa
}


/* 
 * reap_connections()
 * ------------------
 * - Chạy định kỳ (thường từ main loop).
 * - Nhiệm vụ: Dọn dẹp (reap) các kết nối đã kết thúc (should_remove = 1).
 * - Thao tác gồm:
 *   1. Join thread của kết nối (đợi thread kết thúc để tránh zombie thread).
 *   2. Xóa PeerConn đó khỏi danh sách liên kết toàn cục (conn_head).
 *   3. Giải phóng bộ nhớ liên quan.
 *
 * Cơ chế: cần lock/unlock conn_mutex để tránh race condition 
 *         khi sửa danh sách kết nối trong lúc các thread khác chạy.
 */
void reap_connections() {
    pthread_mutex_lock(&conn_mutex);

    PeerConn *prev = NULL;    // node trước (để xóa node khi cần)
    PeerConn *cur  = conn_head; // duyệt từ node đầu tiên

    while (cur) {
        if (cur->should_remove) {
            // Nếu kết nối này được đánh dấu cần xóa

            pthread_mutex_unlock(&conn_mutex);
            // ⚠️ Mở khóa trước khi gọi pthread_join
            // Vì pthread_join có thể block lâu (chờ thread con kết thúc).
            // Nếu giữ lock thì các thread khác không thể truy cập conn_head → deadlock.

            pthread_join(cur->thread, NULL);
            // Đợi thread nhận dữ liệu (conn_recv_thread) của kết nối này kết thúc.
            // Sau lệnh này, thread đó chắc chắn đã thoát.

            pthread_mutex_lock(&conn_mutex);
            // Sau khi join xong, lock lại để xóa node khỏi danh sách.

            // Tìm lại node cur trong danh sách (vì trong lúc unlock, 
            // danh sách có thể đã thay đổi do thread khác).
            PeerConn *scan = conn_head, *scan_prev = NULL;
            while (scan && scan != cur) {
                scan_prev = scan;
                scan = scan->next;
            }

            if (scan == cur) {
                // Nếu tìm thấy node cur trong danh sách thì xóa nó
                remove_node_locked(scan_prev, scan);
                safe_printf("[INFO] Connection id=%d removed.\n", cur->id);
            }

            // Reset lại việc duyệt danh sách (bắt đầu từ đầu danh sách).
            prev = NULL;
            cur  = conn_head;
            continue; // Quay lại while (cur)
        }

        // Nếu kết nối hiện tại chưa cần xóa → sang node tiếp theo
        prev = cur;
        cur  = cur->next;
    }

    pthread_mutex_unlock(&conn_mutex);
}

/* ============================================================
 * Validate IPv4 string (ví dụ: "192.168.1.10")
 * Trả về 1 nếu hợp lệ, 0 nếu không hợp lệ.
 * - Dùng inet_pton(AF_INET, ...) để parse IPv4.
 * - Không tạo socket, không truy cập mạng, chỉ kiểm tra cú pháp/chuyển đổi.
 * ============================================================ */
int is_valid_ipv4(const char *ip) {
    if (ip == NULL || *ip == '\0') return 0;   // chuỗi rỗng -> không hợp lệ
    struct in_addr addr;                       // vùng nhận kết quả parse
    // inet_pton trả 1 nếu parse OK, 0 nếu chuỗi sai định dạng, -1 nếu AF không hỗ trợ
    return inet_pton(AF_INET, ip, &addr) == 1;
}
