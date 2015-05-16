#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include "Message.h"

int main(int argc, char **argv) {

    int qid;
    int i;
    Message msg;
    if (argc < 3) {
        printf("Not enough arguments.\n"
               " Program takes pair {messageType messageText} (see source, blin)\n");
        exit(EXIT_FAILURE);
    }

    if((qid = msgget(getuid(), IPC_CREAT | 0660 )) == -1){
        perror("msgget");
        return EXIT_FAILURE;
    }


    for (i = 1; i < argc - 1; i += 2) {
        msg.type = atol(argv[i]);
        strncpy(msg.text, argv[i + 1], MAX_LENGTH);
        if(strlen(argv[i + 1]) > MAX_LENGTH) {
            msg.text[strlen(msg.text)] = '\0';
        }
        msgsnd(qid, &msg, strlen(msg.text) + 1, 0);
        printf("type=%ld message=[%s]\n", msg.type, msg.text);
    }
    return EXIT_SUCCESS;
}
