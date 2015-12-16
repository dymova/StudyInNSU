#ifndef MULTITHREADPROXY_PROXY_H
#define MULTITHREADPROXY_PROXY_H
#include <cstring>
#include <map>


#include <stdexcept>
#include "CacheBucket.h"
#include "ClientConnection.h"
#include "CacheStorage.h"


class Proxy {

public:
    Proxy(char *port);

    void start();

private:
    int listenSocket;
    CacheStorage *cacheStorage;


    bool createNewClientThread(int i) const;
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

typedef struct ClientThreadArgs {
    int clientSocket;
    CacheStorage* cacheStorage;

}ClientThreadArgs;

typedef struct ServerThreadArgs {
    int serverSocket;
    CacheStorage* cacheStorage;
    ClientConnection* clientConnection;
    char* url;
    int byteInBuf;
}ServerThreadArgs;

#endif //MULTITHREADPROXY_PROXY_H
