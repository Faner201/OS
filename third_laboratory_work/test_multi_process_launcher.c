#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "multi_process_launcher.h"

extern volatile int *counter;
extern sem_t *counter_sem;

int main() {
    int shm_fd = shm_open("/counter_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int));
    counter = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    *counter = 0;
    counter_sem = sem_open("/counter_sem", O_CREAT, 0666, 1);

    int new_value;
    while (1) {
        printf("Enter new counter value: ");
        scanf("%d", &new_value);
        sem_wait(counter_sem);
        set_counter(new_value);
        sem_post(counter_sem);
        start_process_launcher();
    }

    return 0;
}