//
// Created by nastya on 15.12.15.
//

#ifndef MULTITHREADPROXY_CONNECTION_H
#define MULTITHREADPROXY_CONNECTION_H

#include <map>
#include <cstring>
#include "CacheBucket.h"
#include "CacheStorage.h"
//#include "Proxy.h"

//#define BUFSIZE 1024
#define BUFSIZE 4096

enum ClientConnectionState {
    FROM_CACHE, FROM_SERVER, CLIENT_EXIT, CLIENT_ERROR
};



class ClientConnection {

public:
    ClientConnection(int clientSocket);

    bool readRequest();

    bool handleRequest(CacheStorage *cacheStorage);

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


    void setState(const ClientConnectionState &state);

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
