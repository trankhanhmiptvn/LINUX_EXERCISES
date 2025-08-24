#include "common.h"
#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ==========================================
   Hàm tiện ích: safe_printf
   Mục đích: In ra màn hình an toàn trong môi trường đa luồng
   ========================================== */
void safe_printf(const char *fmt, ...) {
    va_list ap;

    // Khóa mutex để đảm bảo chỉ 1 thread in tại một thời điểm
    pthread_mutex_lock(&print_mutex);

    // Chuẩn bị danh sách tham số biến đổi (giống printf)
    va_start(ap, fmt);

    // In ra màn hình theo format truyền vào
    vprintf(fmt, ap);

    // Kết thúc xử lý tham số biến đổi
    va_end(ap);

    // Mở khóa mutex để thread khác có thể in
    pthread_mutex_unlock(&print_mutex);
}

/* 
   Hàm: get_local_ip
   Mục đích: Lấy IP cục bộ (local IP) của máy.
   Cách làm: tạo một UDP socket, "kết nối giả" tới 8.8.8.8:53,
             rồi dùng getsockname() để xem kernel chọn IP nào
             làm địa chỉ nguồn (source IP).
   Kết quả: ghi IP dạng chuỗi vào buf, trả về 0 nếu thành công, -1 nếu lỗi.
*/
int get_local_ip(char *buf, size_t buflen) {
    int sock;
    struct sockaddr_in serv;

    // Tạo một UDP socket (không cần kết nối thực sự).
    // Dùng AF_INET (IPv4), SOCK_DGRAM (UDP).
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1; // nếu tạo socket lỗi thì báo lỗi

    // Xoá sạch struct serv (địa chỉ server giả định)
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;                  // dùng IPv4
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");// chọn địa chỉ Google DNS
    serv.sin_port = htons(53);                  // cổng DNS (53)

    // "Kết nối giả" tới 8.8.8.8:53
    // Thực tế không gửi gói tin nào, chỉ để kernel quyết định IP nguồn.
    connect(sock, (struct sockaddr*)&serv, sizeof(serv));

    // Lấy địa chỉ nguồn (IP local mà kernel sẽ dùng cho socket này).
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) == -1) {
        // Nếu lỗi, đóng socket rồi trả -1
        close(sock);
        return -1;
    }

    // Chuyển địa chỉ IP nhị phân sang chuỗi "xxx.xxx.xxx.xxx"
    inet_ntop(AF_INET, &name.sin_addr, buf, buflen);

    // Đóng socket
    close(sock);

    // Thành công, trả về 0
    return 0;
}
