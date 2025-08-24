#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6789
#define BUFFER_SIZE 1024

// Tạo UDP socket
int create_udp_client_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Thiết lập địa chỉ server
void setup_server_address(struct sockaddr_in *server_addr, const char *ip, int port) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port   = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr->sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
}

// Gửi message tới server
void send_message(int sockfd, const char *message, struct sockaddr_in *server_addr) {
    sendto(sockfd, message, strlen(message), 0,
           (struct sockaddr *)server_addr, sizeof(*server_addr));
}

// Nhận message từ server
ssize_t receive_message(int sockfd, char *buffer, struct sockaddr_in *server_addr) {
    socklen_t server_len = sizeof(*server_addr);
    ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                      (struct sockaddr *)server_addr, &server_len);
    if (bytes_received < 0) {
        perror("recvfrom");
        return -1;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tạo socket client
    sockfd = create_udp_client_socket();

    // Thiết lập địa chỉ server
    setup_server_address(&server_addr, SERVER_IP, SERVER_PORT);

    printf("Connected to UDP server. Type messages (Ctrl+D to quit):\n");

    // Vòng lặp gửi/nhận
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        send_message(sockfd, buffer, &server_addr);

        if (receive_message(sockfd, buffer, &server_addr) > 0) {
            printf("From server: %s", buffer);
        }
    }

    printf("\nDisconnected.\n");
    close(sockfd);
    return 0;
}
