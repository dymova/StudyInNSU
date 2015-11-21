//
// Created by nastya on 15.11.15.
//

#ifndef LAB29_PROXY_CONNECTION_H
#define LAB29_PROXY_CONNECTION_H

#define BUFSIZE 1024

class Connection {

public:
    Connection(int clientSocket);

    bool requestSent;
    char request[BUFSIZE];
    char bufServerToClient[BUFSIZE]; //запись в мапе

    int clientSocket;

    int sizeClientToServer; //destroy if -1
    int sizeServerToClient;

};


#endif //LAB29_PROXY_CONNECTION_H
