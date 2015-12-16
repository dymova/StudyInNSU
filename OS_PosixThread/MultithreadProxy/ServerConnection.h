//
// Created by nastya on 15.12.15.
//

#ifndef MULTITHREADPROXY_SERVERCONNECTION_H
#define MULTITHREADPROXY_SERVERCONNECTION_H


#include "ClientConnection.h"

#define BUFSIZE 1024

enum ServerConnectionState {
    CACHING_MODE, NOT_CACHING_MODE
};

class ServerConnection {
private:
    ServerConnectionState state;
    int serverSocket;
    char buf[BUFSIZE];
    int byteInBuf;
    ClientConnection *clientConnection;
    CacheBucket *cacheBucket;

public:

    ServerConnection(int serverSocket, ClientConnection *c);

    const ServerConnectionState &getState() const;

    int getServerSocket() const;

    char *getBuf();


    ClientConnection *getClientConnection();

    int setByteInBuf(int byteInBuf);

    void setCacheBucket(CacheBucket *cacheBucket);


    CacheBucket *getCacheBucket() const;

    bool sendRequest();

    bool receiveResponse();

    void handleAnswer();

    void saveDataToCache() const;

    void copyDataToClientBuf();

    };


#endif //MULTITHREADPROXY_SERVERCONNECTION_H
