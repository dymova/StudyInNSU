#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAX_LINE_COUNT 100
#define MAX_BUF_SIZE 70

typedef struct table
{
    int size;
    size_t length[MAX_LINE_COUNT];
    off_t offset[MAX_LINE_COUNT];

}Table;

void addInfoToTable(Table * table, off_t offset, size_t length)
{
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

void fillTable(char* file, Table* table, int size)
{
    off_t currentOffset = 0;
    off_t prevOffset = currentOffset;

        for (int i = 0; i < size; i++)
        {
            currentOffset++;
            if (file[i] == '\n')
            {
                addInfoToTable(table, prevOffset, currentOffset - prevOffset - 1);
                prevOffset = currentOffset;

            }
        }
    addInfoToTable(table, prevOffset, currentOffset - prevOffset);
}
void printLine(char* file, off_t offset, size_t size) {
    char buffer[size + 1];
    buffer[size] = 0;
    memcpy(buffer, file + offset, size);
    printf("%s\n", buffer);

}
void printTable(char* file, Table * table)
{
    printf("\n");
    for (int i = 0; i < table->size; i++)
    {
        printLine(file, table->offset[i], table->length[i]);
    }
}

int main(int argc, char * argv[])
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
    if((handle = open(pathToFile, O_RDONLY)) == -1)
    {
        perror(pathToFile);
        exit(EXIT_FAILURE);
    }

    struct stat bufState;
    if(fstat(handle, &bufState) == -1)
    {
        perror("fstate");
        exit(EXIT_FAILURE);
    }

    char *file;
    if((file = mmap(0, bufState.st_size, PROT_READ, MAP_PRIVATE, handle, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    Table table;
    table.size = 0;
    fillTable(file, &table, bufState.st_size);

    printTable(file, &table);

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
        printTable(file, &table);
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
            printLine(file, table.offset[line], table.length[line]);
        }

        printf("Please, enter the line number: ");
    }
    close(handle);
    munmap(file, bufState.st_size);
    return EXIT_SUCCESS;
}