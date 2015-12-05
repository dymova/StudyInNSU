//
// Created by nastya on 05.12.15.
//

#ifndef PROXY_SERVERCONNECTION_H
#define PROXY_SERVERCONNECTION_H

#include "ClientConnection.h"
#include "CacheBucket.h"

#define BUFSIZE 1024

enum ServerConnectionState {NEW_SERVER_CONNECTION, EXPECTED_RESPONSE, CACHING_MODE, NOT_CACHING_MODE, SERVER_ERROR};

class ServerConnection {
private:
    ServerConnectionState state;
    int serverSocket;
    char buf[BUFSIZE];
    int byteInBuf;
    ClientConnection* clientConnection;
    CacheBucket* cacheBucket;



public:

    ServerConnection(int serverSocket, ClientConnection *c);

    const ServerConnectionState &getState() const;
    int getServerSocket() const ;

    char *getBuf() ;


//    const char *getBuf() const {
//        return buf;
//    }

    ClientConnection *getClientConnection() {
        return clientConnection;
    }

    int getByteInBuf();

    int setByteInBuf(int byteInBuf) {
        ServerConnection::byteInBuf = byteInBuf;
        return byteInBuf;
    }

    void setState(const ServerConnectionState &state) {
        ServerConnection::state = state;
    }


    void setCacheBucket(CacheBucket *cacheBucket) {
        ServerConnection::cacheBucket = cacheBucket;
    }

    CacheBucket *getCacheBucket() const {
        return cacheBucket;
    }
};


#endif //PROXY_SERVERCONNECTION_H
