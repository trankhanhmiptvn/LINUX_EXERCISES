#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TIME 5
volatile  sig_atomic_t g_count = 0;
void handle_sigusr1 (int sig) {
    g_count++;    
}
int main() {
    
    pid_t pid_child;
    pid_child = fork();

    if (pid_child < 0) {
        perror("fork failed");
        exit(1);
    }
    if(pid_child == 0) {
        //child
        signal(SIGUSR1, handle_sigusr1);
        while(1) { 
            pause(); 
            printf("Child: SIGUSR1 count %d.\n", g_count);
            if(g_count >= MAX_TIME){
                printf("Child: reached %d signals. Exiting...\n", g_count);
                fflush(stdout);
                _exit(0);
            }
        }
    }
    else {
        //parent
        usleep(100000); //
        for(int i = 0; i < MAX_TIME; i++) {
            kill(pid_child, SIGUSR1);
            sleep(1);
        }
        
    }
    
    kill(pid_child, SIGTERM);
    wait(NULL);
    printf("Prent finished sending signals. Exitting...\n");
    return 0;
}