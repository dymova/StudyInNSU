//
// Created by nastya on 15.12.15.
//

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include "ServerConnection.h"


ServerConnection::ServerConnection(int serverSocket, ClientConnection *c, char* url, int byteInBuf) {
    state = NOT_CACHING_MODE;
    this->serverSocket = serverSocket;
    memset(buf, 0, sizeof(buf));
    this->clientConnection = c;
    cacheBucket = NULL;

    memcpy(buf, url, (size_t) byteInBuf);
    this->byteInBuf = byteInBuf;

}


const ServerConnectionState &ServerConnection::getState() const {
    return state;
}

int ServerConnection::getServerSocket() const {
    return serverSocket;
}


char *ServerConnection::getBuf() {
    return buf;
}

ClientConnection *ServerConnection::getClientConnection() {
    return clientConnection;
}

int ServerConnection::setByteInBuf(int byteInBuf) {
    ServerConnection::byteInBuf = byteInBuf;
    return byteInBuf;
}


void ServerConnection::setCacheBucket(CacheBucket *cacheBucket) {
    ServerConnection::cacheBucket = cacheBucket;

}

CacheBucket *ServerConnection::getCacheBucket() const {
    return cacheBucket;

}

bool ServerConnection::sendRequest() {

//    int res = (int) write(serverSocket, clientConnection->getBuf(),
//                          (size_t) clientConnection->getByteInBuf());

    int res = (int) write(serverSocket, buf,
                          (size_t) byteInBuf);
    byteInBuf = 0;

//    int res = (int) send(serverSocket, clientConnection->getBuf(),
//                         (size_t) clientConnection->getByteInBuf(), MSG_NOSIGNAL);
    if (res == -1) {
        return false;
    } else {
        printf(">>ClientToServer: %s\n", buf);
        byteInBuf = 0;
        memset(buf, 0, BUFSIZE);
    }
    return true;
}

bool ServerConnection::receiveResponse() {
    if (0 == (byteInBuf = (int) read(serverSocket, buf,
                                         BUFSIZE))) {
        return false;
    }
    return true;
}

void ServerConnection::handleAnswer() {
    if (strstr(buf, "200") != NULL) {
        state = CACHING_MODE;


    } else {
        state = NOT_CACHING_MODE;
    }
}

void ServerConnection::saveDataToCache() const {
    char *newItem = (char *) malloc(byteInBuf * sizeof(char));
    memcpy(newItem, buf, (size_t) byteInBuf);
    cacheBucket->addItem(newItem, byteInBuf);
}

void ServerConnection::copyDataToClientBuf() {
    pthread_mutex_lock(clientConnection->getByteInBufMutex());
    while (clientConnection->getByteInBuf() != 0) {
        pthread_cond_wait(clientConnection->getByteInBufCond(), clientConnection->getByteInBufMutex());
    }
    memcpy(clientConnection->getBuf(), buf, (size_t) byteInBuf);
    clientConnection->setByteInBuf(byteInBuf);
    memset(buf, 0, BUFSIZE);
    byteInBuf = 0;
    pthread_cond_signal(clientConnection->getByteInBufCond());
    pthread_mutex_unlock(clientConnection->getByteInBufMutex());
}
