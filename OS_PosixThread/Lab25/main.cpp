#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
const char *USAGE = "Usage: Server <listenPort> <remoteHost> <remotePort>";

typedef struct connection {
    int clientSocket;
    int serverSocket;
    struct connection *prev, *next;
    char bufClientToServer[BUFSIZE];
    char bufServerToClient[BUFSIZE];
    int sizeClientToServer; //destroy if -1
    int sizeServerToClient;
//    int destoyFlag;
} Connection;

Connection *head = NULL;
Connection *tail = NULL;

void checkSocket(int socketId, int *maxFd) {
    if (socketId >= FD_SETSIZE) {
        fprintf(stderr, "Socket number out of range\n");
        exit(EXIT_FAILURE);
    }
    if (socketId > *maxFd) {
        *maxFd = socketId;
    }
}

void dropConnection(Connection *c) {
    if (c == head && c == tail) {
        head = NULL;
        tail = NULL;
    } else if (c == head) {
        head = c->next;
        head->prev = NULL;
    } else if (c == tail) {
        tail = c->prev;
        tail->next = NULL;
    } else {
        c->next->prev = c->prev;
        c->prev->next = c->next;
    }
    close(c->serverSocket);
    close(c->clientSocket);
    free(c);
}

void fillMasksForSelect(fd_set *readfs, fd_set *writefs,
                        int listenSocket, Connection *pHead) {
    FD_ZERO(readfs);
    FD_ZERO(writefs);
    FD_SET(listenSocket, readfs);

    Connection *c = pHead;
    while (c) {
        if ((c->sizeClientToServer < 0 && c->sizeServerToClient <= 0) ||
            (c->sizeServerToClient < 0 && c->sizeClientToServer <= 0)) {
            dropConnection(c);
        } else {
            if (c->sizeClientToServer == 0) {
                FD_SET(c->clientSocket, readfs);
            }
            if (c->sizeServerToClient == 0) {
                FD_SET(c->serverSocket, readfs);
            }
            if (c->sizeClientToServer > 0) {
                FD_SET(c->serverSocket, writefs);
            }
            if (c->sizeServerToClient > 0) {
                FD_SET(c->clientSocket, writefs);
            }
            c = c->next;
        }
    }
}

void checkReadfsAndWritefs(fd_set *readfs, fd_set *writefs, Connection *pHead) {
    Connection *c = pHead;
    while (c) {
        if (c->sizeClientToServer == 0 && FD_ISSET(c->clientSocket, readfs)) {
            if (0 == (c->sizeClientToServer = (int) read(c->clientSocket, c->bufClientToServer,
                                                         sizeof(c->bufClientToServer)))) {
                c->sizeClientToServer = -1;
            }
        }//todo don't check buf
        if (c->sizeServerToClient == 0 && FD_ISSET(c->serverSocket, readfs)) {
            if (0 == (c->sizeServerToClient = (int) read(c->serverSocket, c->bufServerToClient,
                                                         sizeof(c->bufServerToClient)))) {
                c->sizeServerToClient = -1;
            }
        }
        if (c->sizeClientToServer > 0 && FD_ISSET(c->serverSocket, writefs)) {
            int res = (int) write(c->serverSocket, c->bufClientToServer, c->sizeClientToServer);
            if (res == -1) {
                c->sizeServerToClient = -1;
            } else {
                c->sizeClientToServer = 0;
            }
        }
        if (c->sizeServerToClient > 0 && FD_ISSET(c->clientSocket, writefs)) {
            int res = (int) write(c->clientSocket, c->bufServerToClient, c->sizeServerToClient);
            if (res == -1) {
                c->sizeClientToServer = -1;
            } else {
                c->sizeServerToClient = 0;
            }
        }
        c = c->next;
    }
}


Connection *addConnection(int clientSocket, struct sockaddr_in *serverAddr, int *maxFd) {
    Connection *c = (Connection *) malloc(sizeof(Connection));
    if (c == NULL) {
        perror("malloc");
        return NULL;
    }
    c->clientSocket = clientSocket;
    c->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    checkSocket(c->serverSocket, maxFd);

    if (-1 == (connect(c->serverSocket, (struct sockaddr *) serverAddr, sizeof(*serverAddr)))) {
        perror("connent to server");
        close(c->serverSocket); //todo check
        free(c);
        return NULL;
    }

    c->prev = NULL;
    c->next = head;
    if (head == NULL) {
        tail = c;
    } else {
        head->prev = c;
    }
    head = c;
    c->sizeServerToClient = 0;
    c->sizeClientToServer = 0;
    memset(c->bufClientToServer, 0, sizeof(c->bufClientToServer));
    memset(c->bufServerToClient, 0, sizeof(c->bufServerToClient));

    return c;
}

int main(int argc, char **argv) {
    int maxfd = 0;

    if (argc != 4) {
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }

    int listenPort = atoi(argv[1]);
    int remotePort = atoi(argv[3]);

    if (listenPort <= 0 || remotePort <= 0) {
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }

    struct addrinfo *remoteInfo;
    struct addrinfo hint;//todo fill hint
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(argv[2], argv[3], &hint, &remoteInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }
    struct sockaddr_in *serverAddr = (struct sockaddr_in *) remoteInfo->ai_addr;


    // преобразуем IP в строку и выводим его:
    void *addr = &(serverAddr->sin_addr);
    char buf[40];
    inet_ntop(remoteInfo->ai_family, addr, buf, sizeof buf);
    printf("\n %s", buf);


    hint.ai_flags = AI_PASSIVE;
    struct addrinfo *listenInfo;
    if ((status = getaddrinfo(NULL, argv[1], &hint, &listenInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    int listenSocket = socket(listenInfo->ai_family,
                              listenInfo->ai_socktype,
                              listenInfo->ai_protocol);
    checkSocket(listenSocket, &maxfd);

    int val = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    if (bind(listenSocket, listenInfo->ai_addr, listenInfo->ai_addrlen)) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    if ((listen(listenSocket, SOMAXCONN)) == -1) {
        perror("listen");
        return EXIT_FAILURE;
    }

    fd_set readfs;
    fd_set writefs;
    int readyFdCount;
    for (; ;) {
        fillMasksForSelect(&readfs, &writefs, listenSocket, head);
        if ((readyFdCount = select(maxfd + 1, &readfs, &writefs, NULL, NULL)) == -1) {
            perror(argv[0]);
            return EXIT_FAILURE;
        }

        checkReadfsAndWritefs(&readfs, &writefs, head);

        struct sockaddr_in cliaddr;
        if (FD_ISSET(listenSocket, &readfs)) {
            socklen_t addrlen = sizeof(cliaddr);
            int newClientFd = accept(listenSocket, (struct sockaddr *) &cliaddr, &addrlen);
            if (newClientFd < 0) {
                perror(argv[0]);
                return EXIT_FAILURE;
            }
            checkSocket(newClientFd, &maxfd);
            if (NULL == addConnection(newClientFd, serverAddr, &maxfd)) {
                perror(argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    freeaddrinfo(remoteInfo);
    return EXIT_SUCCESS;
}
