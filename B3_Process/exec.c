#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t child_pid;
    //int setenv(const char *name, const char *value, int overwrite);
    setenv("MY_COMMAND", "/bin/ls", 1);
    child_pid = fork();
    if(child_pid < 0){
        perror("Lỗi khởi tạo tiến trình con bằng fork.\n");
        exit(EXIT_FAILURE);
    }
    else if(child_pid == 0) {
        printf("Con: Tiến trình con với PID = %d.\n", getpid());
        char *cmd = getenv("MY_COMMAND");
        if(cmd == NULL) {
            printf("Lỗi không tìm thấy biến môi trường.\n");
            exit(EXIT_FAILURE);
        }
        printf("Tiến trình con sẽ thực thi lệnh MY_COMMAND.\n");
        execl(cmd, cmd, NULL);
        perror("Lỗi thực thi lệnh execl.\n");
        exit(EXIT_FAILURE);
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
