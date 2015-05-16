#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>

#define SIZE 128

int main() {
    struct sembuf waitWriting;
    waitWriting.sem_num = 0;
    waitWriting.sem_op = -1;
    waitWriting.sem_flg = SEM_UNDO;

    struct sembuf consumed;
    consumed.sem_num = 1;
    consumed.sem_op = 1;
    consumed.sem_flg = SEM_UNDO;
    int semId;
    if (-1 == (semId = semget(getuid(), 2, 0))) {
        perror("semget");
        return EXIT_FAILURE;
    }
    int memId;
    if (-1 == (memId = shmget(getuid(), SIZE, 0))) {
        perror("shmget");
        return EXIT_FAILURE;
    }
    char* shmAddr = shmat(memId, 0, 0);
    for (;;) {
        if (-1 == semop(semId, &waitWriting, 1)) {
            break;
        }
        printf("%s\n", shmAddr);
        if(semop(semId, &consumed, 1) == -1) {
            perror("shmdt");
            return EXIT_FAILURE;
        }
    }
    if(shmdt(shmAddr)) {
        perror("shmdt");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}