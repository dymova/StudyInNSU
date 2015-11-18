//
// Created by nastya on 15.11.15.
//

#include "Connection.h"

Connection::Connection(int clientSocket, int serverSocket) {
    this->clientSocket = clientSocket;
    this->serverSocket = serverSocket;
}
