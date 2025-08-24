#ifndef COMMON_H
#define COMMON_H

#include <netinet/in.h> // Cấu trúc sockaddr_in cho IPv4
#include <pthread.h>    // Thư viện thread POSIX (pthread_create, mutex,...)

#define MAX_MSG_LEN 100   // Độ dài tối đa của 1 tin nhắn chat
#define BACKLOG 10        // Số lượng kết nối chờ (queue) tối đa trong listen()
#define BUFSIZE 512       // Kích thước bộ đệm dùng để nhận dữ liệu từ socket

/* 
   Cấu trúc dữ liệu quản lý 1 kết nối peer.
   Mỗi khi có kết nối mới (incoming hoặc outgoing), chương trình sẽ cấp phát 
   một PeerConn để lưu thông tin kết nối đó.
*/
typedef struct PeerConn {
    int id;                    // ID định danh kết nối (tăng dần, dùng để tham chiếu nhanh)
    int sockfd;                // Socket file descriptor liên quan đến peer
    char ip[INET_ADDRSTRLEN];  // Địa chỉ IP của peer (dạng chuỗi "xxx.xxx.xxx.xxx")
    int port;                  // Port số của peer (dạng int)
    pthread_t thread;          // Thread phục vụ cho kết nối này (lắng nghe tin nhắn từ peer)
    int active;                // Trạng thái hoạt động: 1 = còn hoạt động, 0 = đang đóng/kết thúc
    int should_remove;         // Đánh dấu: 1 => cần xoá khỏi danh sách quản lý (reaper thread sẽ xoá)
    struct PeerConn *next;     // Con trỏ tới kết nối tiếp theo (danh sách liên kết đơn để quản lý nhiều peer)
} PeerConn;


/* ==============================
   Các biến toàn cục (Globals)
   ============================== */

// Danh sách liên kết quản lý tất cả các kết nối (danh sách PeerConn).
// conn_head trỏ tới phần tử đầu tiên, các kết nối khác nối bằng trường "next".
extern PeerConn *conn_head;

// Mutex bảo vệ danh sách kết nối (conn_head).
// Vì nhiều thread (người nghe, người gửi, reaper) có thể thao tác danh sách 
// đồng thời, nên cần khoá mutex để tránh race condition.
extern pthread_mutex_t conn_mutex;

// Mutex bảo vệ in ra màn hình (printf).
// Nếu nhiều thread cùng gọi printf, output sẽ bị xen lẫn gây rối loạn.
// print_mutex đảm bảo chỉ có 1 thread in tại một thời điểm.
extern pthread_mutex_t print_mutex;

// Biến đếm tự động cấp phát ID cho kết nối mới.
// Mỗi khi có kết nối đến, giá trị này tăng lên để gán cho PeerConn->id.
extern int next_conn_id;

// Socket server chính (lắng nghe kết nối mới).
// Khi gọi socket(), bind(), listen(), ta lưu file descriptor này ở đây.
extern int server_sock;

// Port mà server sẽ lắng nghe (lấy từ tham số dòng lệnh).
extern int listen_port;

// Địa chỉ IP cục bộ (local IP) của máy chạy chương trình.
// Khởi tạo mặc định là "0.0.0.0" (nghĩa là listen trên tất cả các interface).
extern char local_ip[INET_ADDRSTRLEN];

// Thread đảm nhiệm việc lắng nghe kết nối từ peer khác.
// listener_thread_id sẽ lưu ID của thread này để quản lý (join/cancel).
extern pthread_t listener_thread_id;

// Cờ điều khiển vòng lặp chính của chương trình.
// running = 1 -> chương trình đang chạy; 
// running = 0 -> báo hiệu tất cả thread nên dừng và cleanup.
extern int running;

#endif
