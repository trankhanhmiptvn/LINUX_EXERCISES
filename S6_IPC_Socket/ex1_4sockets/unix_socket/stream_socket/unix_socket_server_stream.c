#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUFFER_SIZE 1024

// Hàm khởi tạo server socket
int init_server_socket(const char *path, int backlog) {
    int server_fd;
    struct sockaddr_un addr;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Xóa file cũ (nếu có)
    unlink(path);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, backlog) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

// Hàm accept client
int accept_client(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return client_fd;
}

// Hàm gửi
ssize_t send_message(int fd, const char *msg) {
    return write(fd, msg, strlen(msg));
}

// Hàm nhận
ssize_t receive_message(int fd, char *buffer, size_t size) {
    ssize_t len = read(fd, buffer, size - 1);
    if (len >= 0) buffer[len] = '\0';
    return len;
}

int main() {
    int server_fd = init_server_socket(SOCKET_PATH, 5);

    printf("Server listening at %s...\n", SOCKET_PATH);

    int client_fd = accept_client(server_fd);
    printf("Client connected!\n");

    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t num_read = receive_message(client_fd, buffer, BUFFER_SIZE);
        if (num_read == 0) {
            printf("Client disconnected.\n");
            break;
        }
        if (num_read < 0) {
            perror("read");
            break;
        }

        printf("Received from client: %s", buffer);

        char response[BUFFER_SIZE +18];
        snprintf(response, BUFFER_SIZE +18, "Server received: %s", buffer);
        send_message(client_fd, response);
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
