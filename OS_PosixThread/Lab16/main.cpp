#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

const int BUFFER_SIZE = 80;

typedef struct list
{
    struct list* next;
    char* data;
}List;


typedef struct headElement{
    struct headElement* first;
}headElement;

pthread_mutex_t mutex;
List* head;

void printList(List* head)
{
    pthread_mutex_lock(&mutex);
    if(NULL != head)
    {
        List* currentNode = head;
        while(NULL != currentNode)
        {
            printf("%s\n", currentNode->data);
            currentNode = currentNode->next;
        }
    }
    pthread_mutex_unlock(&mutex);
    printf("-----------------------------------\n");
}

void addStringToList(char* buffer, List** head)
{
    pthread_mutex_lock(&mutex);
    if(NULL == *head)
    {
        *head = (List*) malloc (sizeof(List));
        (*head)->data = buffer;
        (*head)->next = NULL;
    }
    else
    {
        List * newNode = (List*)malloc(sizeof(List));
        newNode->data = buffer;
        newNode->next = *head;
        *head = newNode;
    }
    if(head != NULL) {
        printf(" not null head!");
    }
    pthread_mutex_unlock(&mutex);
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

void swap(List *a, List *b) {
    char* tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

void* bubble_sort(void* head) {

    for(;;) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        if(head == NULL) {
            printf("null head!");
        }
        for (List* i = (List*)head; i != NULL; i = i->next) {
            printf("qwer%s:::\n", i->data);
            for (List*j = i->next; j != NULL; j = j->next) {
                if (0 < strcmp(i->data, j->data)) {
                    swap(i, j);
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        printList((List*)head);
    }
    pthread_exit(NULL);
}



int main(int argc, char* argv[]) {
    int code;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: setting mutexattr type: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_mutex_init(&mutex, &attr);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: init mutex: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    pthread_t sort_thread;
    List* head = NULL;
//    List* head = (List*)malloc(sizeof(List));
//        head->first = NULL;


    pthread_create(&sort_thread, NULL, bubble_sort, (void*)(head));


    printf("Please, enter message:");
    for(;;)
    {
        char* str = (char*) calloc(BUFFER_SIZE, sizeof(char));
        if(NULL == fgets(str, BUFFER_SIZE, stdin)) {
            break;//todo
        }
        if ('\n' == str[strlen(str) - 1]) {
            str[strlen(str) - 1] = '\0';
        }

        if(0 == strlen(str)) {
            printf("emty line\n");
            printList(head);
            continue;
        }
        addStringToList(str, &head);
        printList(head);
    }

    pthread_cancel(sort_thread);
    pthread_join(sort_thread, NULL);
    pthread_mutex_destroy(&mutex);
    releaseMemory(&head);
    return EXIT_SUCCESS;
}