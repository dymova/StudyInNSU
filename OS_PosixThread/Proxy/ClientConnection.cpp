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

int ClientConnection::getByteInBuf() {
    return byteInBuf;
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

CacheBucket *ClientConnection::getBucket() const {
    return bucket;

}

void ClientConnection::setBucket(CacheBucket *bucket) {
    ClientConnection::bucket = bucket;

}

unsigned long ClientConnection::getCurrentCachePosition() const {
    return currentCachePosition;
}
