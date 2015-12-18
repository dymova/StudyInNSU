#ifndef PROXY_PROXY_H
#define PROXY_PROXY_H


#include <sys/select.h>
#include <stdexcept>
#include <list>
#include <map>
#include <cstring>

#include "ClientConnection.h"
#include "ServerConnection.h"
#include "CacheBucket.h"

struct cmp_str {
    bool operator()(char const *a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

class Proxy {
public:
    Proxy(char *listenPort);

    void start();

private:
    int maxFd;
    int listenSocket;
    fd_set readfs;
    fd_set writefs;
    std::list<ClientConnection *> clientConnections;
    std::list<ServerConnection *> serverConnections;

    std::map<char *, CacheBucket *, cmp_str> cache;


public:
    const std::map<char *, CacheBucket *, cmp_str> &getCache() const {
        return cache;
    }

private:
    int checkSocket(int socketId);

    void fillMasksForSelect();

    void fillMaskForClientsConnections();

    void fillMaskForServersConnections();

    void checkClientsReadfsAndWritefs();

    void checkServersReadfsAndWritefs();

    ClientConnection *addConnection(int clientSocket);

    void handleRequest(ClientConnection *ptr);

    bool isRightRequest(ClientConnection *c);

    bool isRightUrl(ClientConnection *c) const;

    bool connectWithServer(ClientConnection *c, char *string);

    void handleAnswer(ServerConnection *c);

    void saveDataToCache(ServerConnection *c) const;

    void copyDataToClientBuf(ServerConnection *c) const;
};

class IllegalArgumentException : public std::runtime_error {
public:
    explicit IllegalArgumentException(std::string message) :
            std::runtime_error("Illegal argument: " + message) {
    }
};

class ConstructorProxyException : public std::runtime_error {
public:
    explicit ConstructorProxyException(std::string message) :
            std::runtime_error("Error in constructor: " + message) {
    }
};

class StartProxyException : public std::runtime_error {
public:
    explicit StartProxyException(std::string message) :
            std::runtime_error("Error in start(): " + message) {
    }
};

#endif //PROXY_PROXY_H
