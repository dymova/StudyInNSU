#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5
#define SIZE 128

int main() {
    struct sembuf waitReading;
    waitReading.sem_num = 1;
    waitReading.sem_op = -1;
    waitReading.sem_flg = SEM_UNDO;

    struct sembuf produced;
    produced.sem_num = 0;
    produced.sem_op = 1;
    produced.sem_flg = SEM_UNDO;

    int semId;
    if (-1 == (semId = semget(getuid(), 2, IPC_CREAT | 0666))) {
        perror("semget");
        return EXIT_FAILURE;
    }

    int memId;
    if (-1 == (memId = shmget(getuid(), SIZE, IPC_CREAT | 0666))) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    char* shmAddr = shmat(memId, 0, 0);
    for (int i = 0; i < N; i++) {
        if (i > 0) {
            semop(semId, &waitReading, 1);
        }
        sprintf(shmAddr, "%d", i);
        semop(semId, &produced, 1);
    }
    if (-1 == shmdt(shmAddr)) {
        perror("shmdt");
        return EXIT_FAILURE;
    }
    if (semctl(semId, 0, IPC_RMID, 0)) {
        perror("shmdt");
        return EXIT_FAILURE;
    }
    if (shmctl(memId, IPC_RMID, 0)) {
        perror("shmdt");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}