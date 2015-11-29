//
// Created by nastya on 25.11.15.
//

#ifndef LAB29_PROXY_CACHEBUCKET_H
#define LAB29_PROXY_CACHEBUCKET_H

#include <list>
#include <vector>

class CacheBucket {
public:
    bool isFull;
    std::vector<char*> pagePieces;


    CacheBucket();
};


#endif //LAB29_PROXY_CACHEBUCKET_H
