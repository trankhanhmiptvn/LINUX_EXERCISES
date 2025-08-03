#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t child_pid;
    child_pid = fork();
    if(child_pid < 0){
        perror("Lỗi khởi tạo tiến trình con bằng fork.\n");
        return 1;
    }
    else if(child_pid == 0) {
        printf("Con: Tiến trình con với PID = %d.\n", getpid());
        exit(10);
    }
    else {
        int status;
        printf("Cha: PID = %d PID_CHILD = %d.\n", getpid(), child_pid);
        pid_t wpid = wait(&status);
        if(WIFEXITED(status)) {
            printf("Mã thoát của chương trình con là %d.\n", WEXITSTATUS(status));
        }

    }
    return 0;
}
