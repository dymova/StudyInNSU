#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define MAX_LINE_COUNT 100
#define MAX_BUF_SIZE 70

typedef struct table
{
    int size;
    ssize_t length[MAX_LINE_COUNT];
    off_t offset[MAX_LINE_COUNT];

}Table;

void addInfoToTable(Table * table, off_t offset, ssize_t length) {

    if (MAX_LINE_COUNT > table->size + 1) {

        table->offset[table->size] = offset;
        table->length[table->size] = length;
        table->size++;
    }
    else
    {
        perror("addInfoToTable()");
        exit(EXIT_FAILURE);
    }
}

void fillTable(int handle, Table* table)
{
    char stringBuffer[MAX_BUF_SIZE];
    ssize_t count;
    if((count = read(handle, stringBuffer, MAX_BUF_SIZE)) == -1)
    {
        exit(EXIT_FAILURE);
    }
    off_t currentOffset = 0;
    off_t prevOffset = currentOffset;

    while(0 != count)
    {
        for (int i = 0; i < count; i++)
        {
            currentOffset++;
            if (stringBuffer[i] == '\n')
            {
                addInfoToTable(table, prevOffset, currentOffset - prevOffset - 1);
                prevOffset = currentOffset;

            }
        }
        if(count = read(handle, stringBuffer, MAX_BUF_SIZE) == -1)
        {
            exit(EXIT_FAILURE);
        }
    }
     addInfoToTable(table, prevOffset, currentOffset - prevOffset);
}
void printLine(int handle, off_t offset, size_t size) {
    char buffer[size + 1];
    buffer[size] = 0;
    //buffer[0] = 'a';
    if(lseek(handle, offset, SEEK_SET) == -1)
    {
        exit(EXIT_FAILURE);
    }
    read(handle, buffer, size);
    buffer[size + 1] = '\0';
    printf("%s\n", buffer);

}
void printTable(int handle, Table * table)
{
    printf("\n");
    for (int i = 0; i < table->size; i++)
    {
        //printf("line:%d offset:%d lenght:%d\n", i, table->offset[i], table->length[i]);
        printLine(handle, table->offset[i], table->length[i]);
    }
}

int main(int argc, char * argv[])
{
    //int maxPath = pathconf("/", _PC_PATH_MAX);
    int handle;
    char pathToFile[MAX_BUF_SIZE];
    if (2 != argc)
    {
        printf("usearg: %s <pathToFile>\n", argv[0]);
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
        //scanf("%s", &pathToFile);
        //pathToFile = "/home/nastya/NSU/ОС/Task5/Task5/inputFile";
    }
    else
    {
        strcpy(pathToFile, argv[1]);
    }
    if((handle = open(pathToFile, O_RDONLY)) == -1)
    {
        perror(pathToFile);
        exit(EXIT_FAILURE);
    }

    Table table;
    table.size = 0;
    fillTable(handle, &table);

    //printTable(handle, &table);


    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    fd_set fdSet;
    int nfds = STDIN_FILENO + 1;
    FD_ZERO(&fdSet);
    FD_SET(STDIN_FILENO, &fdSet);
    printf("Please, enter the line number: ");
    fflush(stdout);
    int returnValue = select(nfds, &fdSet, NULL, NULL, &timeout);
    if(returnValue == -1)
    {
        perror("select()");
        exit(EXIT_FAILURE);
    }
    else  if(returnValue == 0)
    {
        printTable(handle, &table);
        exit(EXIT_SUCCESS);
    }

    for(;;)
    {
        int line;
        scanf("%d", &line);
        if(line < 0 || line - 1 >= table.size)
        {
            printf("Line %d doesn't exist.\n", line);
        }
        else
        {
            if(line == 0)
            {
                exit(EXIT_SUCCESS);
            }
            line--;
            printLine(handle, table.offset[line], table.length[line]);
        }

        printf("Please, enter the line number: ");
    }
    close(handle);
    return EXIT_SUCCESS;
}

