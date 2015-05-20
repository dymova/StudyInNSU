#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define THREAD_COUNT 3
int main (int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "No enough arguments.Program takes size of matrix.");
        exit(EXIT_FAILURE);
    }
    MPI_Init(&argc,&argv);
    int N = atoi(argv[1]);
    int rank;
    int processNumber;
    MPI_Comm_size(MPI_COMM_WORLD, &processNumber);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    int countLine[processNumber];
    int displs[processNumber];
    int rowsInPart = N / processNumber;
    int bigPartNumber = N % processNumber;
    double start, end;
    start = MPI_Wtime();
    double ompTime1 = omp_get_wtime();
    omp_set_num_threads(THREAD_COUNT);

    #pragma omp parallel for
        for (int i = 0; i < processNumber; ++i)
        {
            displs[i] = (rowsInPart * i) + (i < bigPartNumber ? i : bigPartNumber);
            countLine[i] = rowsInPart + (i < bigPartNumber ? 1 : 0);
        }
        double *AB = (double*)malloc(countLine[rank] * (N + 1)* sizeof(double*));
        double *buf = (double*)malloc((N + 1) * sizeof(double));
        double R;
        #pragma omp parallel for
            for (int i = 0; i < countLine[rank]; ++i)
            {
                AB[i*(N+1) +N] = N + 1;
                for (int j = 0; j < N; ++j)
                {
                    if (j == displs[rank] + i)
                    {
                        AB[i*(N+1) +j] = 2.0;
                    }
                    else
                    {
                        AB[i*(N+1) +j] = 1.0;
                    }
                }
            }
        #pragma omp parallel
        {
            for (int p = 0; p < processNumber; ++p)
            {
                for (int k = 0; k < countLine[p]; ++k)
                {
                    if (rank == p)
                    {
                        #pragma omp for
                            for (int j = N; j > displs[p] + k; --j)
                            {
                                AB[k*(N+1) +j] /= AB[k*(N+1) +displs[p] + k];
                            }
                        #pragma omp single
                        {
                            AB[k*(N+1) +displs[p] + k] /= AB[k*(N+1) +displs[p] + k];
                        }

//                        #pragma omp for nowait
                            #pragma omp single
                            for (int s = p + 1; s < processNumber; ++s)
                            {
                                MPI_Send(&AB[k*(N+1)], N + 1, MPI_DOUBLE, s, 1, MPI_COMM_WORLD);
                            }
                        #pragma omp for
                            for (int i = k + 1; i < countLine[p]; ++i)
                            {
                                for (int j = N; j > displs[p] + k; --j)
                                {
                                    AB[i*(N+1) +j] -= AB[i*(N+1) +displs[p] + k] * AB[k*(N+1) +j];
                                }
                            }
                        #pragma omp for nowait
                            for (int i = k + 1; i < countLine[p]; ++i)
                            {
                                AB[i*(N+1) +displs[p] + k] -= AB[i*(N+1) +displs[p] + k] * AB[k*(N+1) +displs[p] + k];
                            }
                    }
                    else if (rank > p)
                    {
                        #pragma omp single
                        {
                            MPI_Recv(buf, N + 1, MPI_DOUBLE, p, 1, MPI_COMM_WORLD, &status);
                        }
                        #pragma omp for
                            for (int i = 0; i < countLine[rank]; ++i)
                            {
                                for (int j = N; j > displs[p] + k; --j)
                                {
                                    AB[i*(N+1) +j] -= AB[i*(N+1) +displs[p] + k] * buf[j];
                                }
                            }
                        #pragma omp for nowait
                            for (int i = 0; i < countLine[rank]; ++i)
                            {
                                AB[i*(N+1) +displs[p] + k] -= AB[i*(N+1) +displs[p] + k] * buf[displs[p] + k];
                            }
                    }
                }
            }
            #pragma omp barrier
            for (int p = processNumber - 1; p >= 0; --p)
            {
                for (int k = countLine[p] - 1; k >= 0; --k)
                {
                    if (rank == p)
                    {
//                        #pragma omp for
                        #pragma omp single
                            for (int s = p - 1; s >= 0; --s)
                            {
                                MPI_Send(&AB[k*(N+1) +N], 1, MPI_DOUBLE, s, 2, MPI_COMM_WORLD);
                            }
                        #pragma omp for
                            for (int i = k - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= AB[k*(N+1) +N] * AB[i*(N+1) +displs[p] + k];
                            }
                    }
                    else if (rank < p)
                    {
                        #pragma omp single
                        {
                            MPI_Recv(&R, 1, MPI_DOUBLE, p, 2, MPI_COMM_WORLD, &status);
                        }
                        #pragma omp for
                            for (int i = countLine[rank] - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= R * AB[i*(N+1) +displs[p] + k];
                            }
                    }
                }
            }
        }
        double ompTime2 = omp_get_wtime();
        printf("OMP_Time: %f sec.\n", ompTime2 - ompTime1);
        double L1 = 0;
        double L2 = 0;
        double L_inf = 0;
        for (int i = 0; i < N; ++i)
        {
            if (rank == i % processNumber)
            {
                L1 += fabs(AB[(i/ processNumber)*(N+1) + N] - 1);
                L2 += fabs((AB[(i/ processNumber)*(N+1) + N] - 1)*(AB[(i/ processNumber)*(N+1) +N] - 1));
                double dif = fabs(AB[(i/ processNumber)*(N+1) + N] - 1);
                if(dif > L_inf)
                {
                    L_inf = dif;
                }
            }
        }
        printf("L1: %e \t L2:%e \t L_inf: %e\n", L1, L2, L_inf);
        free(AB);
        free(buf);
    MPI_Finalize();
    return 0;
}
