#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include "myQueue.h"

void mymsginit(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->destroyFlag = 0;
    q->size = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond), NULL);
}

int mymsgput(Queue *q, char *msg) {
    QueueRecord *record;

    pthread_mutex_lock(&q->mutex);
    while (q->size >= 10 && !(q->destroyFlag)) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    if (q->destroyFlag) {
        pthread_mutex_unlock(&q->mutex);
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
    if (q->size==0) {
        pthread_cond_signal(&q->cond);
    }
    q->size++;
    pthread_mutex_unlock(&q->mutex);
    return strlen(record->buf);
}

int mymsgget(Queue *q, char *buf, size_t bufsize) {
    QueueRecord *record;

    pthread_mutex_lock(&q->mutex);
    while(q->size==0 && !q->destroyFlag) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    if (q->destroyFlag) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    record = q->head;
    if (q->tail == record) {
        q->tail = q->head = NULL;
    } else {
        q->head = record->next;
        q->head->prev = NULL;
    }
    if (q->size==10) {
        pthread_cond_signal(&q->cond);
    }
    q->size--;
    pthread_mutex_unlock(&q->mutex);
    strncpy(buf, record->buf, bufsize);
    buf[bufsize-1]='\0';
    free(record);
    return strlen(buf);
}

void mymsgdestroy(Queue *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void mymsgdrop(Queue *q) {
    QueueRecord *record;
    q->destroyFlag = 1;
    pthread_mutex_lock(&q->mutex);
    pthread_cond_broadcast(&q->cond);//todo check
    record = q->head;
    while (record) {
        QueueRecord *tmp;
        tmp = record->next;
        free(record);
        record = tmp;
    }
    pthread_mutex_unlock(&q->mutex);
}

