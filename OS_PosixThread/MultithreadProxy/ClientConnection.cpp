#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "ClientConnection.h"
#include "Proxy.h"
#include "ServerConnection.h"




ClientConnection::ClientConnection(int clientSocket) {
    this->clientSocket = clientSocket;
    serverSocket = -1;
    memset(buf, 0, sizeof(buf));
    byteInBuf = 0;
    url = NULL;
    state = FROM_SERVER;
    bucket = NULL;
    currentCachePosition = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "setting mutexattr type: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_mutex_init(&byteInBufMutex, &attr);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "init mutex: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_cond_init(&byteInBufCond, NULL);
    if (code != 0) {
        char buf[BUFSIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "init condition: %s\n", buf);
        exit(EXIT_FAILURE);
    }
}


bool ClientConnection::isRightUrl() {
    char *tmp = strchr(buf, ' ');
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

    url = (char *) calloc(length, sizeof(char));
    strncpy(url, tmp + 1, length);
    printf("url: %s\n", url);

    return true;
}


bool ClientConnection::isRightRequest() {
    const char HTTP_405_ERROR[] = "HTTP/1.0 405";
    const char HTTP_505_ERROR[] = "HTTP/1.0 505";
    const char HTTP_400_ERROR[] = "HTTP/1.0 400";

    if (NULL == strstr(buf, "GET") && NULL == strstr(buf, "HEAD")) {
        std::cout << "not supported method" << std::endl;
        write(clientSocket, HTTP_405_ERROR, sizeof(HTTP_405_ERROR));
//        send(clientSocket, HTTP_405_ERROR, sizeof(HTTP_405_ERROR), MSG_NOSIGNAL);
        return false;
    }
    if (NULL == strstr(buf, "HTTP/1.0")) {
        std::cout << "not supported protocol" << std::endl;
        write(clientSocket, HTTP_505_ERROR, sizeof(HTTP_505_ERROR));
//        send(clientSocket, HTTP_505_ERROR, sizeof(HTTP_505_ERROR), MSG_NOSIGNAL);
        return false;
    }

    if (!isRightUrl()) {
        std::cout << "error in url parsing" << std::endl;
        write(clientSocket, HTTP_400_ERROR, sizeof(HTTP_400_ERROR));
//        send(clientSocket, HTTP_400_ERROR, sizeof(HTTP_400_ERROR), MSG_NOSIGNAL);
        return false;
    }

    return true;
}

bool ClientConnection::handleRequest(std::map<char *, CacheBucket *, cmp_str> &pMap) {
    std::cout << buf << std::endl;
    if (!isRightRequest()) {
        return false;
    }
    char *host = (char *) calloc(BUFSIZE, sizeof(char));
    sscanf(url, "%*[^/]%*[/]%[^/]", host);
    printf("host: %s\n", host);


    //check cache
    std::map<char *, CacheBucket *, cmp_str> cache = pMap;
    if (cache.find(url) == cache.end()) {
        state = FROM_SERVER;
        if (!connectWithServer(host)) {
            std::cout << "connect error" << std::endl;
            return false;
        }
    } else {
        state = FROM_CACHE;
        bucket = cache.find(url)->second;
    }
    free(host);
    return true;
}

bool ClientConnection::readRequest() {
    if (0 == (byteInBuf = (int) read(clientSocket, buf, BUFSIZE))) { //todo -1
        return false;
    }
    return true;
}

bool ClientConnection::connectWithServer(char *remoteHost) {
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

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if (-1 == checkSocket(serverSocket)) {
//        fprintf(stderr, "checkSocket");
//        return false;
//    }



    if (-1 == (connect(serverSocket, (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
        perror("connent to server");
        return false;
    }

//    serverConnections.push_back(new ServerConnection(serverSocket, c));
    //создать новый поток и в неё сервер соединение

    return true;
}

int ClientConnection::getServerSocket() const {
    return serverSocket;
}

const ClientConnectionState &ClientConnection::getState() const {
    return state;
}

int ClientConnection::getClientSocket() const {
    return clientSocket;
}

char *ClientConnection::getBuf() {
    return buf;
}

int ClientConnection::getByteInBuf() const {
    return byteInBuf;
}

char *ClientConnection::getUrl() const {
    return url;
}

CacheBucket *ClientConnection::getBucket() const {
    return bucket;
}


void ClientConnection::setByteInBuf(int byteInBuf) {
    ClientConnection::byteInBuf = byteInBuf;
}

unsigned long ClientConnection::getCurrentCachePosition() const {
    return currentCachePosition;
}

void ClientConnection::incrementCachePosition() {
    currentCachePosition++;
}

pthread_mutex_t * ClientConnection::getByteInBufMutex()  {
    return &byteInBufMutex;
}

pthread_cond_t * ClientConnection::getByteInBufCond() {
    return &byteInBufCond;
}