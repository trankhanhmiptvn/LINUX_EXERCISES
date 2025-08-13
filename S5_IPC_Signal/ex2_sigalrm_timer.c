#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAX_TIME 10
volatile  sig_atomic_t g_count = 0;
void handle_sigalrm (int sig) {
    g_count++;
}
    
int main() {
    signal(SIGALRM, handle_sigalrm);
    printf("Program is running.\n");
    alarm(1);
    while(1) { 
        pause();
        printf("SIGALRM. Timer: %d seconds.\n", g_count);
        if(g_count >= MAX_TIME) {
            printf("Program exit. SIGARLM received %d times.\n", g_count);
            fflush(stdout);
            _exit(0);
        }
        alarm(1);
     }
    return 0;
}