#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


typedef struct list {
    struct list *next;
    char *data;
    pthread_mutex_t *mutex;
} List;


void printList(List *head) {
    if (NULL != head) {
        List *currentNode = head;
        pthread_mutex_lock(currentNode->mutex);
        pthread_mutex_t *prev = currentNode->mutex;

        while (NULL != currentNode) {
            printf("%s\n", currentNode->data);

            currentNode = currentNode->next;
            if (currentNode == NULL) {
                break;
            }
            pthread_mutex_lock(currentNode->mutex);
            pthread_mutex_unlock(prev);
            prev = currentNode->mutex;
        }
        pthread_mutex_unlock(prev);
    }
    printf("-----------------------------------\n");
}

void addStringToList(char *buffer, List **head, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
    pthread_mutex_t *m = NULL;
    if (*head != NULL) {
        m = (*head)->mutex;
        pthread_mutex_lock(m);
    }

    List *newNode = (List *) malloc(sizeof(List));
    newNode->data = buffer;
    newNode->next = *head;
    newNode->mutex = mutex;
    *head = newNode;
    if (m != NULL) {
        pthread_mutex_unlock(m);
    }
    pthread_mutex_lock(mutex);

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
        int sorted = 0;
        List *first;
        while (!sorted) {
            first = *((List **) head);
            sorted = 1;
            List *j;
            List *i = first;
            List* prev = i;
            pthread_mutex_lock(i->mutex);
            j = i->next;
            while(j) {
                pthread_mutex_lock(j->mutex);
                if (0 < strcmp(i->data, j->data)) {
                    sorted = 0;
                    swap(i, j);
                }
                prev = i;
                i = i->next;
                j = i->next;
                pthread_mutex_unlock(prev->next->mutex);
                pthread_mutex_unlock(prev->mutex);
            }
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

    pthread_t *sorted = (pthread_t *) malloc(10 * sizeof(pthread_t));

    for (int i = 0; i < 10; ++i) {
        pthread_create(&sorted[i], NULL, bubble_sort, (void *) (&head));
    }


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

    for (int i = 0; i < 10; ++i) {
        pthread_cancel(sorted[i]);
        pthread_join(sorted[i], NULL);
    }

    releaseMemory(&head);
    return EXIT_SUCCESS;
}
