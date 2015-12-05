//
// Created by nastya on 04.12.15.
//

#include <string.h>
#include "ClientConnection.h"

ClientConnection::ClientConnection(int clientSocket) {
    this->clientSocket = clientSocket;
    memset(buf, 0, sizeof(buf));
    state = NEW_CONNECTION;
    byteInBuf = 0;
    url = NULL;
    bucket = NULL;
    currentCachePosition = 0;
}


const ClientConnectionState &ClientConnection::getState() const {
    return state;
}

char *ClientConnection::getBuf() {
    return buf;
}

int ClientConnection::getClientSocket() const {
    return clientSocket;
}

int ClientConnection::getByteInBuf(){
    return byteInBuf;
}




ClientConnection::~ClientConnection() {
    //todo free host and url but think about cache
}

void ClientConnection::setUrl(char *url) {
    ClientConnection::url = url;
}

char *ClientConnection::getUrl() const {
    return url;
}

void ClientConnection::setState(const ClientConnectionState &state) {
    ClientConnection::state = state;
}

int ClientConnection::setByteInBuf(int count) {
    byteInBuf = count;
    return count;
}

void ClientConnection::incrementCachePosition() {
    currentCachePosition++;
}
