#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 3
#define INCREMENTS 1000000

long long g_counter = 0;
pthread_mutex_t g_lock;
void * increment_counter (void *arg) {
    for(int i = 0; i < INCREMENTS; i++) {
        //pthread_mutex_lock(&g_lock);
        g_counter++;
        //printf("Couter:%lld\n", g_counter);
        //pthread_mutex_unlock(&g_lock);
    }

    pthread_exit(NULL);
}
int main() {

    pthread_t thread[NUM_THREADS];

    //Khởi tạo mutex, khởi tạo động, có thể khởi tạo tĩnh PTHREAD_MUTEX_INIT
    if(pthread_mutex_init(&g_lock, NULL) != 0) {
        perror("Mutex init fail.\n");
        exit(EXIT_FAILURE);
    }

    //Creat Threads
    for(int i = 0; i < NUM_THREADS; i++) {
        if(pthread_create(&thread[i], NULL, &increment_counter, NULL) != 0) {
            perror("Thread creation fail.\n");
            exit(EXIT_FAILURE);
        }
    }

    //Wait for threas end
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    //Delete mutex
    pthread_mutex_destroy(&g_lock);
    printf("Giá trị cuối cùng của couter là %lld\n", g_counter);
    return 0;

}
#if 0
Tại sao cần dùng mutex? Để tránh Race condition xảy ra khi nhiều thread cùng 
đọc & ghi vào một biến dùng chung mà không đồng bộ, gây lỗi logic.
Có 3 thread → cùng tăng biến counter
temp = counter;
temp = temp + 1;
counter = temp;
Nếu 2 thread chạy đồng thời → dễ ghi đè lẫn nhau và tăng sai.
#endif
