//
// Created by nastya on 15.12.15.
//

#include <malloc.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include "ThreadsBodies.h"
#include "Proxy.h"
#include "ServerConnection.h"

void *exitClientConnectionThread(ClientConnection * c);

void exitServerConnectionThread(ServerConnection *pConnection);

bool createServerThread(int serverSocket, CacheStorage *cache, ClientConnection *pConnection);

void *serverConnectionThreadBody(void *args);

void *clientConnectionThreadBody(void *args) {
    ClientThreadArgs *threadArgs = (ClientThreadArgs *) args;
    int clientSocket = threadArgs->clientSocket;
    CacheStorage* cacheStorage = threadArgs->cacheStorage;

    ClientConnection *c = new ClientConnection(clientSocket);

    if (!c->readRequest()) {
        exitClientConnectionThread(c);
    }

    if (!c->handleRequest(cacheStorage)) {
        exitClientConnectionThread(c);
    }

    if (c->getState() == FROM_SERVER) {
        createServerThread(c->getServerSocket(), cacheStorage, c);
    }
    c->setByteInBuf(0);


    while (true) {
        if (c->getState() == FROM_CACHE) {
            if (c->getCurrentCachePosition() < c->getBucket()->size()) {
                std::pair<char *, int> pair = c->getBucket()->getItem(c->getCurrentCachePosition());
                int res = (int) write(c->getClientSocket(), pair.first, (size_t) pair.second);
//                int res = (int) send(c->getClientSocket(), pair.first, (size_t) pair.second, MSG_NOSIGNAL);
                if (res == -1) {
                    exitClientConnectionThread(c);
                } else {
                    c->incrementCachePosition();
                    if (c->getCurrentCachePosition() == c->getBucket()->size() && c->getBucket()->isFull()) {
                        exitClientConnectionThread(c);
                    }
                }

            } else if(c->getCurrentCachePosition() == c->getBucket()->size() && !c->getBucket()->isFull()) {
                c->getBucket()->waitData(c->getCurrentCachePosition());
            }
        } else if (c->getState() == FROM_SERVER) {
            pthread_mutex_lock(c->getByteInBufMutex());
            while(c->getByteInBuf() == 0) {
                pthread_cond_wait(c->getByteInBufCond(), c->getByteInBufMutex());
            }
            int res = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());
//            int res = (int) send(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf(), MSG_NOSIGNAL);
            if (res == -1) {
                printf("write error: %s\n", strerror(errno));
                exitClientConnectionThread(c);
            } else {
                c->setByteInBuf(0);
                memset(c->getBuf(), 0, sizeof(c->getBuf()));
            }
            pthread_cond_signal(c->getByteInBufCond());
            printf("<<ServerTOClient%d:%d: %s\n", c->getServerSocket(), c->getByteInBuf(), c->getBuf());
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

    args->byteInBuf = pConnection->getByteInBuf();
    args->url = (char*) malloc(args->byteInBuf * sizeof(char));
    memcpy(args->url, pConnection->getBuf(), args->byteInBuf);



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

    char*savedUrl =  threadArgs->url;
    int byteInBuf = threadArgs->byteInBuf;


    ServerConnection *sc = new ServerConnection(serverSocket, clientConnection, savedUrl, byteInBuf);

    if (!sc->sendRequest()) {
        exitServerConnectionThread(sc);
    }

    std::cout << "send request" << std::endl;


    if (!sc->receiveResponse()) {
        exitServerConnectionThread(sc);
    }
    std::cout << "receive response" << std::endl;

    sc->handleAnswer();
    std::cout << "handle answer" << std::endl;

    int byteInResponse = sc->getByteInBuf();

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
    std::cout << "send response" << std::endl;

    if(byteInResponse < BUFSIZE) {
        exitServerConnectionThread(sc);
    }

    while (true) {
        if(sc->getState() == CACHING_MODE) {
            if (0 >= (sc->setByteInBuf((int) read(sc->getServerSocket(), sc->getBuf(),
                                                 BUFSIZE)))) {
                std::cout << "read return 0 in cahching mode handle" << std::endl;
                if(byteInBuf == 0) {
                    sc->getCacheBucket()->setIsFull(true);
                }
                exitServerConnectionThread(sc);
            } else {
                sc->saveDataToCache();
                sc->copyDataToClientBuf();
            }
        } else if(sc->getState() == NOT_CACHING_MODE) {
            if (0 >= (sc->setByteInBuf((int) read(sc->getServerSocket(), sc->getBuf(),
                                                 BUFSIZE)))) {
                perror("read from socket in not_caching_mode");
                printf("read error: %s\n", strerror(errno));

                exitServerConnectionThread(sc);
            } else {
                sc->copyDataToClientBuf();
            }
        }
//        if(sc->getByteInBuf() < BUFSIZE) {
//            exitServerConnectionThread(sc);
//        }

    }

}

void exitServerConnectionThread(ServerConnection *c) {
    close(c->getClientConnection()->getClientSocket());
    free(c->getClientConnection());
    close(c->getServerSocket());
    free(c);
    std::cout << "drop server connection" << std::endl;
    pthread_exit(NULL);
}


void *exitClientConnectionThread(ClientConnection *c) {
    close(c->getClientSocket());
    c->setState(CLIENT_EXIT);
    pthread_mutex_lock(c->getByteInBufMutex());
    pthread_cond_signal(c->getByteInBufCond());
    pthread_mutex_unlock(c->getByteInBufMutex());
    std::cout << "drop client connection" << std::endl;
    pthread_exit(NULL);
}

