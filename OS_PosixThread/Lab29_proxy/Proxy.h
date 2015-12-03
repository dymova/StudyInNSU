
#ifndef LAB29_PROXY_PROXY_H
#define LAB29_PROXY_PROXY_H


#include <stdexcept>
#include <sys/select.h>
#include <memory>
#include <cstring>
#include <vector>
#include <map>
#include "Connection.h"
#include "CacheBucket.h"

struct cmp_str
{
    bool operator()(char const *a, char const *b)
    {
        return std::strcmp(a, b) < 0;
    }
};

class Proxy {

public:
//    const char* HTTP_405_ERROR = "HTTP/1.0 405";
//    const char* HTTP_505_ERROR = "HTTP/1.0 505";
//    const char* HTTP_PORT = "80";
    Proxy(char *listenPortAsString);
    void start();

private:
    int maxFd;
    int readyFdCount;
    int listenSocket;
    fd_set readfs;
    fd_set writefs;
    std::list<Connection*> connections;
    std::map<char*, CacheBucket*, cmp_str> cache;

    int checkSocket(int socketId);
    void fillMasksForSelect();
    void checkReadfsAndWritefs();
    Connection *addConnection(int clientSocket);


    void handleRequest(Connection* ptr);
    bool checkRequest(Connection* c);

    void getUrl(Connection* c, char[]) const;

    int connectWithServer(Connection* c, char *string);

    void handleAnswer(Connection *c);
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
