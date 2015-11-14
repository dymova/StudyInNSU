#include <malloc.h>
#include <string.h>
#include "myQueue.h"

void mymsginit(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->destroyFlag = 0;
    sem_init(&(q->tailSem), 0, 10);
    sem_init(&(q->headSem), 0, 0);
    sem_init(&(q->queueSem), 0, 1);
}

int mymsgput(Queue *q, char *msg) {
    QueueRecord *record;

    sem_wait(&q->queueSem);
    sem_wait(&q->tailSem);
    if (q->destroyFlag) {
        sem_post(&q->tailSem);
        sem_post(&q->queueSem);
        return 0;
    }
    record = (QueueRecord *) malloc(sizeof(QueueRecord));
    strncpy(record->buf, msg, sizeof(record->buf));
    record->buf[sizeof(record->buf) - 1] = '\0';
    record->prev = q->tail;
    record->next = NULL;
    if (q->tail == NULL) {
        q->head = q->tail = record;
    } else {
        q->tail->next = record;
        q->tail = record;
    }
    sem_post(&q->queueSem);
    sem_post(&q->headSem);
    return strlen(record->buf);
}

int mymsgget(Queue *q, char *buf, size_t bufsize) {
    QueueRecord *record;

    sem_wait(&q->headSem);
    sem_wait(&q->queueSem);
    if (q->destroyFlag) {
        sem_post(&q->headSem);
        sem_post(&q->queueSem);
        return 0;
    }
    record = q->head;
    if (q->tail == record) {
        q->tail = q->head = NULL;
    } else {
        q->head = record->next;
        q->head->prev = NULL;
    }
    sem_post(&q->queueSem);
    strncpy(buf, record->buf, bufsize);
    buf[bufsize - 1] = '\0';
    free(record);
    sem_post(&q->tailSem);
    return strlen(buf);
}

void mymsgdestroy(Queue *q) {
    sem_destroy(&q->headSem);
    sem_destroy(&q->tailSem);
    sem_destroy(&q->queueSem);
}

void mymsgdrop(Queue *q) {
    QueueRecord *record;
    q->destroyFlag=1;
    sem_wait(&q->queueSem);
    sem_post(&q->headSem);
    sem_post(&q->tailSem);
    record =q->head;
    while(record) {
        QueueRecord *tmp;
        tmp = record->next;
        free(record);
        record = tmp;
    }
    sem_post(&q->queueSem);
}

