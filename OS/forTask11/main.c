#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

extern char **environ;

void printEnvironmentVariables()
{
    char **p;
    printf("Environment variables:\n");
    for (p = environ; *p; p++)
    {
        printf("%s\n", *p);
    }
}

int main(int argc, char *argv[]) {
    printEnvironmentVariables();
    return EXIT_SUCCESS;
}