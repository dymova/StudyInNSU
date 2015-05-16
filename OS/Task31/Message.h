#ifndef TASK31_MESSAGE_H
#define TASK31_MESSAGE_H

#define BUF_SIZE 512

typedef struct message {
    long type;
    char text[BUF_SIZE];
}Message;

#endif //TASK31_MESSAGE_H
