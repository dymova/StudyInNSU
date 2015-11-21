
#ifndef LAB29_PROXY_PROXY_H
#define LAB29_PROXY_PROXY_H


#include <stdexcept>
#include <memory>
#include <list>
#include "Connection.h"

class Proxy {

public:
    Proxy(char *listenPortAsString);

    void start();

private:
    int maxFd;
    int readyFdCount;
    int listenSocket;
    fd_set readfs;
    fd_set writefs;
    std::list<std::shared_ptr<Connection>> connections;


    int checkSocket(int socketId);
    void fillMasksForSelect();
    void checkReadfsAndWritefs();
    Connection *addConnection(int clientSocket);


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
