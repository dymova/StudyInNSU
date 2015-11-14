#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "myQueue.h"

#define PRODUCER_COUNT 2
#define CONSUMER_COUNT 2
#define BUF 30

void *consumer(void *param) {
    Queue *q = (Queue *) param;
    char buf[BUF];

    int count = 0;
    for (; ;) {
        sleep(1);
        if (!mymsgget(q, buf, sizeof(buf))) {
            return NULL;
        }
        printf("Received from queue %d: %s\n",count,  buf);
        count++;
    }
}


void* producer(void *param) {
    Queue *q = (Queue *) param;

    int counter = 0;
    for (; ;) {
        char buf[BUF];
        sprintf(buf, "Message %d from thread %d", counter, (int) pthread_self());

        if (!mymsgput(q, buf)) {
            return NULL;
        }
        counter++;
    }
}

int main(int argc, char *argv[]) {
    pthread_t *producers = (pthread_t *) malloc(PRODUCER_COUNT * sizeof(pthread_t));
    pthread_t *consumers = (pthread_t *) malloc(CONSUMER_COUNT * sizeof(pthread_t));
    Queue queue;

    mymsginit(&queue);
    int code;
    
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        code = pthread_create(&producers[i], NULL, producer, &queue);
        if (code != 0) {
            char buf[256];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: cancelling thread: %s\n", argv[0], buf);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        code = pthread_create(&consumers[i], NULL, consumer, &queue);
        if (code != 0) {
            char buf[256];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: cancelling thread: %s\n", argv[0], buf);
            exit(EXIT_FAILURE);
        }
    }
    sleep(10);

    printf("after sleep");
    mymsgdrop(&queue);

    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        pthread_join(consumers[i], NULL);
    }

    mymsgdestroy(&queue);
    return (EXIT_SUCCESS);
}