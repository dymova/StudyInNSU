#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern char** environ;

int execvpe(char* file, char* argv[], char* newEnv[]){

    environ = newEnv;

    return execvp(file, argv);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Not enough arguments.\n ");
        printf("The programm takes the name and arguments of the programm to be executed.\n ");
        exit(EXIT_SUCCESS);
    }

    char* newEnv[]= {"environmentVar1=value1", "environmentVar2=value2", (char*) 0};
    if(execvpe(argv[1], &argv[1], newEnv) == -1) {
        perror("execvpe");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}