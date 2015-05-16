#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#define T 0.01
#define EPS 0.00001
#define N 80
#define THREAD_COUNT 1


void printDoubleVector(double* v)
{
    for(int i = 0; i < N; ++i)
    {
        printf("%.4lf ", v[i]);
    }
    printf("\n");
}

int main (int argc, char** argv)
{
  srand(time(NULL));
  long clocks_per_sec = sysconf(_SC_CLK_TCK);
  long clocks;
  struct tms start, end;
  double* x = (double*) malloc(N * sizeof(double));
  double* b = (double*) malloc(N * sizeof(double));
  double* A = (double*) malloc(N*N * sizeof(double));
  double normB = 0;

  times(&start);
//filling start value
  for(int i = 0; i < N; ++i)
  {
      for(int j = 0; j < N; ++j)
      {
          if(j == i)
          {
              A[i*N + j] = 2;
          }
          else
          {
              A[i*N + j] = 1;
          }
      }
      b[i] = N + 1;
      x[i] = 0;
      normB += b[i]*b[i];
  }
  normB = sqrt(normB);
  double normC;
  int count = 0;
 omp_set_num_threads(THREAD_COUNT);
 double* C = (double*) malloc(N * sizeof(double));
 double* Ax = (double*) calloc(N, sizeof(double));
 double* tC = (double*) malloc(N * sizeof(double));
  do
  {
      count++;
          for(int i = 0; i < N; ++i)
          {
              Ax[i] = 0;
              C[i] = 0;
              tC[i] = 0;
          }
          normC = 0;
#pragma omp parallel for ordered
          for(int i = 0 ; i < N; ++i)
          {
              for(int j = 0; j < N; ++j)
              {
                  Ax[i] += A[i*N + j] * x[j];
              }
              C[i] = Ax[i] - b[i];
              tC[i] = C[i] * T;
          }

            //substruction: x = x - tc
#pragma omp parallel for reduction(+:normC)
            for(int i = 0; i < N; ++i)
            {
                x[i] -= tC[i];
                normC += C[i]*C[i];
            }
            normC = sqrt(normC);
       }
  while(normC / normB > EPS);
  times(&end);
  clocks = end.tms_utime - start.tms_utime;
  printf("Time taken: %lf sec.\ncount=%d\n", (double)clocks / clocks_per_sec, count);
  int badSolution = 0;
    for(int i = 0; i < N; ++i)
    {
        if(1 - x[i] > EPS)
        {
            badSolution++;
        }
    }
    if(badSolution)
    {
         printf("\nWrong solution!\n");
    }
    else
    {
        printf("\nRight solution!\n");
    }

    free(Ax);
    free(tC);
    free(C);
    free(x);
    free(b);
    free(A);

  return 0;

}
