//
// Created by nastya on 05.12.15.
//

#ifndef PROXY_CACHEBUCKET_H
#define PROXY_CACHEBUCKET_H

#include <vector>
#include <utility>

class CacheBucket {
public:
    CacheBucket();

    bool isFull;
    std::vector<std::pair<char*, int>> pagePieces;

    void setIsFull(bool isFull) {
        CacheBucket::isFull = isFull;
    }

    void addItem(char*, int);

    unsigned long size();
};


#endif //PROXY_CACHEBUCKET_H
