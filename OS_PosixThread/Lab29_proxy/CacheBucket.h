//
// Created by nastya on 25.11.15.
//

#ifndef LAB29_PROXY_CACHEBUCKET_H
#define LAB29_PROXY_CACHEBUCKET_H

#include <list>

class CacheBucket {
    bool isFull = false;
    std::list<char*> pagePieces;
};


#endif //LAB29_PROXY_CACHEBUCKET_H
