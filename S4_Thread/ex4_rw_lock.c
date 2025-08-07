#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREAD_READ  5
#define NUM_THREAD_WRITE 2
#define NUM_ITERATIONS   5

pthread_t read_thread[NUM_THREAD_READ];
pthread_t write_thread[NUM_THREAD_WRITE];

int read_ids[NUM_THREAD_READ];
int write_ids[NUM_THREAD_WRITE];

int g_shared_data = 5;
/// @brief read_write_lock
pthread_rwlock_t g_rwlock = PTHREAD_RWLOCK_INITIALIZER;

void* read_function(void* arg){
    int id = *(int*) arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_rwlock_rdlock(&g_rwlock);
        printf("Reader %d reads data: %d.\n", id, g_shared_data);
        pthread_rwlock_unlock(&g_rwlock);
        usleep(100000 + rand() % 100000);
    }
    pthread_exit(NULL);
}
void* write_function(void* arg) {
    int id = *(int *)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_rwlock_wrlock(&g_rwlock);
        g_shared_data++;
        printf("Writer %d write data: %d.\n",id, g_shared_data);
        pthread_rwlock_unlock(&g_rwlock);
        usleep(150000 + rand() % 100000);
    }
    pthread_exit(NULL);
}
int main() {
    for(int i = 0; i < NUM_THREAD_READ; i++) {
        read_ids[i] = i + 1;
        pthread_create(&read_thread[i], NULL, &read_function, &read_ids[i]);
    }
    for(int i =0; i < NUM_THREAD_WRITE; i++) {
        write_ids[i] = i + 1;
        pthread_create(&write_thread[i], NULL, &write_function, &write_ids[i]);
    }

    for(int i = 0; i < NUM_THREAD_READ; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for(int i =0; i < NUM_THREAD_WRITE; i++) {
        pthread_join(write_thread[i], NULL);
    }
    pthread_rwlock_destroy(&g_rwlock);
    return 0;

}