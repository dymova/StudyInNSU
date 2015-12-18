//
// Created by nastya on 15.12.15.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CacheBucket.h"

#define BUFSIZE 1024

CacheBucket::CacheBucket() {
    full = false;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "setting mutexattr type: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_mutex_init(&mutex, &attr);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "init mutex: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_cond_init(&cond, NULL);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "init condition: %s\n", buf);
        exit(EXIT_FAILURE);
    }
}

void CacheBucket::addItem(char *buf, int size) {
    pthread_mutex_lock(&mutex);
    pagePieces.push_back(std::make_pair(buf, size));
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

unsigned long CacheBucket::size() {
    pthread_mutex_lock(&mutex);
    unsigned long size = pagePieces.size();
    pthread_mutex_unlock(&mutex);
    return size;
}

std::pair<char *, int> CacheBucket::getItem(unsigned long position) {
    pthread_mutex_lock(&mutex);
    std::pair<char *, int> pair = pagePieces.at(position);
    pthread_mutex_unlock(&mutex);
    return pair;
}

bool CacheBucket::isFull() {
    pthread_mutex_lock(&mutex);
    bool res = full;
    pthread_mutex_unlock(&mutex);
    return res;
}

void CacheBucket::setIsFull(bool isFull) {
    pthread_mutex_lock(&mutex);
    full = isFull;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void CacheBucket::waitData(unsigned long currentCachePosition) {
    pthread_mutex_lock(&mutex);
    while (pagePieces.size() == currentCachePosition) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}
