#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

const int BUFFER_SIZE = 80;

typedef struct list {
    struct list *next;
    char *data;
} List;

pthread_rwlock_t rwlock;

void addFirstElement(char *str, List **head) {
    pthread_rwlock_wrlock(&rwlock);
    (*head)->data = str;
    (*head)->next = NULL;
    pthread_rwlock_unlock(&rwlock);

}

void printList(List *head) {
    pthread_rwlock_rdlock(&rwlock);
    if (NULL != head) {
        List *currentNode = head;
        while (NULL != currentNode) {
            printf("%s\n", currentNode->data);
            currentNode = currentNode->next;
        }
    }
    printf("-----------------------------------\n");
    pthread_rwlock_unlock(&rwlock);
}

void addStringToList(char *buffer, List **head) {
    pthread_rwlock_wrlock(&rwlock);
    List *newNode = (List *) malloc(sizeof(List));
    newNode->data = buffer;
    newNode->next = *head;
    *head = newNode;
    pthread_rwlock_unlock(&rwlock);
}

void releaseMemory(List **head) {
    List *currentNode = *head;
    if (NULL != currentNode) {
        while (NULL != currentNode) {
            free(currentNode->data);
            free(currentNode);
            currentNode = currentNode->next;
        }
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
        if (first->next == NULL) {
            continue;
        }
        pthread_rwlock_wrlock(&rwlock);
        for (List *i = first; i; i = i->next) {
            for (List *j = i->next; j; j = j->next) {
                if (0 < strcmp(i->data, j->data)) {
                    swap(i, j);
                }
            }
        }
        printf("Sorted list:\n");
        pthread_rwlock_unlock(&rwlock);
        printList(first);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int code;

    pthread_rwlockattr_t attr;
    code = pthread_rwlockattr_init(&attr);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: rwlock attr init: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code = pthread_rwlock_init(&rwlock, &attr);
    if (code != 0) {
        char buf[BUFFER_SIZE];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: init rwlock: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    pthread_t sort_thread;

    List *head = (List *) malloc(sizeof(List));
    head->next = NULL;

    pthread_create(&sort_thread, NULL, bubble_sort, (void *) (&head));


    printf("Please, enter message:\n");
    int isFirst = 1;
    for (; ;) {
        char *str = (char *) calloc(BUFFER_SIZE, sizeof(char));
        if (NULL == fgets(str, BUFFER_SIZE, stdin)) {
            break;
        }
        if ('\n' == str[strlen(str) - 1]) {
            str[strlen(str) - 1] = '\0';
        }

        if (0 == strlen(str)) {
            printList(head);
            continue;
        }
        if (isFirst) {
            addFirstElement(str, &head);
            isFirst = 0;
        }
        else {
            addStringToList(str, &head);
        }
        printList(head);
    }

    pthread_cancel(sort_thread);
    pthread_join(sort_thread, NULL);
    pthread_rwlockattr_destroy(&attr);
    pthread_rwlock_destroy(&rwlock);
    releaseMemory(&head);
    return EXIT_SUCCESS;
}
