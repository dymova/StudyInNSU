#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const size_t BUFF_SIZE = 256;

void* thread_body(void* param) {
    for(;;) {
        write(1,"This is test message from child thread.\n", 41);
    }
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    sleep(2);
    code = pthread_cancel(thread);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: cancelling thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }
    printf("\nCancelled\n");

    return (EXIT_SUCCESS);
}