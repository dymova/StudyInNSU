#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF_SIZE 100
#define CHECK_ID_ERROR -1

int checkID(char* pathToFile)
{
    printf("Real uid:%d\n", getuid());
    printf("Effective uid:%d\n", geteuid());
    FILE* file;
    if((file = fopen(pathToFile, "r")) == NULL)
    {
        return CHECK_ID_ERROR;
    }
    printf("Successfully opened file!\n");
    fclose(file);
    return 0;
}

int main(int argc, char * argv[])
{
    char pathToFile[MAX_BUF_SIZE];
    if (2 != argc)
    {
        printf("usearg: %s <pathToFile>\n", argv[0]);
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
        //pathToFile = "/home/nastya/NSU/ОС/Task3/Task3/testFile";
    }
    else
    {
        strcpy(pathToFile, argv[1]);
    }

    if(CHECK_ID_ERROR == checkID(pathToFile))
    {
        perror(pathToFile);
        exit(EXIT_FAILURE);
    }

    if(-1 == setuid(getuid()))
    {
        perror("setuid");
        exit(EXIT_FAILURE);

    }

    if(CHECK_ID_ERROR == checkID(pathToFile))
    {
        perror(pathToFile);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

