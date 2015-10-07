#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const int COUNT_LINE = 10;
const int BUFFER_SIZE = 256;

void* thread_body(void* param) {
    char* str = (char *) "\t Test\n";
    for(int i = 0; i < COUNT_LINE; i++) {
        printf("Child: %d\n", i);
    }

    double k = 222.0;
    printf("%f\n", k);


    pthread_attr_t attr;
    size_t stack_size;
    pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &stack_size);
    printf("Thread stack size = %d bytes \n", (int) stack_size);
    printf("str addr: %p\n", &str);
    return str;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    char* str;
    printf("str addr before join: %p \n", &str);
    code = pthread_join(thread, (void **) &str);
    printf("%s", str);
    printf("str addr after join: %p \n", &str);


    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: join thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 10; i++) {
        printf("Parent: %d\n", i);
    }

    return(EXIT_SUCCESS);
}