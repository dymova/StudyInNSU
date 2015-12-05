//
// Created by nastya on 05.12.15.
//

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
