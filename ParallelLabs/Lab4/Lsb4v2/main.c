#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

const int N = 17;
#define SEND_V 0
#define EPS 0.00001

void fillingAB(double* AB, int processNumber, int rank)
{
    for (int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            if((rank + processNumber*i) == j)
                AB[i*(N+1) + j] = 2.0;
            else
                AB[i*(N+1) + j] = 1.0;
        }
        AB[i*(N+1) + N] = N + 1.0;
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

void printDoubleVector(double* v, int size, int rank)
{
    for(int i = 0; i < size; ++i)
    {
        printf("%.4lf(%d) ", v[i], rank);
    }
    printf("\n");
}
void printIntVector(int* v, int size, int rank)
{
    for(int i = 0; i < size; ++i)
    {
        printf("%d(%d,x) ", v[i], rank);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    MPI_Init (&argc, &argv);
    int rank;
    int processNumber;
    MPI_Comm_size (MPI_COMM_WORLD, &processNumber);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    //compute count lines in parts
    int bigPartNumber = N % processNumber;
    int rowsInPart = N / processNumber;
    int rowsInBigPart = 0;
    if(0 != bigPartNumber)
    {
        rowsInBigPart = rowsInPart + 1;
    }
    int* countLine = (int*) malloc(processNumber * sizeof(int));
    int* displs = (int*) malloc(processNumber * sizeof(int));
    displs[0] = 0;
    countLine[0] = rowsInBigPart;
    for (int i = 1; i < bigPartNumber; ++i)
    {
        countLine[i] = rowsInBigPart;
        displs[i] = displs[i - 1] + countLine[i - 1];
    }
    for (int i = bigPartNumber; i < processNumber; ++i)
    {
        countLine[i] = rowsInPart;
        displs[i] = displs[i - 1] + countLine[i - 1];
    }
    //filling start value
    int lineCount = countLine[rank];
    int lenghtOfPartA = lineCount * (N+1);
    double* AB;
    if((AB = (double*) malloc(lenghtOfPartA * sizeof(double))) == NULL)
    {
        fprintf(stderr, "not enought memory");
    }
    double* buf;
    if((buf = (double*) calloc((N+1), sizeof(double))) == NULL)
    {
        fprintf(stderr, "not enought memory");
    }
    fillingAB(AB, processNumber, rank);
    long clocks_per_sec = sysconf(_SC_CLK_TCK);
    long clocks;
    struct tms start, end;
    times(&start);

    for(int k = 0; k < N; k++)
    {
        for(int p = 0; p < processNumber; p++)
        {
            if(rank == p)
            {
//    Активная ветвь с номером приводит свою строку с номером k к диагональному виду.
//    Активная строка - k передается всем ветвям.
                double m = 1.0/AB[k*(N+1) + processNumber*k+p];
                for(int j = N; j >= processNumber*k+p; j--)
                    AB[k*(N+1) + j] *= m;
                for(int j = 0; j <= N; j++)
                    buf[j] = AB[k*(N+1) + j];
                MPI_Bcast(buf,N + 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                for(int i = k+1; i < N; i++)
                {
                    for(int j = N; j >= processNumber*k+p; j--)
                        AB[i*(N+1) + j] -= AB[i*(N+1) + processNumber*k+p]*AB[k*(N+1) + j];
                }
            }
//    Работа принимающих ветвей с номерами  < p
            else if(rank < p)
            {
                MPI_Bcast(buf,N+1,MPI_DOUBLE, p, MPI_COMM_WORLD);
                for(int i = k+1; i < N; i++)
                {
                    for(int j = N; j >= processNumber*k+p; j--)
                        AB[i*(N+1) + j] -= AB[i*(N+1) + processNumber*k+p]*buf[j];
                }
            }
            else if(rank > p)
            {
                MPI_Bcast(buf, N+1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                for(int i = k; i < N; i++)
                {
                    for(int j= N;j >= processNumber*k+p;j--)
                        AB[i*(N+1) + j] -= AB[i*(N+1) + processNumber*k+p]*buf[j];
                }
            }
        }
    }
//    обратный ход
    double R ;
    //обработка остатка
            for(int p = bigPartNumber-1; p >=0; p--)
            {
                int k = rowsInBigPart - 1;
                if(rank == p)
                {
                    R = AB[k*(N+1) + N];
                    MPI_Bcast(&R,1,MPI_DOUBLE,p,MPI_COMM_WORLD);
                    for(int i = k-1; i >= 0; i--)
                    {
                        AB[i*(N+1) + N]-= AB[k*(N+1) + N]*AB[i*(N+1) + processNumber*k + p];
                    }
                }
                else if(rank < p)
                {
                    MPI_Bcast(&R,1,MPI_DOUBLE,p,MPI_COMM_WORLD);
                    for(int i = lineCount-1; i >= 0; i--)
                    {
                        AB[i*(N+1) + N] -= R*AB[i*(N+1) + N  - 1];
                    }
                }
                else if(rank > p)
                {
                    MPI_Bcast(&R,1,MPI_DOUBLE,p,MPI_COMM_WORLD);
                    for(int i = k-1; i >= 0; i--)
                    {
                        AB[i*(N+1) + N] -= R*AB[i*(N+1) + N  - 1];
                    }
                }
            }
    //основная часть
    for(int k = rowsInPart-1; k >= 0; k--)
    {
        for(int p = processNumber-1; p >= 0; p--)
        {
            if(rank == p)
            {
                R = AB[k*(N+1) + N];
                MPI_Bcast(&R,1,MPI_DOUBLE,p,MPI_COMM_WORLD);
                for(int i = k-1; i >= 0; i--)
                {
                    AB[i*(N+1) + N] -= AB[k*(N+1) + N]*AB[i*(N+1) + processNumber*k+p];
                }
            }
            else if(rank < p)
            {
                MPI_Bcast(&R,1,MPI_DOUBLE,p, MPI_COMM_WORLD);
                for(int i = k; i >= 0; i--)
                    AB[i*(N+1) + N] -= R*AB[i*(N+1) + processNumber*k+p];
            }
            else if(rank > p)
            {
                MPI_Bcast(&R,1,MPI_DOUBLE,p,MPI_COMM_WORLD);
                    for(int i = k-1; i >= 0; i--)
                        AB[i*(N+1) + N] -= R*AB[i*(N+1) + processNumber*k+p];
            }
        }
    }

    //-------------------
//        if(rank == 0){
//            printf("-------%d---------------\n", lineCount);
//            for(int i = 0; i < lineCount; i++){
//                printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//            }
//        }
//        MPI_Barrier(MPI_COMM_WORLD);
//        if(rank == 1){
//            printf("-------%d---------------\n", lineCount);
//            for(int i = 0; i < lineCount; i++){
//                printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//            }
//        }
//        MPI_Barrier(MPI_COMM_WORLD);
//        if(rank == 2){
//            printf("-------%d---------------\n", lineCount);
//            for(int i = 0; i < lineCount; i++){
//                printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//            }
//        }
    //---------------
    times(&end);
    clocks = end.tms_utime - start.tms_utime;
    printf("Time taken: %lf sec.\n", (double)clocks / clocks_per_sec);
    //check
      int badSolution = 0;
      for(int j = 0; j < processNumber; j++)
      {
          for(int i = 0; i < lineCount; ++i)
          {
              if(1 - AB[i*(N+1) + N] > EPS)
              {
                  printf("%d \n", i);
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
      }
    return EXIT_SUCCESS;
}

