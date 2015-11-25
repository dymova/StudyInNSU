//
// Created by nastya on 15.11.15.
//

#ifndef LAB29_PROXY_CONNECTION_H
#define LAB29_PROXY_CONNECTION_H

#include "CacheBucket.h"

#define BUFSIZE 1024

class Connection {

public:
    Connection(int clientSocket);

    bool fromCache = false;
    bool requestHandled = false;
    char bufClientToServer[BUFSIZE];
    char bufServerToClient[BUFSIZE]; //запись в мапе

    int clientSocket;
    int serverSocket = -1;

    int sizeClientToServer; //destroy if -1
    int sizeServerToClient;

    CacheBucket* bucket = nullptr;
    int currentCashPosition = 0;

};


#endif //LAB29_PROXY_CONNECTION_H
