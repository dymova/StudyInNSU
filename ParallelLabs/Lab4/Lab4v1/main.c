#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>


const int N = 2000;
#define SEND_V 0
#define EPS 0.00001

void fillingAB(double* AB, int firstRow, int lineCount)//todo
{
    for(int i = 0; i < lineCount; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            if(j == firstRow)
            {
                AB[i*(N + 1) + j] = 2;
            }
            else
            {
                AB[i*(N + 1) + j] = 1;
            }
        }
        AB[i*(N+1) + N] = N + 1;
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
    double* buf = (double*) malloc((N+1) * sizeof(double));
    fillingAB(AB, displs[rank], lineCount);

    long clocks_per_sec = sysconf(_SC_CLK_TCK);
    long clocks;
    struct tms start, end;
    times(&start);

    /* Цикл p - цикл по компьютерам. Все ветви,
    начиная с нулевой, последовательно приводят к
    диагональному виду свои строки.*/
    for(int p = 0; p < processNumber; p++)
    {
//        цикл по строкам
        for(int k = 0; k < countLine[p] ; k++)
        {
            //активный процесс приводит свои строки к диагональному виду
            if(rank == p)
            {
                double m = 1.0/AB[(N+1)*k + (displs[p]+k)];
                for(int j = N; j >= k + displs[p]; j--)
                {
                    AB[k*(N+1) + j] *= m;
                }

                for(int d = p+1; d < processNumber; d++)
                {
                    MPI_Send(&AB[k*(N+1)], N+1, MPI_DOUBLE, d, SEND_V, MPI_COMM_WORLD);
                }
                for(int i = k+1; i < countLine[p]; i++)
                {
                    for(int j = N; j >= k ; j--)
                    {
                      AB[i*(N+1) + j] -= AB[i*(N+1) + (displs[p]+k)]*AB[(k)*(N+1) + j];
                    }
                }
            }
            if(rank > p)
            {
                MPI_Recv(buf, N + 1, MPI_DOUBLE, p, SEND_V, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for(int i = 0; i < lineCount; i++)
                {
                    for(int j = N + 1; j >= (displs[p]+k); j--) //k
                    {
                        AB[i*(N + 1) + j] -= AB[i*(N+1) + (displs[p]+k)]*buf[j];
                    }
                }
            }

        }

    }

    /* Обратный ход */
    int M = N;
    for(int p = processNumber-1; p >= 0; p--)
    {
        for(int k = countLine[p]-1; k >= 0; k--)
        {
            if(rank == p)
            {
                for(int d = p-1; d >= 0; d--)
                {
                    MPI_Send(&AB[k*(N + 1) + M], 1, MPI_DOUBLE, d, SEND_V, MPI_COMM_WORLD);
                }
                for(int i = k-1; i >= 0; i--)
                {
                    AB[i*(N+1) + M]-= AB[k*(N+1) + M]*AB[i*(N+1) + (displs[p]+k)]; //k
                }
            }
            else
            {
                if(rank < p)
                {

                    double R;
                    MPI_Recv(&R, 1, MPI_DOUBLE, p, SEND_V, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for(int i = lineCount-1; i >= 0; i--)
                    {
                        AB[i*(N+1) + M] -= R*AB[i*(N+1) + N  - 1];
                    }
                }

            }
        }
    }
//                     if(rank == 0){
//                         printf("-------%d---------------\n", lineCount);
//                         for(int i = 0; i < lineCount; i++){
//                             printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//                         }
//                     }
//                     MPI_Barrier(MPI_COMM_WORLD);
//                     if(rank == 1){
//                         printf("-------%d---------------\n", lineCount);
//                         for(int i = 0; i < lineCount; i++){
//                             printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//                         }
//                     }
//                     MPI_Barrier(MPI_COMM_WORLD);
//                     if(rank == 2){
//                         printf("-------%d---------------\n", lineCount);
//                         for(int i = 0; i < lineCount; i++){
//                             printDoubleVector(&AB[(N+1)*i], N + 1, rank);
//                         }
//                     }

     times(&end);
     clocks = end.tms_utime - start.tms_utime;
     printf("Time taken: %lf sec.\n", (double)clocks / clocks_per_sec);

     int badSolution = 0;
     for(int j = 0; j < processNumber; j++)
     {
         for(int i = 0; i < lineCount; ++i)
         {
             if(fabs(1.0 - AB[i*(N+1) + N]) > EPS)
             {
                 badSolution++;
             }
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
    return EXIT_SUCCESS;
}

