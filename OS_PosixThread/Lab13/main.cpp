#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

sem_t s1;
sem_t s2;

const int COUNT_LINE = 10;
const int BUFFER_SIZE = 256;

void* thread_body(void* param) {
    for(int i = 0; i < COUNT_LINE; i++) {
        sem_wait(&s2);
        printf("Child: %d\n", i);
        sem_post(&s1);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    code = sem_init ( &s1, 0, 0 );
    if (code == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    code = sem_init ( &s2, 0, 1 );
    if (code != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 10; i++) {
        sem_wait(&s1);
        printf("Parent: %d\n", i);
        sem_post(&s2);
    }

    pthread_join(thread, NULL);

    sem_destroy(&s1);
    sem_destroy(&s2);

    return(EXIT_SUCCESS);
}