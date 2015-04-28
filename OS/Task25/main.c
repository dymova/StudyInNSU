#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#define MAX_MESSAGE_SIZE 100

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    if((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { //child
        char message[] = "Hello, world!";
        close(fd[0]);
        if(write(fd[1], message, sizeof(message)) == -1) {
            perror("write");
            close(fd[1]);
            exit(EXIT_FAILURE);
        }
    } else {
        char message[MAX_MESSAGE_SIZE];
        close(fd[1]);
        if(read(fd[0], message, MAX_MESSAGE_SIZE) == -1) {
            perror("read");
            close(fd[0]);
            exit(EXIT_FAILURE);
        }
        int length = strlen(message);

        for (int i = 0; i < length; i++) {
            putchar(toupper(message[i]));
        }
    }

    close(fd[0]);
    close(fd[1]);
    return EXIT_SUCCESS;
}