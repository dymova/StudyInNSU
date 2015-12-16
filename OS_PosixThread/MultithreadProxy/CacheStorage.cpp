//
// Created by nastya on 16.12.15.
//

#include "CacheStorage.h"

bool CacheStorage::contain(char *url) {
    bool contain;
    pthread_mutex_lock(&mutex);
    if(cache.find(url) == cache.end()){
        contain = false;
    } else {
        contain = true;
    }
    pthread_mutex_unlock(&mutex);
    return contain;
}

CacheBucket *CacheStorage::getBucket(char *url) {
    pthread_mutex_lock(&mutex);
    CacheBucket* bucket = cache.find(url)->second;
    pthread_mutex_unlock(&mutex);
    return bucket;
}

void CacheStorage::insertNewBucket(std::pair<char *, CacheBucket *> pair) {
    pthread_mutex_lock(&mutex);
    cache.insert(pair);
    pthread_mutex_unlock(&mutex);
}
