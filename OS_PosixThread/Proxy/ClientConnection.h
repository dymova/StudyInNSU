//
// Created by nastya on 04.12.15.
//

#ifndef PROXY_CLIENTCONNECTION_H
#define PROXY_CLIENTCONNECTION_H

#include "CacheBucket.h"

#define BUFSIZE 1024

enum ClientConnectionState {NEW_CONNECTION, FROM_CACHE, FROM_SERVER, ERROR};

class ClientConnection {


public:
    ClientConnection(int clientSocket);

    char *getBuf() const;
    int getByteInBuf();
    int setByteInBuf(int count);
    int getClientSocket() const;
    const ClientConnectionState &getState() const;
    char *getUrl() const;


    void setUrl(char *url);

    void setState(const ClientConnectionState &state);


    CacheBucket *getBucket() const {
        return bucket;
    }

    void setBucket(CacheBucket *bucket) {
        ClientConnection::bucket = bucket;
    }

    ~ClientConnection();

private:
    char buf[BUFSIZE];
    int byteInBuf;
    int clientSocket;
    char* url;
    CacheBucket* bucket;
    int currentCachePosition;

    ClientConnectionState state;

public:
    int getCurrentCachePosition() const {
        return currentCachePosition;
    }
};


#endif //PROXY_CLIENTCONNECTION_H
