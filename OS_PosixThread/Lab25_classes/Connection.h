//
// Created by nastya on 15.11.15.
//

#ifndef LAB25_CONNECTION_H
#define LAB25_CONNECTION_H
#define BUFSIZE 1024


class Connection {

public:
    Connection(int clientSocket, int serverSocket);

    char bufClientToServer[BUFSIZE];
    char bufServerToClient[BUFSIZE];
    int sizeClientToServer; //destroy if -1
    int sizeServerToClient;


private:
    int clientSocket;
    int serverSocket;



public:
    int getClientSocket() const {
        return clientSocket;
    }

    int getServerSocket() const {
        return serverSocket;
    }


    int getSizeClientToServer() const {
        return sizeClientToServer;
    }

    int getSizeServerToClient() const {
        return sizeServerToClient;
    }

};


#endif //LAB25_CONNECTION_H
