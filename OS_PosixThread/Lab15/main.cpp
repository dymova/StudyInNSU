#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

const int COUNT_LINE = 10;
const char* SEM_NAME_1 = "/print_sem_1";
const char* SEM_NAME_2 = "/print_sem_2";

int main(int argc, char **argv) {
    sem_t* s1;
    sem_t* s2;
    pid_t pid;

    if((s1 = sem_open(SEM_NAME_1, O_CREAT, 0600, 0)) == NULL) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if((s2 = sem_open(SEM_NAME_2, O_CREAT, 0600, 1)) == NULL) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0) { //child

        for(int i = 0; i < COUNT_LINE; i++) {
            sem_wait(s1);
            printf("Child: %d\n", i);
            sem_post(s2);
        }
    } else {
        for(int i = 0; i < 10; i++) {
            sem_wait(s2);
            printf("Parent: %d\n", i);
            sem_post(s1);
        }

    }
    sem_unlink(SEM_NAME_1);
    sem_unlink(SEM_NAME_2);

    return EXIT_SUCCESS;
}