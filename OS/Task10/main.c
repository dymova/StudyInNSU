#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Not enough arguments.\n ");
        printf("The programm takes the name and arguments of the programm to be executed.\n ");
        exit(EXIT_SUCCESS);
    }

    pid_t pid;
    if((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0) {
        if(execv(argv[1], &argv[1]) == -1) {
            perror("execl");
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        if(wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status)){
            printf("child's exit status is: %d\n",WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("signal is: %d\n", WTERMSIG(status));
        }
    }
    return EXIT_SUCCESS;
}