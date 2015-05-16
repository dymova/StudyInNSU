#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <omp.h>

#define SEND_X 0
#define SEND_B 1
#define SEND_A 2
#define T 0.01
#define EPS 0.00001
#define N 1000
#define THREAD_COUNT 2

void fillingA(double* A, int firstRow, int lenghtOfPartA)
{
    int rows = lenghtOfPartA / N;
    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            if(j == firstRow)
            {
                A[i*N + j] = 2;
            }
            else
            {
                A[i*N + j] = 1;
            }
        }
        firstRow++;
    }

}


void printDoubleVector(double* v, int size, int rank)
{
    for(int i = 0; i < size; ++i)
    {
        printf("%.4lf(%d,x) ", v[i], rank);
    }
    printf("\n");
    printf("\n");
}

int main (int argc, char** argv)
{
  MPI_Init (&argc, &argv);
  int rank;
  int processNumber;
  MPI_Comm_size (MPI_COMM_WORLD, &processNumber);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  double* x = (double*) malloc(N * sizeof(double));
  double* b = (double*) malloc(N * sizeof(double));
  double* A;
  int bigPartNumber = N % processNumber;
  int rowsInPart = N / processNumber;
  int rowsInBigPart = 0;
  if(0 != bigPartNumber)
  {
      rowsInBigPart = rowsInPart + 1;
  }
  int* recvCount = (int*) malloc(processNumber * sizeof(int));
  int* displs = (int*) malloc(processNumber * sizeof(int));
//set displs and recvCount
  displs[0] = 0;
  recvCount[0] = rowsInBigPart;
  for (int i = 1; i < bigPartNumber; ++i)
  {
      recvCount[i] = rowsInBigPart;
      displs[i] = displs[i - 1] + recvCount[i - 1];
  }
  for (int i = bigPartNumber; i < processNumber; ++i)
  {
      recvCount[i] = rowsInPart;
     displs[i] = displs[i - 1] + recvCount[i - 1];
  }
  int lenghtOfPartA = recvCount[rank] * N;
//  filling values
  double normB = 0;
  for(int i = 0; i < N; ++i)
  {
      b[i] = N + 1;
      x[i] = 0;
      normB += b[i]*b[i];
  }
  A = malloc(lenghtOfPartA * sizeof(double));
  fillingA(A, displs[rank], lenghtOfPartA);
  double ompTime1 = omp_get_wtime();

  normB = sqrt(normB);
  double generalNormC;
  int count = 0;
  double* recvBuf;
  double localNormC;
  omp_set_num_threads(THREAD_COUNT);

  int countLine = recvCount[rank];
  double* tmp = (double*) malloc(countLine * sizeof(double));
  do
  {
      for(int i = 0; i < N; ++i)
      {
          tmp[i] = 0;
      }

      if(count != 0)
      {
          for(int i = 0; i < N; ++i)
          {
              x[i] = recvBuf[i];
          }
      }
      count++;
      int partB = displs[rank];
      #pragma omp parallel for
      for(int i = 0 ; i < countLine; ++i)
      {
          for(int j = 0; j < N; ++j)
          {
              tmp[i] += A[i*N + j] * x[j];
          }
          tmp[i] = tmp[i] - b[partB + i];

      }
      localNormC = 0;

      #pragma omp parallel for reduction(+:localNormC)
      for(int i = 0; i < N; ++i)
      {
          localNormC += tmp[i] * tmp[i];

      }
      MPI_Allreduce(&localNormC, &generalNormC, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
      generalNormC = sqrt(generalNormC);

      #pragma omp parallel for
      for(int i = 0; i < countLine; ++i)
      {

          x[displs[rank] + i] -= tmp[i] * T;
      }

      recvBuf = (double*) malloc(N * sizeof(double));
      MPI_Allgatherv(&x[displs[rank]], recvCount[rank], MPI_DOUBLE, recvBuf, recvCount, displs, MPI_DOUBLE, MPI_COMM_WORLD);
  }
  while(generalNormC / normB > EPS );
  double ompTime2 = omp_get_wtime();
  printf("OMP_Time: %f sec.\n", ompTime2 - ompTime1);
  printf("%d\n", count);

//check
  int badSolution = 0;
  for(int i = 0; i < N; ++i)
  {
      if(1 - recvBuf[i] > EPS)
      {
          badSolution++;
      }
  }
  if(badSolution)
  {
       printf("Wrong solution!\n");
  }
  else
  {
      printf("Right solution!\n");
  }
  free(x);
  free(b);
  free(A);

  MPI_Finalize();
  return 0;
}
