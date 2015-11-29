//
// Created by nastya on 15.11.15.
//

#include <stddef.h>
#include "Connection.h"

Connection::Connection(int clientSocket) {
    this->clientSocket = clientSocket;

    this->fromCache = false;

    this->requestHandled = false;
    this->answerHandled = false;

    this->cachingMode = false;

    this->serverSocket = -1;

    this->sizeClientToServer = 0;
    this->sizeServerToClient = 0;

    this->bucket = NULL;
    this->currentCashPosition = 0;

}
