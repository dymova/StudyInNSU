
#ifndef LAB29_PROXY_PROXY_H
#define LAB29_PROXY_PROXY_H


#include <stdexcept>
#include <memory>
#include <list>
#include <map>
#include "Connection.h"
#include "CacheBucket.h"

class Proxy {

public:
    const char* HTTP_405_ERROR = "HTTP/1.0 405";
    const char* HTTP_505_ERROR = "HTTP/1.0 505";
    const char* HTTP_PORT = "80";
    Proxy(char *listenPortAsString);
    void start();

private:
    int maxFd;
    int readyFdCount;
    int listenSocket;
    fd_set readfs;
    fd_set writefs;
    std::list<std::shared_ptr<Connection>> connections;
    std::map<char*, CacheBucket*> cache;

    int checkSocket(int socketId);
    void fillMasksForSelect();
    void checkReadfsAndWritefs();
    Connection *addConnection(int clientSocket);


    void handleRequest(std::shared_ptr<Connection> &ptr);

    void getUrl(std::shared_ptr<Connection> &c, char[]) const;

    int connectWithServer(std::shared_ptr<Connection> &c, char *string);

};

class IllegalArgumentException : public std::runtime_error
{
public:
    explicit IllegalArgumentException(std::string message) :
            std::runtime_error("Illegal argument: " + message)
    {
    }
};

class ConstructorProxyException : public std::runtime_error
{
public:
    explicit ConstructorProxyException(std::string message) :
            std::runtime_error("Error in constructor: " + message)
    {
    }
};

class StartProxyException : public std::runtime_error
{
public:
    explicit StartProxyException(std::string message) :
            std::runtime_error("Error in start(): " + message)
    {
    }
};
#endif //LAB29_PROXY_PROXY_H
