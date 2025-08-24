#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 6789
#define BUFFER_SIZE 1024

// Hàm tạo socket và bind vào địa chỉ
int create_server_socket(const char *ip, int port) {
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

// Hàm chấp nhận client
int accept_client(int server_fd, char *client_ip, int *client_port) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        return -1;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    *client_port = ntohs(client_addr.sin_port);

    return client_fd;
}

// Hàm echo (nhận và gửi lại)
void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Received from client: %s\n", buffer);

        char response[BUFFER_SIZE+17];
        snprintf(response, BUFFER_SIZE+17, "Server receive: %s", buffer);
        write(client_fd, response, strlen(response));
    }

    printf("Client disconnected.\n");
    close(client_fd);
}

int main() {
    int server_fd = create_server_socket("127.0.0.1", PORT);
    printf("Server listening on port %d...\n", PORT);

    char client_ip[INET_ADDRSTRLEN];
    int client_port;

    int client_fd = accept_client(server_fd, client_ip, &client_port);
    if (client_fd >= 0) {
        printf("Client connected: IP = %s, Port = %d\n", client_ip, client_port);
        handle_client(client_fd);
    }

    close(server_fd);
    return 0;
}
