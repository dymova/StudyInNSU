#ifndef TASK30_MESSAGE_H
#define TASK30_MESSAGE_H

#define MAX_LENGTH 512

typedef struct message {
    long type;
    char text[MAX_LENGTH];
}Message;

#endif //TASK30_MESSAGE_H
