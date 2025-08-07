#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h> //radom function

#define NUMBER_DATAS 10

// global variables 
int g_data = 0;
bool g_data_ready = false;

//Khởi thạo biến mutex và conditions (khởi tạo tĩnh)
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

//producer sinh ngẫu nhiên và báo cho custormer
void * producer_function (void* arg) {
    for(int i = 0; i < NUMBER_DATAS; i++) {
        pthread_mutex_lock(&g_lock);
        while (g_data_ready) {
            pthread_cond_wait(&g_cond, &g_lock);
        }
        g_data = rand();
        g_data_ready = true;
        printf("Producer sản xuất dữ liệu: %d.\n", g_data);
        pthread_cond_signal(&g_cond);
        pthread_mutex_unlock(&g_lock);
    }
    pthread_exit(NULL);
}

void *consumer_function(void* arg) {
    for(int i = 0; i < NUMBER_DATAS; i++){
        pthread_mutex_lock(&g_lock);
        while(!g_data_ready) {
            pthread_cond_wait(&g_cond, &g_lock);
        }
        printf("Cosumer nhận dữ liệu: %d.\n", g_data);
        g_data_ready = false;
        pthread_cond_signal(&g_cond);
        pthread_mutex_unlock(&g_lock);
    }
    pthread_exit(NULL);
}
 int main() {
    srand(time(NULL)); //Khởi tạo seed cho rand()
    
    pthread_t prod_thread, cons_thread;
    // Mutex và condition đã được khởi tạo tình INITIALIZER

    //Tạo luồng
    pthread_create(&prod_thread, NULL, producer_function, NULL);
    pthread_create(&cons_thread, NULL, consumer_function, NULL);

    //Đợi hai luồng kết thúc
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    //Hủy các đối tượng đồng bộ hóa
    pthread_mutex_destroy(&g_lock);
    pthread_cond_destroy(&g_cond);

    printf("Chương trình kết thúc.\n");

    return 0;
 }