#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/mman.h>   // shm_open, mmap, munmap

#define SHM_NAME "/myshm"
#define SHM_SIZE (sizeof(product_t))

typedef struct
{
    char name[50];
    int id;
    float price; 

} product_t;

static void die_perror(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) die_perror("shm_open consummer");

    product_t *p = mmap(NULL, SHM_SIZE,
                        PROT_READ,
                        MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) die_perror("mmap consummer");

    printf("[Consummer] Read: ID:%d | Name:%s | Price:%.2f\n", p->id, p->name, p->price);
    munmap(p, SHM_SIZE);
    close(fd);

    shm_unlink(SHM_NAME);

    return 0;
}