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
    fclose(fp[0]);

    for(int i = 0; i < N; i++) {
        int x;
        fscanf(fp[1], "%d\n", &x);
        printf("%d\n", x);
    }
    fclose(fp[1]);

    return 0;
}