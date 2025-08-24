#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 6789
#define BUFFER_SIZE 1024

// Hàm tạo và bind UDP socket
int create_udp_server_socket(const char *ip, int port, struct sockaddr_in *server_addr) {
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port   = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr->sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Hàm nhận message từ client
ssize_t receive_message(int sockfd, char *buffer, struct sockaddr_in *client_addr, socklen_t *client_len) {
    ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                      (struct sockaddr *)client_addr, client_len);
    if (bytes_received < 0) {
        perror("recvfrom");
        return -1;
    }

    buffer[bytes_received] = '\0';
    return bytes_received;
}

// Hàm gửi response lại client
void send_response(int sockfd, const char *message, struct sockaddr_in *client_addr, socklen_t client_len) {
    sendto(sockfd, message, strlen(message), 0,
           (struct sockaddr *)client_addr, client_len);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // Tạo UDP server socket
    int sockfd = create_udp_server_socket("127.0.0.1", PORT, &server_addr);
    printf("UDP Server listening on port %d...\n", PORT);

    // Vòng lặp nhận/gửi
    while (1) {
        client_len = sizeof(client_addr);

        ssize_t bytes_received = receive_message(sockfd, buffer, &client_addr, &client_len);
        if (bytes_received < 0) continue;

        // Hiển thị client IP và port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("Received from client: IP = %s, Port = %d\n", client_ip, ntohs(client_addr.sin_port));
        printf("Message: %s\n", buffer);

        // Chuẩn bị và gửi response
        char response[BUFFER_SIZE+18];
        snprintf(response, BUFFER_SIZE+18, "Server received: %s", buffer);
        send_response(sockfd, response, &client_addr, client_len);
    }

    close(sockfd);
    return 0;
}
