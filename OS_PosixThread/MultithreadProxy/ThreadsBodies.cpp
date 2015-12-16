//
// Created by nastya on 15.12.15.
//

#include <malloc.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include "ThreadsBodies.h"
#include "Proxy.h"
#include "ClientConnection.h"
#include "ServerConnection.h"

void *exitClientConnectionThread(int clientSocket);

void exitServerConnectionThread(ServerConnection *pConnection);

bool createServerThread(int serverSocket, CacheStorage *cache, ClientConnection *pConnection);

void *serverConnectionThreadBody(void *args);

void *clientConnectionThreadBody(void *args) {
    ClientThreadArgs *threadArgs = (ClientThreadArgs *) args;
    int clientSocket = threadArgs->clientSocket;
    CacheStorage* cacheStorage = threadArgs->cacheStorage;
//    std::map<char *, CacheBucket *, cmp_str> cache = threadArgs->cache;

    ClientConnection *c = new ClientConnection(clientSocket);

    if (!c->readRequest()) {
        exitClientConnectionThread(clientSocket);
    }

    if (!c->handleRequest(cacheStorage)) {
        exitClientConnectionThread(clientSocket);
    }

    if (c->getState() == FROM_SERVER) {
        createServerThread(c->getServerSocket(), cacheStorage, c);
    }
    c->setByteInBuf(0);

//    while (true) {
    for (int i = 0; i < 10; ++i) {
        if (c->getState() == FROM_CACHE) {
            if (c->getCurrentCachePosition() < c->getBucket()->size()) {
                std::pair<char *, int> pair = c->getBucket()->getItem(c->getCurrentCachePosition());
                int res = (int) write(c->getClientSocket(), pair.first, (size_t) pair.second);
//                int res = (int) send(c->getClientSocket(), pair.first, (size_t) pair.second, MSG_NOSIGNAL);
                if (res == -1) {
                    exitClientConnectionThread(c->getClientSocket());
                } else {
                    c->incrementCachePosition();
                    if (c->getCurrentCachePosition() == c->getBucket()->size() && c->getBucket()->isFull()) {
                        exitClientConnectionThread(c->getClientSocket());
                    }
                }

            }
        } else if (c->getState() == FROM_SERVER) {
            pthread_mutex_lock(c->getByteInBufMutex());
            while(c->getByteInBuf() == 0) {
                pthread_cond_wait(c->getByteInBufCond(), c->getByteInBufMutex());
            }
            int res = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());
//            int res = (int) send(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf(), MSG_NOSIGNAL);
            if (res == -1) {
                exitClientConnectionThread(c->getClientSocket());
            } else {
                c->setByteInBuf(0);
            }
            pthread_cond_signal(c->getByteInBufCond());
            printf("<<ServerTOClient: %s\n", c->getBuf());
            pthread_mutex_unlock(c->getByteInBufMutex());
        }
    }

    free(args);
}

bool createServerThread(int serverSocket, CacheStorage *cache, ClientConnection *pConnection) {
    ServerThreadArgs *args = (ServerThreadArgs *) malloc(sizeof(ServerThreadArgs));
    args->serverSocket = serverSocket;
    args->cacheStorage = cache;
    args->clientConnection = pConnection;


    args->url = pConnection->getBuf();
    args->byteInBuf = pConnection->getByteInBuf();



    pthread_t thread;
    int code;
    code = pthread_create(&thread, NULL, serverConnectionThreadBody, args);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "creating thread:%s\n", buf);
        return false;
    }
    return true;

}

void *serverConnectionThreadBody(void *args) {
    ServerThreadArgs *threadArgs = (ServerThreadArgs *) args;
    int serverSocket = threadArgs->serverSocket;
    CacheStorage* cacheStorage = threadArgs->cacheStorage;
    ClientConnection *clientConnection = threadArgs->clientConnection;
    char* url = threadArgs->url;
    int byteInBuf = threadArgs->byteInBuf;

    ServerConnection *sc = new ServerConnection(serverSocket, clientConnection, url, byteInBuf);

    if (!sc->sendRequest()) {
        exitServerConnectionThread(sc);
    }

    if (!sc->receiveResponse()) {
        exitServerConnectionThread(sc);
    }

    sc->handleAnswer();

    if (sc->getState() == CACHING_MODE) {
        CacheBucket *bucket = new CacheBucket();
        cacheStorage->insertNewBucket(std::pair<char *, CacheBucket *>
                                              (sc->getClientConnection()->getUrl(),
                                               bucket));

        sc->setCacheBucket(bucket);
        sc->saveDataToCache();
        sc->copyDataToClientBuf();
    } else {
        sc->copyDataToClientBuf();
    }

//    while (true) {
    for (int i = 0; i < 10; ++i) {
        if(sc->getState() == CACHING_MODE) {
            if (0 == (sc->setByteInBuf((int) read(sc->getServerSocket(), sc->getBuf(),
                                                 BUFSIZE)))) {
                exitServerConnectionThread(sc);
                sc->getCacheBucket()->setIsFull(true);
            } else {
                sc->saveDataToCache();
                sc->copyDataToClientBuf();
            }
        } else if(sc->getState() == NOT_CACHING_MODE) {
            if (0 == (sc->setByteInBuf((int) read(sc->getServerSocket(), sc->getBuf(),
                                                 BUFSIZE)))) {
             exitServerConnectionThread(sc);
            } else {
                sc->copyDataToClientBuf();
            }
        }
    }

}

void exitServerConnectionThread(ServerConnection *c) {
    close(c->getClientConnection()->getClientSocket());
    close(c->getServerSocket());
    std::cout << "drop server connection" << std::endl;
    pthread_exit(NULL);
}


void *exitClientConnectionThread(int clientSocket) {
    close(clientSocket);
    std::cout << "drop client connection" << std::endl;
    pthread_exit(NULL);
}

