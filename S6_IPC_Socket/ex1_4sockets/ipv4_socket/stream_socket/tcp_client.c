#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 6789
#define BUFFER_SIZE 1024

// Hàm tạo TCP client socket và kết nối đến server
int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Hàm gửi tin
ssize_t send_message(int sockfd, const char *msg) {
    return write(sockfd, msg, strlen(msg));
}

// Hàm nhận tin
ssize_t receive_message(int sockfd, char *buffer, size_t size) {
    ssize_t len = read(sockfd, buffer, size - 1);
    if (len >= 0) buffer[len] = '\0';
    return len;
}

int main() {
    int sockfd = connect_to_server(SERVER_IP, SERVER_PORT);
    printf("Connected to server. Type messages (Ctrl+D to quit):\n");

    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        send_message(sockfd, buffer);

        ssize_t bytes_read = receive_message(sockfd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) break;

        printf("From server: %s", buffer);
    }

    printf("\nDisconnected.\n");
    close(sockfd);
    return 0;
}
