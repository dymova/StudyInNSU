//
// Created by nastya on 15.12.15.
//

#ifndef MULTITHREADPROXY_THREADSBODIES_H
#define MULTITHREADPROXY_THREADSBODIES_H

void *clientConnectionThreadBody(void *param);

void *exitClientConnectionThread(int clientSocket);

#endif //MULTITHREADPROXY_THREADSBODIES_H
