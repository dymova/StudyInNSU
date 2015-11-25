#ifndef LAB25_FORWARDER_H
#define LAB25_FORWARDER_H


#include <sys/select.h>
#include <stdexcept>
#include <memory>
#include <list>
#include "Connection.h"

class Forwarder {
public:
    Forwarder(char *listenPort, char *remoteHost, char *remotePort);
    void start();


private:
    fd_set readfs;
    fd_set writefs;
    std::list<Connection*> connections;

    int maxfd;
    int listenSocket;
    int readyFdCount;
    struct sockaddr_in *serverAddr;


    void fillMasksForSelect(fd_set *readfs, fd_set *writefs,
                        int listenSocket);
    int checkSocket(int socketId, int *maxFd);

    void checkReadfsAndWritefs(fd_set *readfs, fd_set *writefs);
    Connection *addConnection(int clientSocket, struct sockaddr_in *serverAddr, int *maxFd);
};

class IllegalArgumentException : public std::runtime_error
{
public:
    explicit IllegalArgumentException(std::string message) :
            std::runtime_error("Illegal argument: " + message)
    {
    }
};

class ConstructorForwarderException : public std::runtime_error
{
public:
    explicit ConstructorForwarderException(std::string message) :
            std::runtime_error("Error in constructor: " + message)
    {
    }
};

class StartForwarderException : public std::runtime_error
{
public:
    explicit StartForwarderException(std::string message) :
            std::runtime_error("Error in start(): " + message)
    {
    }
};
#endif //LAB25_FORWARDER_H
