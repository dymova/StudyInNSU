#include <string.h>
#include "ServerConnection.h"


const ServerConnectionState &ServerConnection::getState() const {
    return state;
}


int ServerConnection::getServerSocket() const {
    return serverSocket;
}

char *ServerConnection::getBuf() {
    return buf;
}

int ServerConnection::getByteInBuf() {
    return byteInBuf;
}

ServerConnection::ServerConnection(int serverSocket, ClientConnection *c) {
    state = NEW_SERVER_CONNECTION;
    this->serverSocket = serverSocket;
    memset(buf, 0, sizeof(buf));
    this->clientConnection = c;
    cacheBucket = NULL;
}

ClientConnection *ServerConnection::getClientConnection() {
    return clientConnection;
}

int ServerConnection::setByteInBuf(int byteInBuf) {
    ServerConnection::byteInBuf = byteInBuf;
    return byteInBuf;
}

void ServerConnection::setState(const ServerConnectionState &state) {
    ServerConnection::state = state;

}

void ServerConnection::setCacheBucket(CacheBucket *cacheBucket) {
    ServerConnection::cacheBucket = cacheBucket;

}

CacheBucket *ServerConnection::getCacheBucket() const {
    return cacheBucket;

}
