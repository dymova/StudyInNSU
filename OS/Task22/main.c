#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define MAX_BUF_SIZE 100

extern int errno;
#define TIMEOUT 1
typedef struct List List;
struct List {
    FILE * file;

    List * prev;
    List * next;
};

List * list_create(FILE * file) {
    List * list = malloc(sizeof(List));

    list->file = file;
    list->prev = NULL;
    list->next = NULL;

    return list;
}

List * list_insert(List * current, List * node) {

    node->next = current->next;
    node->next->prev = node;
    node->prev = current;
    current->next = node;
    return node;
}

List * list_remove(List * list) {
    if (list->next == list || list->prev == list) {
        return NULL;
    }

    list->next->prev = list->prev;
    list->prev->next = list->next;
    free(list);
    return list->prev;
}
int flag;
void sigalarm(int sig) {
    if (sig == SIGALRM) {
        flag = 1;
    }
}

int main(int argc, char *argv[]) {
    char pathToFile[MAX_BUF_SIZE];
    if (argc < 2) {
        printf("usearg: %s <pathToFile>\n", argv[0]);
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
    } else {
        strcpy(pathToFile, argv[1]);
    }

    for (int i = 1; i < argc; i++) {
        int handle;
        if((handle = open(pathToFile, O_RDONLY)) == -1) {
            perror(pathToFile);
            exit(EXIT_FAILURE);
        }

    }



    List * root = NULL;
    List * current;
    int handle;
    root = list_create(fopen(argv[1], "r"));
    root->next = root->prev = root;
    current = root;
    for (int i = 2; i < argc; i++) {
        current = list_insert(current, list_create(fopen(argv[i], "r")));
    }
    current = root;
    sigset(SIGALRM, sigalarm);
    while (1) {

        char buffer[1024];
        alarm(TIMEOUT);
        if (fgets(buffer, 1024, current->file) != NULL) {
            puts(buffer);
        } else {
            if (flag && errno == EINTR) {
                flag = 0;
                printf("timeout[%d]\n", fileno(current->file));
            } else {
                printf("closed[%d]\n", fileno(current->file));
                fclose(current->file);
                current = list_remove(current);
                if (current == NULL) {
                    printf("exit\n");
                    return 0;
                }
            }
        }

        current = current->next;
    }

    return 0;
}