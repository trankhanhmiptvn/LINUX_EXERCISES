#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUFFER_SIZE 1024

// Hàm khởi tạo client socket STREAM
int init_client_socket(const char *server_path) {
    int sockfd;
    struct sockaddr_un addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, server_path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Hàm gửi message
ssize_t send_message(int sockfd, const char *msg) {
    return write(sockfd, msg, strlen(msg));
}

// Hàm nhận message
ssize_t receive_message(int sockfd, char *buffer, size_t size) {
    ssize_t len = read(sockfd, buffer, size - 1);
    if (len >= 0) buffer[len] = '\0';
    return len;
}

int main() {
    int sockfd = init_client_socket(SOCKET_PATH);

    char buffer[BUFFER_SIZE];

    while (1) {
        printf("Enter message to send (Ctrl+D to quit): ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;  // EOF

        send_message(sockfd, buffer);

        ssize_t num_read = receive_message(sockfd, buffer, BUFFER_SIZE);
        if (num_read <= 0) {
            printf("Server closed connection.\n");
            break;
        }

        printf("Response from server: %s", buffer);
    }

    close(sockfd);
    return 0;
}
