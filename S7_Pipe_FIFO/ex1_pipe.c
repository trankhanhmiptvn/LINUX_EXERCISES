#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //pipe, fork
#include <sys/types.h> // pid_t
#include <sys/wait.h> 

static void die(const char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    // Tạo pipe
    int pipe_fds[2]; //read from pipe_fds[0], write from pipe_fds[1]
    if (pipe(pipe_fds) == -1) die("Pipe creat fail.");

    //Tạo tiến trình con thứ nhất
    pid_t pid_child1 = fork();
    if(pid_child1 == -1) die("Process child1 creat fail.");

    //Thực thi các nhiệm vụ trong tiến trình con thứ nhất
    if(pid_child1 == 0) {
        //Đóng đầu pipe không dùng (không đọc, chỉ ghi từ stdout)
        close(pipe_fds[0]);
        //Chuyển hướng stdout của tiến trình con vào đầu ghi của pipe
        if(dup2(pipe_fds[1], STDOUT_FILENO) == -1) die("Dup2 child1 fail.");
        close(pipe_fds[1]);
        //Thực thi ls -l
        execlp("ls", "ls", "-l", (char *) NULL);
        //Nếu lỗi 
        perror("Execpl ls -l fail.");
        _exit(EXIT_FAILURE);
    }

     //Tạo tiến trình con thứ nhất
    pid_t pid_child2 = fork();
    if(pid_child2 == -1) die("Process child2 creat fail.");

    //Thực thi các nhiệm vụ trong tiến trình con thứ hai
    if(pid_child2 == 0) {
        //Đóng đầu pipe không dùng (không ghi, chỉ đọc từ pipe)
        close(pipe_fds[1]);
        //Chuyển hướng stdin của tiến trình con lấy từ đầu đọc của pipe
        if(dup2(pipe_fds[0], STDIN_FILENO) == -1) die("Dup2 child2 fail.");
        close(pipe_fds[0]);
        //Thực thi ls -l
        execlp("wc", "wc", "-l", (char *) NULL);
        //Nếu lỗi 
        perror("Execpl wc -l fail.");
        _exit(EXIT_FAILURE);
    }

    //Tiên trình cha
    //Đóng cả hai đầu pipe để con biết nhận được EPF đúng lúc
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    //Đợi hai con kết thúc
    int st1, st2;
    if(waitpid(pid_child1, &st1, 0) == -1) die("Waitpid child1 fail.");
    if(waitpid(pid_child2, &st2, 0) == -1) die("Waitpid child2 fail.");
    if(WIFEXITED(st1) && WEXITSTATUS(st1) != 0) 
        fprintf(stderr, "ls exited with status %d\n", WEXITSTATUS(st1));
    if(WIFEXITED(st2) && WEXITSTATUS(st2) != 0) 
        fprintf(stderr, "wc exited with status %d\n", WEXITSTATUS(st2));
    return 0;
}