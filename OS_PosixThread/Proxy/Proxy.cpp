#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "Proxy.h"
#include "ServerConnection.h"

Proxy::Proxy(char *listenPortAsString) {

    int listenPort = atoi(listenPortAsString);
    if (listenPort <= 0) {
        throw new IllegalArgumentException(listenPortAsString);
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
    if (-1 == checkSocket(listenSocket)) {
        throw new ConstructorProxyException("checkSocket");
    }

    int val = 1;
    if (-1 == setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int))) {
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
        if (select(maxFd + 1, &readfs, &writefs, NULL, NULL) == -1) {
            throw StartProxyException(std::string("select"));
        }

        checkClientsReadfsAndWritefs();
        checkServersReadfsAndWritefs();

        struct sockaddr_in clientAddr;
        if (FD_ISSET(listenSocket, &readfs)) {
            socklen_t addrlen = sizeof(clientAddr);
            int newClientFd = accept(listenSocket, (struct sockaddr *) &clientAddr, &addrlen);
            if (newClientFd < 0) {
                throw StartProxyException("accept");
            }
            if (-1 == checkSocket(newClientFd)) {
                throw StartProxyException("checkSocket");
            }
            if (NULL == addConnection(newClientFd)) {
                throw StartProxyException("addConnection");
            }
        }
    }
}

ClientConnection *Proxy::addConnection(int clientSocket) {
    ClientConnection *c = new ClientConnection(clientSocket);
    clientConnections.push_back(c);

    printf("new connection\n");

    return c;
}

void Proxy::fillMasksForSelect() {
    FD_ZERO(&readfs);
    FD_ZERO(&writefs);
    FD_SET(listenSocket, &readfs);

    fillMaskForClientsConnections();
    fillMaskForServersConnections();
}

void Proxy::fillMaskForServersConnections() {
    std::list<ServerConnection *> removedConnections;
    for (std::list<ServerConnection *>::iterator iterator = serverConnections.begin();
         iterator != serverConnections.end(); ++iterator) {
        ServerConnection *c = *iterator;
        switch (c->getState()) {
            case ServerConnectionState::ERROR:
                removedConnections.push_back(c);
                break;
            case ServerConnectionState::NEW_CONNECTION:
                FD_SET(c->getServerSocket(), &writefs);
                break;
            default:
                if (c->getByteInBuf() == 0) {
                    FD_SET(c->getServerSocket(), &readfs);
                }
                break;
        }

        for (std::list<ServerConnection *>::iterator list_iterator = removedConnections.begin();
             list_iterator != removedConnections.end(); ++list_iterator) {
            ServerConnection *removedConnection = *list_iterator;
            close(removedConnection->getServerSocket());
//            if(removedConnection->getServerSocket() != -1) {
//                close(removedConnection->getServerSocket());
//            }
            serverConnections.remove(removedConnection);
            delete (removedConnection);
            std::cout << "drop connection" << std::endl;

        }
    }
}

void Proxy::fillMaskForClientsConnections() {
    std::list<ClientConnection *> removedConnections;
    for (std::list<ClientConnection *>::iterator iterator = clientConnections.begin();
         iterator != clientConnections.end(); ++iterator) {
        ClientConnection *c = *iterator;
        switch (c->getState()) {
            case ClientConnectionState::ERROR:
                removedConnections.push_back(c);
                break;
            case ClientConnectionState::NEW_CONNECTION:
                FD_SET(c->getClientSocket(), &readfs);
                break;
            default:
                if (c->getByteInBuf() > 0) {
                    FD_SET(c->getClientSocket(), &writefs);
                }
                break;
        }

        for (std::list<ClientConnection *>::iterator list_iterator = removedConnections.begin();
             list_iterator != removedConnections.end(); ++list_iterator) {
            ClientConnection *removedConnection = *list_iterator;
            close(removedConnection->getClientSocket());
            clientConnections.remove(removedConnection);
            delete (removedConnection);
            std::cout << "drop connection" << std::endl;

        }
    }
}

void Proxy::checkServersReadfsAndWritefs() {
    for (std::list<ServerConnection *>::iterator iterator = serverConnections.begin();
         iterator != serverConnections.end(); ++iterator) {
        ServerConnection *c = *iterator;

        switch (c->getState()) {
            case ServerConnectionState::ERROR:
                break;
            case ServerConnectionState::NEW_CONNECTION:
                if (FD_ISSET(c->getServerSocket(), &writefs)) {

                    int res = (int) write(c->getServerSocket(), c->getClientConnection()->getBuf(),
                                          (size_t) c->getClientConnection()->getByteInBuf());
                    if (res == -1) {
                        c->getClientConnection()->setState(ClientConnectionState::ERROR);
                        c->setState(ServerConnectionState::ERROR);
                    } else {
                        c->getClientConnection()->setByteInBuf(0);
                        memset(c->getClientConnection()->getBuf(), 0, sizeof(c->getClientConnection()->getBuf()));
                    }
                    printf(">>ClientToServer: %s\n", c->getClientConnection()->getBuf());
                }
                break;
            case ServerConnectionState::EXPECTED_RESPONSE:
                if (FD_ISSET(c->getServerSocket(), &readfs)) {
                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(),
                                                         sizeof(c->getBuf()))))) {
                        c->getClientConnection()->setState(ClientConnectionState::ERROR);
                        c->setState(ServerConnectionState::ERROR);
                    } else {
                        handleAnswer(c);
                    }
                }
                break;
            case ServerConnectionState::CACHING_MODE:
                if (FD_ISSET(c->getServerSocket(), &readfs)) {
                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(),
                                                         sizeof(c->getBuf()))))) {
                        c->getClientConnection()->setState(ClientConnectionState::ERROR);
                        c->setState(ServerConnectionState::ERROR);
                        c->getCacheBucket()->setIsFull(true);
                    } else {
                        char *newItem = (char *) malloc(c->getByteInBuf() * sizeof(char));
                        memcpy(newItem, c->getBuf(), (size_t) c->getByteInBuf());
                        c->getCacheBucket()->addItem(newItem, c->getByteInBuf());

                        memcpy(c->getClientConnection()->getBuf(), c->getBuf(), (size_t) c->getByteInBuf());
                        c->getClientConnection()->setByteInBuf(c->getByteInBuf());
                    }
                }
                break;
            case ServerConnectionState::NOT_CACHING_MODE:
                if (FD_ISSET(c->getServerSocket(), &readfs)) {
                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(),
                                                         sizeof(c->getBuf()))))) {
                        c->getClientConnection()->setState(ClientConnectionState::ERROR);
                        c->setState(ServerConnectionState::ERROR);
                    } else {
                        memcpy(c->getClientConnection()->getBuf(), c->getBuf(), (size_t) c->getByteInBuf());
                        c->getClientConnection()->setByteInBuf(c->getByteInBuf());
                    }
                }
        }
    }
}

bool Proxy::isRightUrl(ClientConnection *c) const {
    char *tmp = strchr(c->getBuf(), ' ');
    if (tmp == NULL) {
        std::cout << "bad url" << std::endl;
        return false;
    }
    char *tmp2 = strchr(tmp + 1, ' ');
    if (tmp2 == NULL) {
        std::cout << "bad url" << std::endl;
        return false;
    }

    int endIndex = (int) (tmp2 - tmp);
    size_t length = (size_t) (endIndex - 1);

    char *url = (char *) calloc(length, sizeof(char)); //todo free memory
    strncpy(url, tmp + 1, length);
    c->setUrl(url);
    printf("url: %s\n", url);

    return true;
}

bool Proxy::isRightRequest(ClientConnection *c) {
    const char HTTP_405_ERROR[] = "HTTP/1.0 405";
    const char HTTP_505_ERROR[] = "HTTP/1.0 505";
    const char HTTP_400_ERROR[] = "HTTP/1.0 400";


    if (NULL == strstr(c->getBuf(), "GET") && NULL == strstr(c->getBuf(), "HEAD")) {
        std::cout << "not supported method" << std::endl;
//        write(c->getClientSocket(), HTTP_405_ERROR, strlen(HTTP_405_ERROR));
        write(c->getClientSocket(), HTTP_405_ERROR, sizeof(HTTP_405_ERROR));
        c->getState() = ERROR;
        return false;
    }
    if (NULL == strstr(c->getBuf(), "HTTP/1.0")) {
        std::cout << "not supported protocol" << std::endl;
//        write(c->getClientSocket(), HTTP_505_ERROR, strlen(HTTP_505_ERROR));
        write(c->getClientSocket(), HTTP_505_ERROR, sizeof(HTTP_505_ERROR));
        return false;
    }
    strcat(c->getBuf(), "\n\n"); //todo check if it needed?

    if (!isRightUrl(c)) {
        std::cout << "error in url parsing" << std::endl;
//        write(c->getClientSocket(), HTTP_400_ERROR, strlen(HTTP_400_ERROR));
        write(c->getClientSocket(), HTTP_400_ERROR, sizeof(HTTP_400_ERROR));
        return false;
    }

    return true;
}

void Proxy::handleRequest(ClientConnection *c) {
    std::cout << c->getBuf() << std::endl;
    if (isRightRequest(c)) {
        char *host = (char *) calloc(BUFSIZE, sizeof(char));
        sscanf(c->getUrl(), "%*[^/]%*[/]%[^/]", host);
        printf("host: %s\n", host);


        //check cache
        if (cache.find(c->getUrl()) == cache.end()) {
            c->setState(FROM_SERVER);
            int status;
            if (!connectWithServer(c, host)) {
                std::cout << "connect error" << std::endl;
                c->setState(ERROR);
            }
        } else {
            c->setState(ClientConnectionState::FROM_CACHE);
            c->setBucket(cache.find(c->getUrl())->second);
        }
        free(host);
    }

}

bool Proxy::connectWithServer(ClientConnection *c, char *remoteHost) {
    const char *HTTP_PORT = "80";
    struct addrinfo *remoteInfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(remoteHost, HTTP_PORT, &hint, &remoteInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return false;
    }
    struct sockaddr_in *serverAddr = (struct sockaddr_in *) remoteInfo->ai_addr;

    // преобразуем IP в строку и выводим его:
    void *addr = &(serverAddr->sin_addr);
    char buf[40];
    inet_ntop(remoteInfo->ai_family, addr, buf, sizeof buf);
    printf("ip: %s\n", buf);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == checkSocket(serverSocket)) {
        fprintf(stderr, "checkSocket");
        return false;
    }

    serverConnections.push_back(new ServerConnection(serverSocket, c));

    if (-1 == (connect(serverSocket, (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
        perror("connent to server");
        return false;
    }

    return true;
}

void Proxy::checkClientsReadfsAndWritefs() {
    for (std::list<ClientConnection *>::iterator iterator = clientConnections.begin();
         iterator != clientConnections.end(); ++iterator) {
        ClientConnection *c = *iterator;
        ClientConnectionState state = c->getState();
        switch (state) {
            case ClientConnectionState::NEW_CONNECTION:
                if (FD_ISSET(c->getClientSocket(), &readfs)) {
                    if (0 == (c->setByteInBuf((int) read(c->getClientSocket(), c->getBuf(),
                                                         sizeof(c->getBuf()))))) {
                        c->getState() = ClientConnectionState::ERROR;
                    } else {
                        handleRequest(c);
                    }
                }
                break;
            case ClientConnectionState::FROM_CACHE:
                if (FD_ISSET(c->getClientSocket(), &writefs)) {
                    if (c->getCurrentCachePosition() < c->getBucket()->size()) {
                        char *currentBuf = c->bucket->pagePieces[c->currentCashPosition];

                        size_t size = strlen(currentBuf);
                        int res = (int) write(c->clientSocket, currentBuf, (size_t) size); //todo read length buffer
                        if (res == -1) {
                            c->sizeClientToServer = -1;
                        } else {
                            c->currentCashPosition++;
                            if (c->currentCashPosition == c->bucket->pagePieces.size() && c->bucket->isFull) {
                                c->sizeClientToServer = -1;
                            }
                        }
                    }
                }
            case ClientConnectionState::FROM_SERVER:
                if (c->getByteInBuf() > 0 && FD_ISSET(c->getClientSocket(), &writefs)) {
                    int res = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());
                    if (res == -1) {
                        c->setState(ClientConnectionState::ERROR);
                    } else {
                        c->setByteInBuf(0);
                    }
                    printf("<<ServerTOClient: %s\n", c->getBuf());
                }
                break;
        }

    }


}

void Proxy::handleAnswer(ServerConnection *c) {
    if (strstr(c->getBuf(), "200") != NULL) {
        c->setState(CACHING_MODE);

        CacheBucket *bucket = new CacheBucket(); //todo free
        cache.insert(std::pair<char *, CacheBucket *>(c->getClientConnection()->getUrl(), bucket));
        c->setCacheBucket(bucket);
    } else {
        c->setState(NOT_CACHING_MODE);
    }

}
