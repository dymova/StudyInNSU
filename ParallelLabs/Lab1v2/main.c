#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#define SEND_X 0
#define T 0.01
#define EPS 0.00001
#define N 5000

void fillingA(double* A, int firstRow, int lineCount)
{
    for(int i = 0; i < lineCount; ++i)
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
void fillingB(double* b, int size)
{
    for(int i = 0; i < size; ++i)
    {
        b[i] = N + 1;
    }
}
void fillingX(double* x, int size)
{
    for(int i = 0; i < size; ++i)
    {
        x[i] = 0;
    }
}
int conditionIsSatisfied(double normB, double normC)
{
    if(normC / normB > EPS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

double computeNormB(double* b, int size)
{
    double normB = 0;
    for(int i = 0; i < size; ++i)
    {
        normB += b[i]*b[i];
    }
    return sqrt(normB);
}

void printDoubleVector(double* v, int size, int rank)
{
    for(int i = 0; i < size; ++i)
    {
        printf("%.4lf(%d) ", v[i], rank);
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

  int lineCount = recvCount[rank];
  int lenghtOfPartA = lineCount * N;
  A = malloc(lenghtOfPartA * sizeof(double));
  fillingA(A, displs[rank], lineCount);
  double* x = (double*) malloc(lineCount * sizeof(double));
  double* b = (double*) malloc(lineCount * sizeof(double));
  fillingB(b, lineCount);
  fillingX(x, lineCount);
  long clocks_per_sec = sysconf(_SC_CLK_TCK);
  long clocks;
  struct tms start, end;
  times(&start);


  double generalNormB;
  double generalNormC;
  int count = 0;
  double localNormB = 0;
  for(int i = 0; i < lineCount; ++i)
  {
      localNormB += b[i]*b[i];
  }
  MPI_Allreduce(&localNormB, &generalNormB, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  generalNormB = sqrt(generalNormB);
  do
  {
      count++;
      double* C = (double*) calloc(lineCount, sizeof(double));

      //compute Ax
      double* xBuf = (double*) calloc(recvCount[0] , sizeof(double)); //заведомо больший
      for(int i = 0; i < lineCount; ++i)
      {
          xBuf[i] = x[i];
      }
      for(int k = 0; k < processNumber; ++k)
      {

          for(int i = 0; i < lineCount; ++i)
          {
              for(int j = 0; j < recvCount[(rank + k) % processNumber]; ++j)
              {
                  C[i] += A[i * N + displs[(rank + k) % processNumber] + j] * xBuf[j];
              }
          }
          MPI_Sendrecv_replace(xBuf, recvCount[0], MPI_DOUBLE, (rank - 1 + processNumber) % processNumber, SEND_X,
                            (rank + 1) % processNumber, SEND_X,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);


      }
//compute Ax-b
    for(int i = 0; i < lineCount; ++i)
    {
      C[i] -= b[i];
    }


      double* tC = (double*) malloc(lineCount * sizeof(double));
      for(int i = 0; i < lineCount; ++i)
      {
          tC[i] = C[i] * T;
      }
      //substruction: x = x - tc
      for(int i = 0; i < lineCount; ++i)
      {
          x[i] -= tC[i];
      }
      double localNormC = 0;
      for(int i = 0; i < lineCount; ++i)
      {
          localNormC += C[i]*C[i];
      }
      MPI_Allreduce(&localNormC, &generalNormC, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
      generalNormC = sqrt(generalNormC);
      free(tC);
      free(C);
      free(xBuf);
  }
  while(conditionIsSatisfied(generalNormB, generalNormC));
  times(&end);
  clocks = end.tms_utime - start.tms_utime;
  printf("Time taken: %lf sec.\n", (double)clocks / clocks_per_sec);
  printf("%d \n(%d)", count, rank);
  //check
    int badSolution = 0;
    for(int j = 0; j < processNumber; j++)
    {
        for(int i = 0; i < lineCount; ++i)
        {
            if(1 - x[i] > EPS)
            {
                 printf("%d \n", i);
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
    }

  MPI_Finalize();
  return 0;
}
