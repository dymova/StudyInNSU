#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


typedef struct list {
    struct list *next;
    char *data;
    pthread_mutex_t* mutex;
} List;


void printList(List *head) {
    if (NULL != head) {
        List *currentNode = head;
        while (NULL != currentNode) {
            pthread_mutex_lock(currentNode->mutex);
            printf("%s\n", currentNode->data);
            pthread_mutex_unlock(currentNode->mutex);
            currentNode = currentNode->next;
        }
    }
    printf("-----------------------------------\n");
}

void addStringToList(char *buffer, List **head, pthread_mutex_t* mutex) {
    if (*head != NULL) {
        pthread_mutex_lock((*head)->mutex);
    }
    List *newNode = (List *) malloc(sizeof(List));
    newNode->data = buffer;
    newNode->next = *head;
    newNode->mutex = mutex;
    if(*head != NULL) {
        pthread_mutex_unlock((*head)->mutex);
    }
    *head = newNode;

}

void releaseMemory(List **head) {
    List *currentNode = *head;
    while (NULL != currentNode) {
        free(currentNode->data);
        free(currentNode);
        currentNode = currentNode->next;
    }
}

void swap(List *a, List *b) {
    char *tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}


void *bubble_sort(void *head) {
    for (; ;) {
        sleep(5);
        List *first = *((List **) head);
        for (List *i = first; i; i = i->next) {
            pthread_mutex_lock(i->mutex);
            for (List *j = i->next; j; j = j->next) {
                pthread_mutex_lock(j->mutex);
                if (0 < strcmp(i->data, j->data)) {
                    swap(i, j);
                }
                pthread_mutex_unlock(j->mutex);
            }
            pthread_mutex_unlock(i->mutex);

        }
        printf("Sorted list:\n");
        printList(first);
    }
}

int main(int argc, char *argv[]) {
    const int BUFFER_SIZE = 80;
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

    List *head = NULL;



    pthread_t sort_thread;
    pthread_create(&sort_thread, NULL, bubble_sort, (void *) (&head));

//    pthread_t sort_thread2;
//    pthread_create(&sort_thread2, NULL, bubble_sort, (void *) (&head));

    printf("Please, enter message or '.' for exit:\n");
    for (; ;) {
        char *str = (char *) calloc(BUFFER_SIZE, sizeof(char));
        if (NULL == fgets(str, BUFFER_SIZE, stdin) || '.' == str[0]) {
            break;
        }
        if ('\n' == str[strlen(str) - 1]) {
            str[strlen(str) - 1] = '\0';
        }

        if (0 == strlen(str)) {
            printList(head);
            continue;
        }
        pthread_mutex_t mutex;
        int code = pthread_mutex_init(&mutex, &attr);
        if (code != 0) {
            char buf[BUFFER_SIZE];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: init mutex: %s\n", buf);
            exit(EXIT_FAILURE);
        }
        addStringToList(str, &head, &mutex);

        printList(head);
    }

    pthread_cancel(sort_thread);
    pthread_join(sort_thread, NULL);
//    pthread_cancel(sort_thread2);
//    pthread_join(sort_thread2, NULL);

    releaseMemory(&head);
    return EXIT_SUCCESS;
}
