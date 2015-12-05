//
// Created by nastya on 05.12.15.
//

#include "CacheBucket.h"

CacheBucket::CacheBucket() {
    isFull = false;
}

void CacheBucket::addItem(char *buf, int size) {
    pagePieces.push_back(std::make_pair(buf, size));
}

unsigned long CacheBucket::size() {
    return pagePieces.size();
}
