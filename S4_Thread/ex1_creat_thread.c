#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
void * thread_funtion(void *) {
    pthread_t tid = pthread_self(); // Lấy ID của thread
    printf("Thread với ID = %lu đang chạy.\n", (unsigned long)tid);
    pthread_exit(NULL);
}
int main() {
    pthread_t thread1, thread2;
    //Tạo thread1
    if(pthread_create(&thread1, NULL, &thread_funtion, NULL) != 0) {
        perror("Tạo thread thất bại.\n");
        exit(EXIT_FAILURE);
    }
    //Tạo thread2
    if(pthread_create(&thread2, NULL, &thread_funtion, NULL) != 0) {
        perror("Tạo thread thất bại.\n");
        exit(EXIT_FAILURE);
    }

    //Chờ hai luồng kết thúc
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);     
    printf("Luồng main kết thúc sau khi luồng con hoàn tất.\n");
    return 0;
}
#if 0
int pthread_create(
    pthread_t *thread,             // [out] lưu ID của thread mới
    const pthread_attr_t *attr,   // [in] thuộc tính thread (NULL = mặc định)
    void *(*start_routine)(void*),// [in] con trỏ hàm thread sẽ chạy
    void *arg                      // [in] tham số truyền vào hàm trên
);
int pthread_join(
    pthread_t thread,  // [in] ID của thread cần chờ
    void **retval      // [out] lấy giá trị trả về từ thread (NULL nếu không cần)
);
Một luồng có thể kết thúc khi:
- Hàm được truyền vào pthread_create() return
- Gọi pthread_exit() từ bên trong luồng
- Bị thread khác hủy bằng pthread_cancel()
- Hoặc khi main thread gọi exit(), thì toàn bộ process dừng, kể cả các thread chưa xong.

#endif