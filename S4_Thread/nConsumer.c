#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE     5
#define NUM_PRODUCERS   2
#define NUM_CONSUMERS   3
#define NUM_ITEMS       10   // mỗi producer sinh 10 item

int buffer[BUFFER_SIZE];
int count = 0;     // số phần tử hiện tại
int in = 0;        // chỉ số ghi vào
int out = 0;       // chỉ số đọc ra

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full  = PTHREAD_COND_INITIALIZER;  // buffer đầy
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;  // buffer rỗng

void* producer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;

        pthread_mutex_lock(&lock);

        // chờ nếu buffer đầy
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_full, &lock);
        }

        // thêm item vào buffer
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Producer %d sản xuất %d → buffer count: %d\n", id, item, count);

        pthread_cond_signal(&cond_empty);  // đánh thức consumer
        pthread_mutex_unlock(&lock);

        usleep(100000);  // giả lập thời gian sản xuất
    }
    pthread_exit(NULL);
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < (NUM_PRODUCERS * NUM_ITEMS) / NUM_CONSUMERS; i++) {
        pthread_mutex_lock(&lock);

        // chờ nếu buffer rỗng
        while (count == 0) {
            pthread_cond_wait(&cond_empty, &lock);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumer %d tiêu thụ %d ← buffer count: %d\n", id, item, count);

        pthread_cond_signal(&cond_full);  // đánh thức producer
        pthread_mutex_unlock(&lock);

        usleep(150000);  // giả lập thời gian xử lý
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t prod_threads[NUM_PRODUCERS];
    pthread_t cons_threads[NUM_CONSUMERS];
    int ids[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    // tạo producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        ids[i] = i + 1;
        pthread_create(&prod_threads[i], NULL, producer, &ids[i]);
    }

    // tạo consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&cons_threads[i], NULL, consumer, &ids[i]);
    }

    // chờ kết thúc
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prod_threads[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    // hủy đối tượng đồng bộ
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond_full);
    pthread_cond_destroy(&cond_empty);

    printf("Chương trình kết thúc.\n");
    return 0;
}
