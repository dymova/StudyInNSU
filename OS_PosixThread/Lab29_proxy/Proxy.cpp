#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include	<time.h>
#include	<netinet/in.h>
#include	<fcntl.h>
#include	<netdb.h>
//#include	<signal.h>
#include	<string.h>
#include	<sys/uio.h>
//#include    <assert.h>

#include "Proxy.h"
#include "CacheBucket.h"

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

    std::list<Connection*> removedConnections;
    for (std::list<Connection*>::iterator iterator = connections.begin(); iterator != connections.end(); ++iterator) {
        Connection * c = *iterator;
        if ((c->sizeClientToServer < 0 && c->sizeServerToClient <= 0) ||
            (c->sizeServerToClient < 0 && c->sizeClientToServer <= 0)) {
            removedConnections.push_back(c);

        } else {
            if (c->sizeClientToServer == 0) {
                FD_SET(c->clientSocket, &readfs);
            }
            if (c->sizeServerToClient >= 0) {
                FD_SET(c->clientSocket, &writefs);
            }
            if (c->serverSocket != -1) {
                if (c->sizeServerToClient <= 0) { // для докачки страницы
//                if (c->sizeServerToClient == 0) {
                    FD_SET(c->serverSocket, &readfs);
                }
                if (c->sizeClientToServer > 0) {
                    FD_SET(c->serverSocket, &writefs);
                }
            }
        }
    }
    for (std::list<Connection*>::iterator list_iterator = removedConnections.begin(); list_iterator != removedConnections.end(); ++list_iterator) {
        Connection * removedConnection = *list_iterator;
        close(removedConnection->clientSocket);
        close(removedConnection->serverSocket);
        connections.remove(removedConnection);
        std::cout<< "drop connection" <<std::endl;

    }
}

void Proxy::checkReadfsAndWritefs() {
    for (std::list<Connection*>::iterator iterator = connections.begin(); iterator != connections.end(); ++iterator) {
        Connection *  c = *iterator;
        if (c->sizeClientToServer == 0 && FD_ISSET(c->clientSocket, &readfs)) {
            if (0 == (c->sizeClientToServer = (int) read(c->clientSocket, c->bufClientToServer,
                                                         sizeof(c->bufClientToServer)))) {
                c->sizeClientToServer = -1;
            }
            if(!c->requestHandled) {
                handleRequest(c);

            }
        }
        if(c->fromCache) {
            if(FD_ISSET(c->clientSocket, &writefs)){
                if(c->currentCashPosition < c->bucket->pagePieces.size()){
                    char* currentBuf =  c->bucket->pagePieces[c->currentCashPosition];

                    size_t size = strlen(currentBuf);
                    int res = (int) write(c->clientSocket, currentBuf, (size_t) size); //todo read length buffer
                    if (res == -1) {
                        c->sizeClientToServer = -1;
                    } else {
                        c->currentCashPosition++;
                        if(c->currentCashPosition == c->bucket->pagePieces.size() && c->bucket->isFull) {
                            c->sizeClientToServer = -1;
                        }
                    }
                }
            }
        } else {
            if ( c->sizeClientToServer > 0 && FD_ISSET(c->serverSocket, &writefs)) {
                int res = (int) write(c->serverSocket, c->bufClientToServer, (size_t) c->sizeClientToServer);
                if (res == -1) {
                    c->sizeServerToClient = -1;
                } else {
                    c->sizeClientToServer = 0;
                }
                printf(">>ClientToServer: %s\n", c->bufClientToServer);
            }
            if (c->sizeServerToClient == 0 && FD_ISSET(c->serverSocket, &readfs)) {
                if (0 == (c->sizeServerToClient = (int) read(c->serverSocket, c->bufServerToClient,
                                                             sizeof(c->bufServerToClient)))) {
                    c->sizeServerToClient = -1;
                    if(c->bucket != NULL) {
                        c->bucket->isFull = true;
                    }
                } else {
                    if(!c->answerHandled) {
                        handleAnswer(c);
                    }
                    if(c->cachingMode) {
                        char * newItem = (char*) malloc(c->sizeServerToClient * sizeof(char));
                        memcpy(newItem, c->bufServerToClient, (size_t) c->sizeServerToClient);
                        c->bucket->pagePieces.push_back(newItem);
                    }
                }
                printf("<<ServerTOClient: %s\n", c->bufServerToClient);

            }
            if (c->sizeServerToClient > 0 && FD_ISSET(c->clientSocket, &writefs)) {
                int res = (int) write(c->clientSocket, c->bufServerToClient, (size_t) c->sizeServerToClient);
//                if (res == -1) {
//                    c->sizeClientToServer = -1;
//                } else {
//                    c->sizeServerToClient = 0;
//                }
                c->sizeServerToClient = 0;
            }
        }
    }
}

Connection *Proxy::addConnection(int clientSocket) {

    Connection *c = new Connection(clientSocket);

    connections.push_back(c);

    c->sizeServerToClient = 0;
    c->sizeClientToServer = 0;
    memset(c->bufClientToServer, 0, sizeof(c->bufClientToServer));
    memset(c->bufServerToClient, 0, sizeof(c->bufServerToClient));

    printf("new connection\n");


    return c;
}

void Proxy::handleAnswer(Connection *c) {
    if(strstr(c->bufServerToClient, "200") != NULL) {
        c->cachingMode = true;
        CacheBucket* bucket = new CacheBucket();
        cache.insert(std::pair<char*, CacheBucket*>(c->url, bucket));
        c->bucket = bucket;
    }

    c->answerHandled = true;
}


bool Proxy::checkRequest(Connection* c) {
    if(NULL == strstr(c->bufClientToServer, "GET") && NULL == strstr(c->bufClientToServer, "HEAD") ) {
        std::cout<< "not supported method" << std::endl;
        c->sizeClientToServer = -1;
        c->sizeServerToClient = -1;
        write(c->clientSocket, HTTP_405_ERROR, strlen(HTTP_405_ERROR));
        return false;
    }
    if(NULL == strstr(c->bufClientToServer, "HTTP/1.0")) {
        std::cout<< "not supported protocol" << std::endl;
        c->sizeClientToServer = -1;
        c->sizeServerToClient = -1;
        write(c->clientSocket, HTTP_505_ERROR, strlen(HTTP_405_ERROR));
        return false;
    }
    strcat(c->bufClientToServer, "\n\n"); //todo check if it needed?
    return true;
}

void Proxy::handleRequest(Connection* c) {
    std::cout<< c->bufClientToServer << std::endl;
    bool isRightRequest = checkRequest(c);
    if(!isRightRequest) {
        return;
    }
    char* url = (char*) calloc(BUFSIZE, sizeof(char));
    getUrl(c, url);
    c->url = url;

    char* host = (char*) calloc(BUFSIZE, sizeof(char));
    sscanf( url, "%*[^/]%*[/]%[^/]", host);
    printf("host: %s\n", host);

    c->requestHandled = true;

    //check cache
    if(cache.find(url) == cache.end()) {
        int status;
        if(-1 == (status = connectWithServer(c, host))) {
            std::cout<< "connect error" << std::endl;
            close(c->serverSocket);
            c->sizeClientToServer = -1;
            c->sizeServerToClient = -1;
        }
    } else {
        c->fromCache = true;
        c->bucket = cache.find(url)->second;
    }
}

void Proxy::getUrl(Connection* c, char url[]) const {
    char* tmp = strchr(c->bufClientToServer, ' ');
    if(tmp == NULL) {
        std::cout<< "bad request 1" << std::endl;
        //todo send error
    }
    char* tmp2 = strchr(tmp+1, ' ');
    if(tmp == NULL) {
        //todo send error
        std::cout<< "bad request 2" << std::endl;
    }
    int endIndex = (int) (tmp2 - tmp);

    strncpy(url, tmp+1, (size_t) endIndex - 1);


    printf("url: %s\n", url);
}

int Proxy::connectWithServer(Connection *c, char *remoteHost) {
    struct addrinfo *remoteInfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(remoteHost, HTTP_PORT, &hint, &remoteInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }
    struct sockaddr_in * serverAddr = (struct sockaddr_in *) remoteInfo->ai_addr;

    // преобразуем IP в строку и выводим его:
    void *addr = &(serverAddr->sin_addr);
    char buf[40];
    inet_ntop(remoteInfo->ai_family, addr, buf, sizeof buf);
    printf("ip: %s\n", buf);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == checkSocket(serverSocket))  {
        fprintf(stderr, "checkSocket");
        return -1;
    }

    c->serverSocket = serverSocket;

    if (-1 == (connect(c->serverSocket, (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
        perror("connent to server");
        return -1;
    }

    return 0;
}
