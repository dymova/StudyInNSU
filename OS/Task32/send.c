#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 512

typedef struct message {
    long type;
    char text[BUF_SIZE];
}Message;

int main(int argc, char ** argv) {

    int qid;
    Message msg;
    qid = msgget(getppid(), 0660);
    msg.type = getpid();

    strcpy(msg.text, "RUNNING");
    if(msgsnd(qid, &msg, strlen(msg.text) + 1, 0) == -1) {
        fprintf(stderr, "Message queue wasn't removed.");
    }
    sleep(1);

    strcpy(msg.text, "STOP");
    if(msgsnd(qid, &msg, strlen(msg.text) + 1, 0) == -1) {
        perror("msgsnd STOP");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}