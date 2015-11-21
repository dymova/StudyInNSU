//
// Created by nastya on 15.11.15.
//

#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include "Proxy.h"

Proxy::Proxy(char *listenPortAsString) {
    int listenPort = atoi(listenPortAsString);
    if (listenPort <= 0) {
        throw new IllegalArgumentException(std::string(listenPortAsString));
    }

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    int status;
    struct addrinfo *listenInfo;
    if ((status = getaddrinfo(NULL, listenPortAsString, &hint, &listenInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        throw new ConstructorProxyException(std::string("getaddrinfo for listening"));
    }

    listenSocket = socket(listenInfo->ai_family,
                          listenInfo->ai_socktype,
                          listenInfo->ai_protocol);
    if(-1 == checkSocket(listenSocket)) {
        throw new ConstructorProxyException("checkSocket");
    }

    int val = 1;
    if(-1 == setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int))) {
        throw ConstructorProxyException("setsockopt");
    }
    if (-1 == bind(listenSocket, listenInfo->ai_addr, listenInfo->ai_addrlen)) {
        throw ConstructorProxyException("bind listen socket");
    }

    if ((listen(listenSocket, SOMAXCONN)) == -1) {
        throw ConstructorProxyException("listen");
    }


}

int Proxy::checkSocket(int socketId) {
    if (socketId >= FD_SETSIZE) {
        fprintf(stderr, "Socket number out of range\n");
        return -1;
    }
    if (socketId > maxFd) {
        maxFd = socketId;
    }
    return 0;
}

void Proxy::start() {
    for (; ;) {
        fillMasksForSelect();
        if ((readyFdCount = select(maxFd + 1, &readfs, &writefs, NULL, NULL)) == -1) {
            throw StartProxyException(std::string("select"));
        }

        checkReadfsAndWritefs();

        struct sockaddr_in clientAddr;
        if (FD_ISSET(listenSocket, &readfs)) {
            socklen_t addrlen = sizeof(clientAddr);
            int newClientFd = accept(listenSocket, (struct sockaddr *) &clientAddr, &addrlen);
            if (newClientFd < 0) {
                throw StartProxyException("accept");
            }
            if(-1 == checkSocket(newClientFd)) {
                throw StartProxyException("checkSocket");
            }
            if (NULL == addConnection(newClientFd)) {
                throw StartProxyException("addConnection");
            }
        }
    }
}


void Proxy::fillMasksForSelect() {
    FD_ZERO(&readfs);
    FD_ZERO(&writefs);
    FD_SET(listenSocket, &readfs);

//    std::list<std::shared_ptr<Connection>> removedConnections;
//    for(auto& c: connections) {
//        if ((c->getSizeClientToServer() < 0 && c->getSizeServerToClient() <= 0) ||
//            (c->getSizeServerToClient() < 0 && c->getSizeClientToServer() <= 0)) {
//            removedConnections.push_back(c);
//        } else {
//            if (c->getSizeClientToServer() == 0) {
//                FD_SET(c->getClientSocket(), &readfs);
//            }
//            if (c->getSizeServerToClient() == 0) {
//                FD_SET(c->getServerSocket(), &readfs);
//            }
//            if (c->getSizeClientToServer() > 0) {
//                FD_SET(c->getServerSocket(), &writefs);
//            }
//            if (c->getSizeServerToClient() > 0) {
//                FD_SET(c->getClientSocket(), &writefs);
//            }
//        }
//    }
//    for (auto removedConnection : removedConnections) {
//        connections.remove(removedConnection);
//        std::cout<< "drop connection" <<std::endl;
//    }
}

void Proxy::checkReadfsAndWritefs() {
//    for (auto& c: connections) {
//        if (c->getSizeClientToServer() == 0 && FD_ISSET(c->getClientSocket(), readfs)) {
//            if (0 == (c->sizeClientToServer = (int) read(c->getClientSocket(), c->bufClientToServer,
//                                                         sizeof(c->bufClientToServer)))) {
//                c->sizeClientToServer = -1;
//            }
//        }
//        if (c->getSizeServerToClient() == 0 && FD_ISSET(c->getServerSocket(), readfs)) {
//            if (0 == (c->sizeServerToClient = (int) read(c->getServerSocket(), c->bufServerToClient,
//                                                         sizeof(c->bufServerToClient)))) {
//                c->sizeServerToClient = -1;
//            }
//            printf("<<ServerTOClient: %s\n", c->bufServerToClient);
//
//        }
//        if (c->sizeClientToServer > 0 && FD_ISSET(c->getServerSocket(), writefs)) {
//            int res = (int) write(c->getServerSocket(), c->bufClientToServer, (size_t) c->sizeClientToServer);
//            if (res == -1) {
//                c->sizeServerToClient = -1;
//            } else {
//                c->sizeClientToServer = 0;
//            }
//            printf(">>ClientToServer: %s\n", c->bufClientToServer);
//        }
//
//        if (c->sizeServerToClient > 0 && FD_ISSET(c->getClientSocket(), writefs)) {
//            int res = (int) write(c->getClientSocket(), c->bufServerToClient, (size_t) c->sizeServerToClient);
//            if (res == -1) {
//                c->sizeClientToServer = -1;
//            } else {
//                c->sizeServerToClient = 0;
//            }
//        }
//    }
}

Connection *Proxy::addConnection(int clientSocket) {
//    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if(-1 == checkSocket(serverSocket, maxFd))  {
//        return NULL;
//    }

//    if (-1 == (connect(c->getServerSocket(), (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
//        perror("connent to server");
//        close(c->getServerSocket());
//        delete(c);
//        return NULL;
//    }
    Connection *c = new Connection(clientSocket);

    connections.push_back(std::shared_ptr<Connection>(c));

    c->sizeServerToClient = 0;
    c->sizeClientToServer = 0;
    memset(c->request, 0, sizeof(c->request));
    memset(c->bufServerToClient, 0, sizeof(c->bufServerToClient));

    printf("\nnew connection\n");

    return c;
}
