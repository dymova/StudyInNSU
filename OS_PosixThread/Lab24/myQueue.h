#ifndef LAB23_MYQUEUE_H
#define LAB23_MYQUEUE_H

#include <semaphore.h>

typedef struct queueRecord {
    struct queueRecord* next;
    struct queueRecord* prev;
    char buf[80];
} QueueRecord;

typedef struct queue {
    QueueRecord *head;
    QueueRecord* tail;
    int destroyFlag;

    int size;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Queue;

void mymsginit(Queue *);

void mymsgdrop(Queue *);

void mymsgdestroy(Queue *);

int mymsgput(Queue *, char *msg);

int mymsgget(Queue *, char *buf, size_t bufSize);


#endif //LAB23_MYQUEUE_H
