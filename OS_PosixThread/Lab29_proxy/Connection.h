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

    bool fromCache ;

    bool requestHandled ;
    bool answerHandled ;

    bool cachingMode ;

    char bufClientToServer[BUFSIZE];
    char bufServerToClient[BUFSIZE]; //запись в мапе

    int clientSocket;
    int serverSocket;

    int sizeClientToServer; //destroy if -1
    int sizeServerToClient;

    CacheBucket* bucket;
    int currentCashPosition;
    char* url;

};


#endif //LAB29_PROXY_CONNECTION_H
