#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int BUFFER_SIZE = 256;

void* thread_body(void* param) {
    char** test_string = (char**)param;
    while(NULL != *test_string){
        printf("%s\n", *test_string);
        test_string++;
    }
    pthread_exit(NULL);
}

void error_handler(int code, char** argv){
    char buf[BUFFER_SIZE];
    strerror_r(code, buf, sizeof buf);
    fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
}

int main(int argc, char *argv[]) {
    static char* param_1[]={(char *) "Thread 1 string 1", (char *) "Thread 1 string 2", NULL};
    static char* param_2[]={(char *) "Thread 2 string 1", (char *) "Thread 2 string 2", NULL};
    static char* param_3[]={(char *) "Thread 3 string 1", (char *) "Thread 3 string 2", NULL};
    static char* param_4[]={(char *) "Thread 4 string 1", (char *) "Thread 4 string 2", NULL};
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, thread_body, param_1);
    if (code != 0) {
        error_handler(code, argv);
        exit(EXIT_FAILURE);
    }
    code = pthread_create(&thread, NULL, thread_body, param_2);
    if (code != 0) {
        error_handler(code, argv);
        exit(EXIT_FAILURE);
    }
    code = pthread_create(&thread, NULL, thread_body, param_3);
    if (code != 0) {
        error_handler(code, argv);
        exit(EXIT_FAILURE);
    }
    code = pthread_create(&thread, NULL, thread_body, param_4);
    if (code != 0) {
        error_handler(code, argv);
        exit(EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}