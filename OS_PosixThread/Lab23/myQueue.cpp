//
// Created by nastya on 26.10.15.
//

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

    sem_wait(&q->tailSem);
    sem_wait(&q->queueSem);
    if (q->destroyFlag) {
        sem_post(&q->tailSem);
        sem_post(&q->queueSem);
        return 0;
    }
    record = (QueueRecord *) malloc(sizeof(QueueRecord));
    strncpy(record->buf, msg, sizeof(record->buf) - 1);
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
    return strlen(record->buf) + 1;
}

int mymsgget(queue *q, char *buf, size_t bufsize) {
    QueueRecord *t;

    sem_wait(&q->headSem);
    sem_wait(&q->queueSem);
    if (q->destroyFlag) {
        sem_post(&q->headSem);
        sem_post(&q->queueSem);
        return 0;
    }
    t = q->head;
    if (q->tail == t) {
        q->tail = q->head = NULL;
    } else {
        q->head = t->next;
        q->head->prev = NULL;
    }
    sem_post(&q->queueSem);
    strncpy(buf, t->buf, bufsize - 1);
    buf[bufsize - 1] = '\0';
    free(t);
    sem_post(&q->tailSem);
    return strlen(buf) + 1;
}

void mymsgdestroy(queue *q) {
    sem_destroy(&q->headSem);
    sem_destroy(&q->tailSem);
    sem_destroy(&q->queueSem);
}

void mymsgdrop(queue *q) {
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

