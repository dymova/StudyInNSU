//
// Created by nastya on 05.12.15.
//

#ifndef PROXY_CACHEBUCKET_H
#define PROXY_CACHEBUCKET_H

#include <vector>
#include <utility>

class CacheBucket {
private:
    bool full;
    std::vector<std::pair<char *, int> > pagePieces;
public:

    CacheBucket();

    void setIsFull(bool isFull);

    void addItem(char *, int);

    unsigned long size();

    std::pair<char *, int> getItem(unsigned long position);

    bool isFull() const;
};


#endif //PROXY_CACHEBUCKET_H
