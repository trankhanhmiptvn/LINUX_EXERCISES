#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAX_TIME 3
volatile sig_atomic_t g_count = 0;
void handle_sigint (int sig) {
    g_count++;
}
int main() {
    signal(SIGINT, handle_sigint);
    printf("Program is running. Press Ctrl+C to send signal SIGINT.\n");
    while(1) { 
        pause();
        printf("SIGINT received  %d times.\n", g_count);
        if(g_count == MAX_TIME) {
            printf("Program exit. SIGINT received %d times.\n", g_count);
            fflush(stdout);
            _exit(0);
        }
     }
    return 0;
}