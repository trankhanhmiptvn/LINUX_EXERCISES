#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>

#define QUEUE_NAME "/my_task_queue"
#define MSGSZ 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <priority> \"<task_description>\"\n", argv[0]);
        return 1;
    }

    unsigned int prio = (unsigned int)atoi(argv[1]);
    char *task = argv[2];

    if (prio > 31) {
        fprintf(stderr, "Priority should be 0-31 (POSIX default max).\n");
        return 1;
    }

    mqd_t mqdes = mq_open(QUEUE_NAME, O_WRONLY);
    if (mqdes == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    if (mq_send(mqdes, task, strlen(task)+1, prio) == -1) {
        perror("mq_send");
        mq_close(mqdes);
        return 1;
    }

    printf("Dispatcher: Task sent (Priority: %u): %s\n", prio, task);
    mq_close(mqdes);
    return 0;
}
