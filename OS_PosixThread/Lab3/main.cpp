#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int BUFFER_SIZE = 256;
const int THREAD_COUNT = 4;

void* thread_body(void* param) {
    struct timespec time;
    unsigned int seed = (unsigned int) time.tv_nsec;
    clock_gettime(NULL, &time);
    int index = rand_r(&seed) % THREAD_COUNT;
//    unsigned long int index = (unsigned long int) pthread_self() % THREAD_COUNT;
    char** test_string = (char**) param;
    test_string += index;

    while(NULL != *test_string){
        printf("%s ", *test_string);
        test_string++;
    }
    printf("\n");
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    static char* param[]={(char *) "string 1", (char *) "string 2", (char *) "string 3", (char *) "string 4", (char *) "string 5", NULL};
    pthread_t thread;
    int code;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        code = pthread_create(&thread, NULL, thread_body, param);
        if (code != 0) {
            char buf[BUFFER_SIZE];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}