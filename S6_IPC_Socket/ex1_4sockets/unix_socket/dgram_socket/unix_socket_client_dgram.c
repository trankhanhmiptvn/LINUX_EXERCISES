#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "/tmp/unix_dgram_server"
#define CLIENT_PATH "/tmp/unix_dgram_client"
#define BUFFER_SIZE 256

// Hàm khởi tạo client socket
int init_client_socket(const char *client_path) {
    int sockfd;
    struct sockaddr_un client_addr;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(client_path);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, client_path, sizeof(client_addr.sun_path) - 1);

    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Hàm gửi tin nhắn cho server
void send_message(int sockfd, const char *server_path, const char *msg) {
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, server_path, sizeof(server_addr.sun_path) - 1);

    if (sendto(sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
    }
}

// Hàm nhận phản hồi từ server
void receive_response(int sockfd, char *buffer, size_t size) {
    ssize_t recv_len = recvfrom(sockfd, buffer, size - 1, 0, NULL, NULL);
    if (recv_len >= 0) {
        buffer[recv_len] = '\0';
        printf("Received from server: %s\n", buffer);
    } else {
        perror("recvfrom");
    }
}

int main() {
    int sockfd = init_client_socket(CLIENT_PATH);

    char buffer[BUFFER_SIZE];
    printf("Enter message to send: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Xóa newline

    send_message(sockfd, SERVER_PATH, buffer);
    receive_response(sockfd, buffer, BUFFER_SIZE);

    close(sockfd);
    unlink(CLIENT_PATH);
    return 0;
}
