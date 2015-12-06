#ifndef PROXY_CLIENTCONNECTION_H
#define PROXY_CLIENTCONNECTION_H

#include "CacheBucket.h"

#define BUFSIZE 1024

enum ClientConnectionState {
    NEW_CONNECTION, FROM_CACHE, FROM_SERVER, CLIENT_ERROR
};

class ClientConnection {

public:
    ClientConnection(int clientSocket);

    char *getBuf();

    int getByteInBuf();

    int setByteInBuf(int count);

    int getClientSocket() const;

    const ClientConnectionState &getState() const;

    char *getUrl() const;


    void setUrl(char *url);

    void setState(const ClientConnectionState &state);


    CacheBucket *getBucket() const;

    void setBucket(CacheBucket *bucket);

    unsigned long getCurrentCachePosition() const;

    void incrementCachePosition();

private:
    char buf[BUFSIZE];
    int byteInBuf;
    int clientSocket;
    char *url;
    CacheBucket *bucket;
    unsigned long currentCachePosition;

    ClientConnectionState state;


};


#endif //PROXY_CLIENTCONNECTION_H
