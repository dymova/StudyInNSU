//
// Created by nastya on 16.12.15.
//

#ifndef MULTITHREADPROXY_CACHESTORAGE_H
#define MULTITHREADPROXY_CACHESTORAGE_H


#include <pthread.h>
#include <cstring>
#include <map>
#include "CacheBucket.h"

struct cmp_str {
    bool operator()(char const *a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

class CacheStorage {

private:
    pthread_mutex_t mutex;
    std::map<char *, CacheBucket *, cmp_str> cache;


public:
    bool contain(char *url);

    CacheBucket *getBucket(char *url);

    void insertNewBucket(std::pair<char *, CacheBucket *> pair);
};


#endif //MULTITHREADPROXY_CACHESTORAGE_H
