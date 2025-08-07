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
        printf("Con: Tiến trình con với PID = %d, PPID = %d trước sleep.\n"
            , getpid(), getppid());
        sleep(3);
         printf("Con: Tiến trình con với PID = %d, PPID = %d sau sleep (orphan).\n"
            , getpid(), getppid());
    }
    else {
        int status;
        printf("Cha: PID = %d PID_CHILD = %d không gọi wait, kết thúc trước con.\n", getpid(), child_pid);
        exit(0);

    }
    return 0;
}
