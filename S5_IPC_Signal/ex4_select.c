#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_TIME 10
volatile  sig_atomic_t g_sigint = 0;
volatile  sig_atomic_t g_sigterm = 0;
void handle_signal (int sig) {
    if(sig == SIGINT) {
        g_sigint = 1;
    }
    else if(sig == SIGTERM) {
        g_sigterm = 1;
    }
}
    
int main() {
    fd_set readfds;
    char buffer[256];

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    printf("Program is running. Please send SIGINT, SIGTERM or mesage.\n");
    while(1) { 
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                if (g_sigint) {
                    g_sigint = 0;
                    printf("SIGINT received.\n");
                }
                if (g_sigterm) {
                    printf("SIGTERM received. Exiting...\n");
                    break;
                }
                continue; // quay lại vòng lặp
            } else {
                perror("select");
                exit(EXIT_FAILURE);
            }
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                buffer[strcspn(buffer, "\n")] = '\0';
                printf("You inputed: %s\n", buffer);
            }
        }
    }
    printf("program is exiting.\n");
    return 0;
}