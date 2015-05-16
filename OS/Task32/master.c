#define _XOPEN_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>

#define BUF_SIZE 512
#define CHILDREN_COUNT 5

typedef struct message {
    long type;
    char text[BUF_SIZE];
}Message;

int main(int argc, char **argv) {

    int qid;
    Message msg;
    pid_t children[CHILDREN_COUNT];
    pid_t pid;
    int count;

    if((qid = msgget(getpid(), IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < CHILDREN_COUNT; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            char buffer[BUF_SIZE];
            sprintf(buffer, "%s[%02d]", "send", i);
            if(execl("send", buffer, 0) == -1) {
                perror("execl(send)");
                exit(EXIT_FAILURE);
            }
        }
        children[i] = pid;
    }

    count = CHILDREN_COUNT;
    while (count > 0) {
        if(msgrcv(qid, &msg, BUF_SIZE, 0, MSG_NOERROR) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        printf("pid: %ld message: %s\n", msg.type, msg.text);
        if (!strncmp(msg.text, "STOP", 3)) {
            count--;
        }
    }

    if(msgctl(qid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Message queue wasn't removed.");
    }

    return EXIT_SUCCESS;
}