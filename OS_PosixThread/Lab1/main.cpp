#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int COUNT_LINE = 10;
const int BUFFER_SIZE = 256;

void* thread_body(void* param) {
    for(int i = 0; i < COUNT_LINE; i++) {
        printf("Child: %d\n", i);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 10; i++) {
        printf("Parent: %d\n", i);
    }
    return(EXIT_SUCCESS);
}