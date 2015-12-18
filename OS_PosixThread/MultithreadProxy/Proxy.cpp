#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <utility>

#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include	<time.h>
#include	<netinet/in.h>
#include	<fcntl.h>
#include	<string.h>
#include	<sys/uio.h>


#include "Proxy.h"
#include "ClientConnection.h"
#include "ThreadsBodies.h"

Proxy::Proxy(char *port) {
    cacheStorage = new CacheStorage();

    int listenPort = atoi(port);
    if (listenPort <= 0) {
        throw new IllegalArgumentException(port);
    }

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    int status;
    struct addrinfo *listenInfo;
    if ((status = getaddrinfo(NULL, port, &hint, &listenInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        throw new ConstructorProxyException(std::string("getaddrinfo for listening"));
    }

    listenSocket = socket(listenInfo->ai_family,
                          listenInfo->ai_socktype,
                          listenInfo->ai_protocol);
//    if (-1 == checkSocket(listenSocket)) {
//        throw new ConstructorProxyException("checkSocket");
//    }


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

void Proxy::start() {
    for (; ;) {
        struct sockaddr_in clientAddr;
        socklen_t addrlen = sizeof(clientAddr);
        int newClientFd = accept(listenSocket, (struct sockaddr *) &clientAddr, &addrlen);
        if (newClientFd < 0) {
            throw StartProxyException("accept");
        }
        std::cout << "new connection" << std::endl;
        if(!createNewClientThread(newClientFd)) {
            throw StartProxyException("creating thread");
        }

    }
}

bool Proxy::createNewClientThread(int clientSocket) const {
    ClientThreadArgs* args = (ClientThreadArgs*) malloc(sizeof(ClientThreadArgs));
    args->clientSocket = clientSocket;
    args->cacheStorage = cacheStorage;

    pthread_t thread;
    int code;
    code = pthread_create(&thread, NULL, clientConnectionThreadBody, args);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "creating thread:%s\n", buf);
        return false;
    }
    return true;
}
