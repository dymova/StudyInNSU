#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINTED_CHILD 1
#define PRINTED_PARENT 2


const int COUNT_LINE = 10;
const int BUFFER_SIZE = 256;
pthread_mutex_t mutex;
pthread_cond_t  condition;

int print_status = PRINTED_PARENT;


void* print_message(void* param) {
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < COUNT_LINE; i++) {
        while (print_status != PRINTED_PARENT) {
            pthread_cond_wait(&condition, &mutex);
        }
        printf("Child: %d\n", i);
        print_status = PRINTED_CHILD;
        pthread_cond_signal(&condition);
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: setting mutexattr type: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_mutex_init(&mutex, &attr);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: init mutex: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_cond_init(&condition, NULL);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: init condition: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_create(&thread, NULL, print_message, NULL);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < COUNT_LINE; i++) {
        while (print_status != PRINTED_CHILD) {
            pthread_cond_wait(&condition, &mutex);
        }
        printf("Parent: %d\n", i);
        print_status = PRINTED_PARENT;
        pthread_cond_signal(&condition);
    }
    pthread_mutex_unlock(&mutex);

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    return(EXIT_SUCCESS);
}