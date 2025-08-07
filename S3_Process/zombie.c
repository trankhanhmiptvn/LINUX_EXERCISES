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
        printf("Con: Tiến trình zombie với PID = %d, PPID = %d.\n"
            , getpid(), getppid());
        exit(0);
    }
    else {
        int status;
        printf("Cha: PID = %d PID_CHILD = %d không gọi wait.\n", getpid(), child_pid);
        sleep(30);

    }
    return 0;
}
//ps -elf | grep defunct