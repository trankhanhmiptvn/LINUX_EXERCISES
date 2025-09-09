#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define QUEUE_NAME "/my_task_queue"
#define MSGSZ 256

mqd_t mqdes = (mqd_t)-1; // descriptor toàn cục

void cleanup_and_exit(int sig) {
    if (mqdes != (mqd_t)-1) mq_close(mqdes);
    mq_unlink(QUEUE_NAME);
    printf("\nWorker: Queue closed and removed. Exiting.\n");
    exit(0);
}

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;          // blocking mode
    attr.mq_maxmsg = 10;        // tối đa 10 message trong queue
    attr.mq_msgsize = MSGSZ;    // kích thước mỗi message
    attr.mq_curmsgs = 0;

    // Đăng ký handler Ctrl+C để dọn dẹp queue
    struct sigaction sa = {0};
    sa.sa_handler = cleanup_and_exit;
    sigaction(SIGINT, &sa, NULL);

    // Tạo hoặc mở message queue
    mqdes = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr);
    if (mqdes == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    printf("Worker: Listening on queue %s\n", QUEUE_NAME);
    printf("Worker: Queue %s opened. Waiting 5s for tasks...\n", QUEUE_NAME);
    sleep(60); // chờ dispatcher gửi nhiều task trước

    while (1) {
        char buffer[MSGSZ];
        unsigned int prio;
        ssize_t n = mq_receive(mqdes, buffer, MSGSZ, &prio);
        if (n >= 0) {
            buffer[n] = '\0'; // null terminate
            printf("Processing task (Priority: %u): %s\n", prio, buffer);
            sleep(1); // mô phỏng xử lý
        } else {
            perror("mq_receive");
            break;
        }
    }

    cleanup_and_exit(0);
    return 0;
}
