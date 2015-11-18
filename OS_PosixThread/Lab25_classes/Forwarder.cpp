//
// Created by nastya on 15.11.15.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include <sys/time.h>
#include <sys/types.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "Forwarder.h"

Forwarder::Forwarder(char* listenPortAsString, char *remoteHost, char* remotePortAsString) {
    maxfd = 0;

    int listenPort = atoi(listenPortAsString);
    if (listenPort <= 0) {
        throw new IllegalArgumentException(std::string(listenPortAsString));
    }

    int remotePort = atoi(remotePortAsString);
    if (remotePort <= 0) {
        throw new IllegalArgumentException(std::string(listenPortAsString));
    }

    struct addrinfo *remoteInfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(remoteHost, remotePortAsString, &hint, &remoteInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        throw new ConstructorForwarderException(std::string("getaddrinfo for remote host"));
    }
    serverAddr = (struct sockaddr_in *) remoteInfo->ai_addr;


    // преобразуем IP в строку и выводим его:
    void *addr = &(serverAddr->sin_addr);
    char buf[40];
    inet_ntop(remoteInfo->ai_family, addr, buf, sizeof buf);
    printf("\n %s", buf);


    hint.ai_flags = AI_PASSIVE;
    struct addrinfo *listenInfo;
    if ((status = getaddrinfo(NULL, listenPortAsString, &hint, &listenInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        throw new ConstructorForwarderException(std::string("getaddrinfo for listening"));
    }

    listenSocket = socket(listenInfo->ai_family,
                              listenInfo->ai_socktype,
                              listenInfo->ai_protocol);
    if(-1 == checkSocket(listenSocket, &maxfd)) {
        throw new ConstructorForwarderException(std::string("checkSocket"));
    }

    int val = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    if (bind(listenSocket, listenInfo->ai_addr, listenInfo->ai_addrlen)) {
        throw new ConstructorForwarderException(std::string("bind listen socket"));
    }

    if ((listen(listenSocket, SOMAXCONN)) == -1) {
        throw new ConstructorForwarderException(std::string("listen"));
    }

}

void Forwarder::start() {

    for (; ;) {
        fillMasksForSelect(&readfs, &writefs, listenSocket, head);
        if ((readyFdCount = select(maxfd + 1, &readfs, &writefs, NULL, NULL)) == -1) {
            throw new StartForwarderException(std::string("select"));
        }

        checkReadfsAndWritefs(&readfs, &writefs, head);

        struct sockaddr_in cliaddr;
        if (FD_ISSET(listenSocket, &readfs)) {
            socklen_t addrlen = sizeof(cliaddr);
            int newClientFd = accept(listenSocket, (struct sockaddr *) &cliaddr, &addrlen);
            if (newClientFd < 0) {
                throw new StartForwarderException(std::string("accept"));
            }
            if(-1 == checkSocket(newClientFd, &maxfd)) {
                throw new StartForwarderException(std::string("checkSocket"));
            }
            if (NULL == addConnection(newClientFd, serverAddr, &maxfd)) {
                throw new StartForwarderException(std::string("addConnection"));
            }
        }
    }



}

void Forwarder::fillMasksForSelect(fd_set *readfs, fd_set *writefs, int listenSocket, Connection *pHead) {
    FD_ZERO(readfs);
    FD_ZERO(writefs);
    FD_SET(listenSocket, readfs);

    Connection *c = pHead;
    while (c) {
        if ((c->getSizeClientToServer() < 0 && c->getSizeServerToClient() <= 0) ||
            (c->getSizeServerToClient() < 0 && c->getSizeClientToServer() <= 0)) {
            dropConnection(c);
        } else {
            if (c->getSizeClientToServer() == 0) {
                FD_SET(c->getClientSocket(), readfs);
            }
            if (c->getSizeServerToClient() == 0) {
                FD_SET(c->getServerSocket(), readfs);
            }
            if (c->getSizeClientToServer() > 0) {
                FD_SET(c->getServerSocket(), writefs);
            }
            if (c->getSizeServerToClient() > 0) {
                FD_SET(c->getClientSocket(), writefs);
            }
        }
        c = c->getNext();
    }
}

int Forwarder::checkSocket(int socketId, int *maxFd) {
    if (socketId >= FD_SETSIZE) {
        fprintf(stderr, "Socket number out of range\n");
        return -1; //todo exit
    }
    if (socketId > *maxFd) {
        *maxFd = socketId;
    }
    return 0;
}

void Forwarder::checkReadfsAndWritefs(fd_set *readfs, fd_set *writefs, Connection *pHead) {
    Connection *c = pHead;
    while (c) {
        if (c->getSizeClientToServer() == 0 && FD_ISSET(c->getClientSocket(), readfs)) {
            if (0 == (c->sizeClientToServer = (int) read(c->getClientSocket(), c->bufClientToServer,
                                                         sizeof(c->bufClientToServer)))) {
                c->sizeClientToServer = -1;
            }
        }
        if (c->getSizeServerToClient() == 0 && FD_ISSET(c->getServerSocket(), readfs)) {
            if (0 == (c->sizeServerToClient = (int) read(c->getServerSocket(), c->bufServerToClient,
                                                         sizeof(c->bufServerToClient)))) {
                c->sizeServerToClient = -1;
            }
        }
        if (c->sizeClientToServer > 0 && FD_ISSET(c->getServerSocket(), writefs)) {
            int res = (int) write(c->getServerSocket(), c->bufClientToServer, (size_t) c->sizeClientToServer);
            if (res == -1) {
                c->sizeServerToClient = -1;
            } else {
                c->sizeClientToServer = 0;
            }
        }
        if (c->sizeServerToClient > 0 && FD_ISSET(c->getClientSocket(), writefs)) {
            int res = (int) write(c->getClientSocket(), c->bufServerToClient, (size_t) c->sizeServerToClient);
            if (res == -1) {
                c->sizeClientToServer = -1;
            } else {
                c->sizeServerToClient = 0;
            }
        }
        c = c->getNext();
    }
}

void Forwarder::dropConnection(Connection *c) {
    if (c == head && c == tail) {
        head = NULL;
        tail = NULL;
    } else if (c == head) {
        head = c->getNext();
        head->setPrev(NULL);
    } else if (c == tail) {
        tail = c->getPrev();
        tail->setNext(NULL);
    } else {
        c->getNext()->setPrev(c->getPrev());
        c->getPrev()->setNext(c->getNext());
    }
    close(c->getServerSocket());
    close(c->getClientSocket());
    free(c);
}

Connection *Forwarder::addConnection(int clientSocket, struct sockaddr_in *serverAddr, int *maxFd) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == checkSocket(serverSocket, maxFd))  {
        return NULL;
    }
    Connection *c = new Connection(clientSocket, serverSocket );

    if (-1 == (connect(c->getServerSocket(), (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
        perror("connent to server");
        close(c->getServerSocket());
        delete(c);
        return NULL;
    }

    c->setPrev(NULL);
    c->setNext(head);
    if (head == NULL) {
        tail = c;
    } else {
        head->setPrev(c);
    }
    head = c;
    c->sizeServerToClient = 0;
    c->sizeClientToServer = 0;
    memset(c->bufClientToServer, 0, sizeof(c->bufClientToServer));
    memset(c->bufServerToClient, 0, sizeof(c->bufServerToClient));

    return c;
}
