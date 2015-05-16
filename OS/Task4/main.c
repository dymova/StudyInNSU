#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGHT 100

typedef struct list
{
    struct list* next;
    char* data;
}List;

char* handleBuff(char* buffer)
{
    int lenght = (int) strlen(buffer);
    char* str = (char*) malloc(sizeof(char) * (lenght));
    strncpy(str, buffer, lenght);
    str[lenght - 1] = '\0';
    return str;
}

int addStringToList(char* buffer, List** head)
{
    if(NULL == *head)
    {
        *head = (List*) malloc (sizeof(List));
        (*head)->data = handleBuff(buffer);
        (*head)->next = NULL;
    }
    else
    {
        List* tail = *head;
        while(NULL != tail->next)
        {
            tail = tail->next;
        }
        List * newNode = (List*)malloc(sizeof(List));
        newNode->data = handleBuff(buffer);
        newNode->next = NULL;
        tail->next = newNode;

    }
}
void printList(List* head)
{
    if(NULL != head)
    {
        List* currentNode = head;
        while(NULL != currentNode)
        {
            printf("%s\n", currentNode->data);
            currentNode = currentNode->next;
        }
    }
}
void releaseMemory(List **head)
{
    List* currentNode = *head;
    if(NULL != currentNode)
    {
        while(NULL != currentNode)
        {
            free(currentNode->data);
            free(currentNode);
            currentNode = currentNode->next;
        }
    }
}

int main()
{
    char buffer[MAX_STRING_LENGHT];
    List* head  = NULL;

    while(NULL != fgets(buffer, MAX_STRING_LENGHT, stdin) && '.' != buffer[0])
    {
        addStringToList(buffer, &head);
    }
    printList(head);
    releaseMemory(&head);
    return EXIT_SUCCESS;
}
