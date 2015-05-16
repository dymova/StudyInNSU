#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#define SEND_X 0
#define SEND_B 1
#define SEND_A 2
#define T 0.01
#define EPS 0.00001
#define N 5000

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
void fillingB(double* b, int size)
{
    for(int i = 0; i < size; ++i)
    {
        b[i] = size + 1;
    }
}
void fillingX(double* x, int size)
{
    for(int i = 0; i < size; ++i)
    {
        x[i] = 0;
    }
}
void computeC(double* A, double* x, double* b, double* C, int countLine, int size, int partB)
{
    double* Ax = (double*) calloc(countLine, sizeof(double));
    for(int i = 0 ; i < countLine; ++i)
    {
        for(int j = 0; j < size; ++j)
        {
            Ax[i] += A[i*size + j] * x[j];
        }
    }
    for(int i = 0; i < countLine; ++i)
    {
        C[i] = Ax[i] - b[partB + i];
    }
    free(Ax);
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
        MPI_Barrier(MPI_COMM_WORLD);
        printf("%.4lf(%d,x) ", v[i], rank);
    }
    printf("\n");
    printf("\n");
}

int main (int argc, char** argv)
{
  srand(time(NULL));
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
  fillingB(b,N);
  fillingX(x, N);
  A = malloc(lenghtOfPartA * sizeof(double));
  fillingA(A, displs[rank], lenghtOfPartA);

  long clocks_per_sec = sysconf(_SC_CLK_TCK);
  long clocks;
  struct tms start, end;
  times(&start);

  double normB;
  double generalNormC;
  int count = 0;
  double* recvBuf;
  do
  {

      if(count != 0)
      {
          for(int i = 0; i < N; ++i)
          {
              x[i] = recvBuf[i];
          }
      }
      count++;
      int countLine = recvCount[rank];
      double* C = (double*) malloc(countLine * sizeof(double));

      computeC(A, x, b, C, countLine, N, displs[rank]);

      double* tC = (double*) malloc(countLine * sizeof(double));
      for(int i = 0; i < countLine; ++i)
      {
          tC[i] = C[i] * T;
      }

      //substruction: x = x - tc
      for(int i = 0; i < countLine; ++i)
      {
          x[displs[rank] + i] -= tC[i];
      }

      recvBuf = (double*) malloc(N * sizeof(double));
      //printDoubleVector(C, N, rank);

      MPI_Allgatherv(&x[displs[rank]], recvCount[rank], MPI_DOUBLE, recvBuf, recvCount, displs, MPI_DOUBLE, MPI_COMM_WORLD);

      double localNormC = 0;
      for(int i = 0; i < countLine; ++i)
      {
          localNormC += C[i]*C[i];
      }
      MPI_Allreduce(&localNormC, &generalNormC, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
      generalNormC = sqrt(generalNormC);
      normB = computeNormB(b, N);
      free(C);
      free(tC);
  }
  while(conditionIsSatisfied(normB, generalNormC));
  times(&end);
  clocks = end.tms_utime - start.tms_utime;
  printf("1Time taken: %lf sec.(%d)\n", (double)clocks / clocks_per_sec, rank);
  printf("%d (%d)", count, rank);
  //printDoubleVector(recvBuf, N, rank);

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
       printf("\nWrong solution!\n");
  }
  else
  {
      printf("\nRight solution!\n");
  }

  free(x);
  free(b);
  free(A);

  MPI_Finalize();
  return 0;
}
