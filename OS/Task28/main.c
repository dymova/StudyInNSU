#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>

#define N 100

int main() {
    srand(time(NULL));

    FILE *fp[2];
    if(p2open("sort -n", fp) == -1) {
        perror("p2open");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < N; i++) {
        fprintf(fp[0], "%d\n", rand() % N);
    }
    fprintf(fp[0], "\n");
    fclose(fp[0]);

    for(int i = 0; i < N / 10; i++) {
        int x;
        for(int j = 0; j < 10; j++) {
            fscanf(fp[1], "%d\n", &x);
            printf("%d ", x);
        }
        printf("\n ");
    }
    fclose(fp[1]);

    return EXIT_SUCCESS;
}