//
// Created by nastya on 15.12.15.
//

#ifndef MULTITHREADPROXY_CONNECTION_H
#define MULTITHREADPROXY_CONNECTION_H

#include <map>
#include <cstring>
#include "CacheBucket.h"
//#include "Proxy.h"

#define BUFSIZE 1024

enum ClientConnectionState {
    FROM_CACHE, FROM_SERVER
};

struct cmp_str {
    bool operator()(char const *a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

class ClientConnection {

public:
    ClientConnection(int clientSocket);

    bool readRequest();

    bool handleRequest(std::map<char *, CacheBucket *, cmp_str> &pMap);

    bool isRightRequest();

    bool isRightUrl();

    bool connectWithServer(char *remoteHost);

    int getServerSocket() const;

    const ClientConnectionState & getState() const;


    int getClientSocket() const;

    char * getBuf();

    int getByteInBuf() const;

    char * getUrl() const;

    CacheBucket * getBucket() const;

    void setByteInBuf(int byteInBuf);

    unsigned long getCurrentCachePosition() const;

    void incrementCachePosition();


    pthread_mutex_t * getByteInBufMutex() ;

    pthread_cond_t * getByteInBufCond() ;

private:
    int clientSocket;
    int serverSocket;
    char buf[BUFSIZE];
    int byteInBuf;
    char *url;
    ClientConnectionState state;
    CacheBucket *bucket;
    unsigned long currentCachePosition;
    pthread_mutex_t byteInBufMutex;
    pthread_cond_t byteInBufCond;

};


#endif //MULTITHREADPROXY_CONNECTION_H
