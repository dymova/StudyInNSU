#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include "Message.h"

#define CHILDREN_COUNT 5

int main(int argc, char **argv) {

    int qid;
    Message msg;
    pid_t children[CHILDREN_COUNT];
    pid_t pid;

    if((qid = msgget(getpid(), IPC_CREAT | 0660)) == -1) {

    perror("msgget");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < CHILDREN_COUNT; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { //child
            char buffer[BUF_SIZE];
            sprintf(buffer, "%s[%02d]", "recive", i);
            if(execl("recive", buffer, 0) == -1) {
                perror("execl(receive)");
                exit(EXIT_FAILURE);
            }
        }
        children[i] = pid;
    }
//    printf("%s", msg.text);

    printf("Plese, enter messageor or whitespace for exit.\n");
    while (fgets(msg.text, BUF_SIZE, stdin) != NULL) {
        if(strlen(msg.text) == 1) {
            printf("EOF");
            break;
        }
        for (int i = 0; i < CHILDREN_COUNT; i++) {
            msg.type = children[i];
            if(msgsnd(qid, &msg, strlen(msg.text) + 1, 0) == -1) {
                fprintf(stderr, "msg wasn't send to %d", i);
            }
        }
        printf("Plese, enter message or whitespace for exit.\n");
    }
    strcpy(msg.text, "EOF");
    for (int i = 0; i < CHILDREN_COUNT; i++) {
        msg.type = children[i];
        if(msgsnd(qid, &msg, strlen(msg.text) + 1, 0) == -1) {
            fprintf(stderr, "EOF msg wasn't send to %d", i);
        }
    }
    printf("send EOF\n");


    for (int i = 0; i < CHILDREN_COUNT;) {
        if(msgrcv(qid, &msg, BUF_SIZE, 0, MSG_NOERROR) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if (!strcmp("end recive", msg.text)) {
            i++;
        }
    }

    printf("The end!\n");
    if(msgctl(qid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Message queue wasn't removed.");
    }

    return EXIT_SUCCESS;
}