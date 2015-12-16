//
// Created by nastya on 15.12.15.
//

#include "CacheBucket.h"

CacheBucket::CacheBucket() {
    full = false;
}

void CacheBucket::addItem(char *buf, int size) {
    pagePieces.push_back(std::make_pair(buf, size));
}

unsigned long CacheBucket::size() {
    return pagePieces.size();
}

std::pair<char *, int> CacheBucket::getItem(unsigned long position) {
    return pagePieces.at(position);
}

bool CacheBucket::isFull() const {
    return full;
}

void CacheBucket::setIsFull(bool isFull) {
    CacheBucket::full = isFull;
}
