#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "/tmp/unix_dgram_server"
#define BUFFER_SIZE 256

// Hàm khởi tạo server socket DGRAM
int init_server_socket(const char *server_path) {
    int sockfd;
    struct sockaddr_un server_addr;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(server_path); // xóa socket file cũ nếu có

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, server_path, sizeof(server_addr.sun_path) - 1);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Hàm nhận tin nhắn từ client
ssize_t receive_message(int sockfd, char *buffer, size_t size,
                        struct sockaddr_un *client_addr, socklen_t *client_len) {
    ssize_t recv_len = recvfrom(sockfd, buffer, size - 1, 0,
                                (struct sockaddr *)client_addr, client_len);
    if (recv_len < 0) {
        perror("recvfrom");
        return -1;
    }
    buffer[recv_len] = '\0'; // đảm bảo chuỗi kết thúc
    return recv_len;
}

// Hàm gửi phản hồi lại cho client
void send_response(int sockfd, const char *msg,
                   struct sockaddr_un *client_addr, socklen_t client_len) {
    if (sendto(sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)client_addr, client_len) < 0) {
        perror("sendto");
    }
}

int main() {
    int sockfd = init_server_socket(SERVER_PATH);
    printf("Server is running and waiting for messages from client...\n");

    char buffer[BUFFER_SIZE];
    struct sockaddr_un client_addr;
    socklen_t client_len;

    while (1) {
        client_len = sizeof(client_addr);

        // Nhận message từ client
        ssize_t recv_len = receive_message(sockfd, buffer, BUFFER_SIZE,
                                           &client_addr, &client_len);
        if (recv_len <= 0) continue;

        printf("Received: %s\n", buffer);

        if (client_addr.sun_path[0] != '\0') {
            printf("Client address (sun_path): %s\n", client_addr.sun_path);
        } else {
            printf("WARNING: Client address is unnamed or abstract.\n");
        }

        // Chuẩn bị phản hồi
        char response[BUFFER_SIZE + 32];
        snprintf(response, BUFFER_SIZE + 32, "Server received: %s", buffer);

        // Gửi lại cho client
        send_response(sockfd, response, &client_addr, client_len);
    }

    close(sockfd);
    unlink(SERVER_PATH);
    return 0;
}
