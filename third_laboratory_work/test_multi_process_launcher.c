#include <stdio.h>
#include "multi_process_launcher.h"

#ifdef _WIN32
#include <windows.h>

volatile int *counter;
HANDLE counter_sem;
HANDLE hMapFile;

#else
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

extern volatile int *counter;
extern sem_t *counter_sem;

#endif

int main() {
#ifdef _WIN32
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(int),
        TEXT("Global\\CounterShm"));

    if (hMapFile == NULL) {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        return 1;
    }

    counter = (int *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

    if (counter == NULL) {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    *counter = 0;

    counter_sem = CreateSemaphore(
        NULL,
        1,
        1,
        TEXT("Global\\CounterSem"));

    if (counter_sem == NULL) {
        printf("CreateSemaphore error: %d\n", GetLastError());
        UnmapViewOfFile(counter);
        CloseHandle(hMapFile);
        return 1;
    }

#else
    int shm_fd = shm_open("/counter_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return 1;
    }

    counter = (volatile int *)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return 1;
    }

    *counter = 0;

    counter_sem = sem_open("/counter_sem", O_CREAT, 0666, 1);
    if (counter_sem == SEM_FAILED) {
        perror("sem_open");
        munmap((void *)counter, sizeof(int));
        close(shm_fd);
        return 1;
    }

#endif

    int new_value;
    while (1) {
        printf("Enter new counter value: ");
        scanf("%d", &new_value);

#ifdef _WIN32
        if (WaitForSingleObject(counter_sem, INFINITE) != WAIT_OBJECT_0) {
            printf("WaitForSingleObject error: %d\n", GetLastError());
            break;
        }
#else
        sem_wait(counter_sem);
#endif

        set_counter(new_value);

#ifdef _WIN32
        if (!ReleaseSemaphore(counter_sem, 1, NULL)) {
            printf("ReleaseSemaphore error: %d\n", GetLastError());
            break;
        }
#else
        sem_post(counter_sem);
#endif

        start_process_launcher();
    }

#ifdef _WIN32
    UnmapViewOfFile(counter);
    CloseHandle(hMapFile);
    CloseHandle(counter_sem);
#else
    munmap((void *)counter, sizeof(int));
    close(shm_fd);
    sem_close(counter_sem);
    sem_unlink("/counter_sem");
    shm_unlink("/counter_shm");
#endif

    return 0;
}