#define _XOPEN_SOURCE  500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 1024
#define TIME_OUT 1

typedef struct List {
    FILE * file;
    struct List * prev;
    struct List * next;
}List;

List* createList(FILE *file) {
    List * list = (List*) malloc(sizeof(List));
    list->file = file;
    list->prev = NULL;
    list->next = NULL;
    return list;
}

List* addToList(List *current, FILE* file) {
    List*newNode = createList(file);
    newNode->next = current->next;
    newNode->next->prev = newNode;
    newNode->prev = current;
    current->next = newNode;
    return newNode;
}

List* removeFromList(List *list) {
    if (list->next == list || list->prev == list) {
        return NULL;
    }
    list->prev->next = list->next;
    list->next->prev = list->prev;
    free(list);
    return list->prev;
}

sig_atomic_t sigAlarm = 0;
void sigalarm(int sig) {
    if (sig == SIGALRM) {
        sigAlarm = 1;
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Not enough args.\n"
                "The program takes the names of one or more files.\n");
        exit(EXIT_FAILURE);
    }
    FILE** files = (FILE**) malloc(argc * sizeof(FILE*));
    for(int i = 1; i < argc; i++) {
        if ((files[i - 1] = fopen(argv[i], "r")) == NULL) {
            perror(argv[i]);
            for(int j =0; j < i; j++){
                fclose(files[j]);
            }
            exit(EXIT_FAILURE);
        }
    }

    List* root;
    List* current;
    root = createList(files[0]);
    root->next = root;
    root->prev = root;
    current = root;
    for (int i = 1; i < argc - 1; i++) {
        current = addToList(current, files[i]);
    }
    current = root;
    sigset(SIGALRM, sigalarm);
    for(;;) {
        char buffer[BUF_SIZE];
        alarm(TIME_OUT);
        if (fgets(buffer, BUF_SIZE, current->file) != NULL) {
            puts(buffer);
        } else {
            if (sigAlarm && errno == EINTR) {
                sigAlarm = 0;
            } else {
                fclose(current->file);
                if((current = removeFromList(current)) == NULL) {
                    return EXIT_SUCCESS;
                }
            }
        }
        current = current->next;
    }
    return EXIT_SUCCESS;
}