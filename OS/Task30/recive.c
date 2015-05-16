#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include "Message.h"


int main() {
    int qid;
    Message msg;
    qid = msgget(getuid(), 0);

    while (msgrcv(qid, &msg, MAX_LENGTH, 0, IPC_NOWAIT | MSG_NOERROR) != -1) {
        printf("type=%ld message=[%s]\n", msg.type, msg.text);
    }


    if(msgctl(qid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Message queue wasn't removed.");
    }

    return EXIT_SUCCESS;
}
