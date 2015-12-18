#ifndef MULTITHREADPROXY_CACHEBUCKET_H
#define MULTITHREADPROXY_CACHEBUCKET_H
#include <vector>
#include <utility>
#include <pthread.h>

class CacheBucket {
private:
    bool full;
    std::vector<std::pair<char *, int> > pagePieces;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
public:

    CacheBucket();

    void setIsFull(bool isFull);

    void addItem(char *, int);

    unsigned long size();

    std::pair<char *, int> getItem(unsigned long position);

    bool isFull() ;

    void waitData(unsigned long currentCachePosition);
};


#endif //MULTITHREADPROXY_CACHEBUCKET_H
