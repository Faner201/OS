#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "multi_process_launcher.h"
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#define LOG_FILE "log.txt"
#define TIMER_INTERVAL 300000
#define LOG_INTERVAL 1000000
#define COPY_INTERVAL 3000000
#define COPY1_INCREMENT 10
#define COPY2_MULTIPLY 2
#define TIMEOUT_2_SEC 2000000

volatile int *counter;
sem_t *counter_sem;
int running_copies = 0;
pid_t main_pid;

void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
}

void log_start_message() {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        pid_t main_pid = getpid();

        char start_message[256];
        
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        struct tm *tm_info;
        char time_buffer[64];
        tm_info = localtime(&ts.tv_sec);

        snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d.%03ld",
                 tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ts.tv_nsec / 1000000);
        
        snprintf(start_message, sizeof(start_message), "Main PID: %d started at %s\n", main_pid, time_buffer);

        fprintf(log_file, "%s", start_message);
        fclose(log_file);
    }
}

void *timer_process(void *arg) {
    while (1) {
        #ifdef _WIN32
        Sleep(TIMER_INTERVAL / 1000);
        #else
        usleep(TIMER_INTERVAL);
        #endif
        (*counter)++;
    }
    return NULL;
}

void *log_process(void *arg) {
    while (1) {
        #ifdef _WIN32
        Sleep(LOG_INTERVAL / 1000);
        #else
        usleep(LOG_INTERVAL);
        #endif
        char message[256];
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        struct tm *tm_info;
        char time_buffer[64];
        tm_info = localtime(&ts.tv_sec);

        snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d.%03ld",
                 tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ts.tv_nsec / 1000000);
        
        sem_wait(counter_sem);
        snprintf(message, sizeof(message), "PID: %d, Time: %s, Counter: %d", getpid(), time_buffer, *counter);
        sem_post(counter_sem);

        log_message(message);
    }
    return NULL;
}

void *copy1_process() {
    char message[256];

    snprintf(message, sizeof(message), "Copy1 PID: %d started", getpid());
    log_message(message);
    (*counter) += COPY1_INCREMENT;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info;
    char time_buffer[64];
    tm_info = localtime(&ts.tv_sec);

    snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ts.tv_nsec / 1000000);
    
    snprintf(message, sizeof(message), "Copy1 PID: %d exiting at %s", getpid(), time_buffer);

    log_message(message);
    return NULL;
}

void *copy2_process() {
    char message[256];

    snprintf(message, sizeof(message), "Copy2 PID: %d started", getpid());
    log_message(message);
    (*counter) *= COPY2_MULTIPLY;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info;
    char time_buffer[64];
    tm_info = localtime(&ts.tv_sec);

    snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ts.tv_nsec / 1000000);
    
    snprintf(message, sizeof(message), "Copy2 PID: %d waiting to reduce counter", getpid());
    log_message(message);

    #ifdef _WIN32
    Sleep(TIMEOUT_2_SEC / 1000);
    #else
    usleep(TIMEOUT_2_SEC);
    #endif

    (*counter) /= COPY2_MULTIPLY;

    snprintf(message, sizeof(message), "Copy2 PID: %d exiting at %s", getpid(), time_buffer);
    log_message(message);
    return NULL;
}

void *copy_launcher_process(void *arg) {
    while (1) {
        #ifdef _WIN32
        Sleep(COPY_INTERVAL / 1000);
        #else
        usleep(COPY_INTERVAL);
        #endif

        #ifdef _WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (CreateProcess(NULL, "copy1_process", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        if (CreateProcess(NULL, "copy2_process", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        #else
        pid_t pid1 = fork();
        if (pid1 == 0) {
            copy1_process();
            exit(0);
        }
        pid_t pid2 = fork();
        if (pid2 == 0) {
            copy2_process();
            exit(0);
        }
        #endif
    }
    return NULL;
}

void set_counter(int new_value) {
    *counter = new_value;
}

void start_process_launcher() {
    log_start_message();

    #ifdef _WIN32
    HANDLE log_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)log_process, NULL, 0, NULL);
    if (log_thread == NULL) {
        log_message("Failed to create log thread");
    }
    #else
    pthread_t log_thread;
    if (pthread_create(&log_thread, NULL, log_process, NULL) != 0) {
        log_message("Failed to create log thread");
    }
    #endif

    if (main_pid == 0) {
        main_pid = getpid();
    }

    while (1) {
        #ifdef _WIN32
        Sleep(COPY_INTERVAL / 1000);
        #else
        usleep(COPY_INTERVAL);
        #endif

        if (getpid() == main_pid) {
            #ifdef _WIN32
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcess(NULL, "copy1_process", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            if (CreateProcess(NULL, "copy2_process", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            #else
            pid_t pid1 = fork();
            if (pid1 == 0) {
                copy1_process();
                exit(0);
            }
            pid_t pid2 = fork();
            if (pid2 == 0) {
                copy2_process();
                exit(0);
            }
            #endif
        }
    }
}