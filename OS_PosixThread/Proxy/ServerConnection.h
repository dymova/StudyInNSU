#ifndef PROXY_SERVERCONNECTION_H
#define PROXY_SERVERCONNECTION_H

#include "ClientConnection.h"
#include "CacheBucket.h"

#define BUFSIZE 1024

enum ServerConnectionState {
    NEW_SERVER_CONNECTION, EXPECTED_RESPONSE, CACHING_MODE, NOT_CACHING_MODE, SERVER_ERROR
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

    int getByteInBuf();

    int setByteInBuf(int byteInBuf);

    void setState(const ServerConnectionState &state);


    void setCacheBucket(CacheBucket *cacheBucket);


    CacheBucket *getCacheBucket() const;
};


#endif //PROXY_SERVERCONNECTION_H
