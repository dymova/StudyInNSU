#ifndef MULTITHREADPROXY_CACHEBUCKET_H
#define MULTITHREADPROXY_CACHEBUCKET_H
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


#endif //MULTITHREADPROXY_CACHEBUCKET_H
