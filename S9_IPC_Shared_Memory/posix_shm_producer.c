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
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) die_perror("shm_open producer");

    if (ftruncate(fd, SHM_SIZE) == -1) die_perror("ftruncate");

    product_t *p = mmap(NULL, SHM_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) die_perror("mmap producer");

    printf("[Producer] Enter information about product:\n");
    printf("Product ID: ");
    scanf("%d", &p->id);
    getchar();
    printf("Product name: ");
    if (fgets(p->name, sizeof(p->name), stdin) == NULL) die_perror("fgets name");
    p->name[strcspn(p->name,"\n")] = '\0';
    printf("Product price: ");
    scanf("%f", &p->price);
    printf("[Producer] Wrote: ID:%d | Name:%s | Price:%.2f\n", p->id, p->name, p->price);
    munmap(p, SHM_SIZE);
    close(fd);

    return 0;
}