#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
typedef struct {
    int num;
    char * message;
} Data;
void * func_return (void * arg) {
    Data* data  = (Data *) arg;
    printf("Thread %d xử lý công việc và trả về %s.\n", data->num, data->message);
    char* result = malloc(100);
    snprintf(result, 100, "Return: Thread %d kết thúc thành công.\n", data->num);
    return (void *)result;

}
void * func_exit (void * arg) {
    Data* data  = (Data *) arg;
    printf("Thread %d xử lý công việc và trả về %s.\n", data->num, data->message);
    char* result = malloc(100);
    snprintf(result,100, "Return: Thread %d kết thúc thành công.\n", data->num);
    pthread_exit((void *)result);

}
int main() {
    pthread_t t1, t2;
    Data arg1 = {1, "Message from thread 1"};
    Data arg2 = {2, "Message from thread 2"};
    pthread_create(&t1, NULL, &func_return, &arg1);
    pthread_create(&t2, NULL, &func_exit, &arg2);
    
    void * ret1, *ret2;
    pthread_join(t1, &ret1);
    pthread_join(t2, &ret2);

    printf("%s", (char *) ret1);
    printf("%s", (char*) ret2);
    free(ret1);
    free(ret2);
    return 0;
}