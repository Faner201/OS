#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "multi_process_launcher.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define LOG_FILE "log.txt"
#define TIMER_INTERVAL 300000 // 300 ms
#define LOG_INTERVAL 1000000   // 1 sec
#define COPY_INTERVAL 3000000  // 3 sec
#define COPY1_INCREMENT 10
#define COPY2_MULTIPLY 2

volatile int counter = 0;
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

void *timer_thread(void *arg) {
    while (1) {
        #ifdef _WIN32
        Sleep(TIMER_INTERVAL / 1000);
        #else
        usleep(TIMER_INTERVAL);
        #endif
        counter++;
    }
    return NULL;
}

void *log_thread(void *arg) {
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
        
        snprintf(message, sizeof(message), "PID: %d, Time: %s, Counter: %d", getpid(), time_buffer, counter);

        log_message(message);
    }
    return NULL;
}

void *copy1_process() {
    char message[256];

    snprintf(message, sizeof(message), "Copy1 PID: %d started", getpid());
    log_message(message);
    counter += COPY1_INCREMENT;

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
    counter *= COPY2_MULTIPLY;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info;
    char time_buffer[64];
    tm_info = localtime(&ts.tv_sec);

    snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ts.tv_nsec / 1000000);
    
    snprintf(message, sizeof(message), "Copy2 PID: %d exiting at %s", getpid(), time_buffer);

    log_message(message);
    return NULL;
}

void *copy_launcher_thread(void *arg) {
    while (1) {
        #ifdef _WIN32
        Sleep(COPY_INTERVAL / 1000);
        #else
        usleep(COPY_INTERVAL);
        #endif
        if (running_copies < 2) {
            pthread_t copy1, copy2;
            running_copies++;
            pthread_create(&copy1, NULL, copy1_process, NULL);
            pthread_create(&copy2, NULL, copy2_process, NULL);
            pthread_detach(copy1);
            pthread_detach(copy2);
        } else {
            char message[256];
            snprintf(message, sizeof(message), "A copy is still running (running_copies: %d), skipping launch.", running_copies);
            log_message(message);
        }
    }
    return NULL;
}

void set_counter(int new_value) {
    counter = new_value;
}

void start_process_launcher() {
    log_start_message();

    pthread_t timer_tid, log_tid, copy_launcher_tid;
    pthread_create(&timer_tid, NULL, timer_thread, NULL);
    pthread_create(&log_tid, NULL, log_thread, NULL);
    pthread_create(&copy_launcher_tid, NULL, copy_launcher_thread, NULL);
}