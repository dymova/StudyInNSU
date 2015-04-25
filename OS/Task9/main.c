#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    pid_t pid;

    if((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0) {
        close(1);
        open("output.txt", O_WRONLY|O_CREAT);
        if(execl("/bin/cat", "cat", "/home/nastya/ООП/trunk/C++/HashTable/HashTable.cpp", (char*) 0) == -1) {
            perror("execl cat");
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        if(wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        printf("child's exit status is %d\n", WEXITSTATUS(status));
    }
    return EXIT_SUCCESS;
}