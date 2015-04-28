#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF_SIZE 70

int main(int argc, char **argv)
{
    int handle;
    char pathToFile[MAX_BUF_SIZE];
    if (2 != argc)
    {
        printf("usearg: %s <pathToFile>\n", argv[0]);
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
    }
    else
    {
        strcpy(pathToFile, argv[1]);
    }
    if((handle = open(pathToFile, O_WRONLY)) == -1)
    {
        perror(pathToFile);
        exit(EXIT_FAILURE);
    }

    struct flock lock;
    lock.l_type   = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start  = 0;
    lock.l_len    = 0;

    if(fcntl(handle, F_SETLK, &lock)== -1)
    {
        perror("fcntl");
        close(handle);
        return EXIT_FAILURE;
    }

    char prog[MAX_BUF_SIZE] = "nano ";
    strcat(prog, pathToFile);
    if (system(prog) == -1){
        perror("nano");
        close(handle);
        //todo handl fcntl
        return EXIT_FAILURE;
    }
    lock.l_type = F_UNLCK;
    fcntl( handle, F_SETLK, &lock);
    close(handle);
    return EXIT_SUCCESS;
}