#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include "Message.h"

int main(int argc, char ** argv) {
    Message msg;
    int qid;

    if((qid = msgget(getppid(), 0)) == -1) {
        perror("msgget");
        return EXIT_FAILURE;
    }

    for(;;) {
        if (msgrcv(qid, &msg, BUF_SIZE, 0, MSG_NOERROR) != -1) {
            if (!strcmp("EOF", msg.text)) {
                printf("%s: recived ending message\n", argv[0]);
                break;
            } else {
                printf("%s: %s", argv[0], msg.text);
            }
        }
        else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }

    strcpy(msg.text, "end recive");
    msg.type = getppid();
    if(msgsnd(qid, &msg, strlen(msg.text) + 1, 0) == -1) {
        perror("msgsnd [end recive]");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}