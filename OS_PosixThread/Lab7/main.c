#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const int ITERATION_NUMBER = 1000000;
int threads_number = 0;
double* sum;
int* ranks;
pthread_t* threads;

void* calculate_particular_sum(void* arg) {
    double tmp = 0;
    int rank = *((int*)arg);
    for (int i = rank; i < ITERATION_NUMBER ; i += threads_number) {
        tmp += 1.0/(i*4.0 + 1.0);
        tmp -= 1.0/(i*4.0 + 3.0);
    }
    sum[rank] = tmp;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if(argc >= 2){
        threads_number = atoi(argv[1]);
    }
    if(threads_number < 1){
        fprintf(stderr, "Incorrect arguments! "
                "Usage: number of threads.\n");
        exit(EXIT_FAILURE);
    }
    printf("threads number:%d\n", threads_number);

    ranks = (int*) malloc(threads_number * sizeof(int));
    sum = (double*) calloc(threads_number, sizeof(double));
    threads = (pthread_t*) malloc(threads_number* sizeof(pthread_t));

    if(ranks == NULL || sum == NULL|| threads == NULL) {
        fprintf(stderr, "Memory allocation failure.");
        exit(EXIT_FAILURE);
    }

    int code = 0;
    for (int i = 0; i < threads_number; ++i) {
        ranks[i] = i;
        code = pthread_create(&threads[i], NULL,
               calculate_particular_sum, &ranks[i]);
        if (code != 0) {
            char buf[256];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < threads_number; ++i) {
        pthread_join(threads[i], NULL);
        pi += sum[i];
    }
    pi *= 4.0;
    printf("Pi =  %.15lf \n", pi);

    free(ranks);
    free(sum);
    free(threads);
    return (EXIT_SUCCESS);
}