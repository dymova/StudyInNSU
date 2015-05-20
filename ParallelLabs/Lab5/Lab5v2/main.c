#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>


int main (int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "No enough arguments.Program takes size of matrix.");
    }
    MPI_Init(&argc,&argv);
    int N = atoi(argv[1]);
    int rank;
    int processNumber;
    MPI_Comm_size(MPI_COMM_WORLD, &processNumber);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    int countLine[processNumber];
    int rowsInPart = N / processNumber;
    int bigPartNumber = N % processNumber;
    double ompTime1 = omp_get_wtime();
#pragma omp parallel for
        for (int i = 0; i < processNumber; ++i)
        {
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
                    if (j == rank + (i * processNumber))
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
            for (int k = 0; k < countLine[processNumber - 1]; ++k)
            {
                for (int p = 0; p < processNumber; ++p)
                {
                    if (rank == p)
                    {
                        #pragma omp for
                        for (int j = N; j > p + (k * processNumber); --j)
                            AB[k*(N+1) +j] /= AB[k*(N+1) +p + (k * processNumber)];

                        #pragma omp single
                        {
                            AB[k*(N+1) +p + (k * processNumber)] /= AB[k*(N+1) +p + (k * processNumber)];
                        }
                        #pragma omp single nowait
                        {
                                MPI_Bcast(&AB[k*(N+1)], N + 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                        for (int i = k + 1; i < countLine[rank]; ++i)
                        {
                            for (int j = N; j > p + (k * processNumber); --j)
                            {
                                AB[i*(N+1) +j] -= AB[i*(N+1) +p + (k * processNumber)] * AB[k*(N+1) +j];
                            }
                        }
                        #pragma omp for nowait
                            for (int i = k + 1; i < countLine[rank]; ++i)
                            {
                                AB[i*(N+1) +p + (k * processNumber)] -= AB[i*(N+1) +p + (k * processNumber)] * AB[k*(N+1) +p + (k * processNumber)];
                            }
                    }
                    else if (rank < p)
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(buf, N + 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = k + 1; i < countLine[rank]; ++i)
                            {
                                for (int j = N; j > p + (k * processNumber); --j)
                                {
                                    AB[i*(N+1) +j] -= AB[i*(N+1) +p + (k * processNumber)] * buf[j];
                                }
                            }
                        #pragma omp for nowait
                            for (int i = k + 1; i < countLine[rank]; ++i)
                            {
                                AB[i*(N+1) +p + (k * processNumber)] -= AB[i*(N+1) +p + (k * processNumber)] * buf[p + (k * processNumber)];
                            }
                    }
                    else
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(buf, N + 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = k; i < countLine[rank]; ++i)
                            {
                                for (int j = N; j > p + (k * processNumber); --j)
                                {
                                    AB[i*(N+1) +j] -= AB[i*(N+1) +p + (k * processNumber)] * buf[j];
                                }
                            }
                        #pragma omp for nowait
                            for (int i = k; i < countLine[rank]; ++i)
                            {
                                AB[i*(N+1) +p + (k * processNumber)] -= AB[i*(N+1) +p + (k * processNumber)] * buf[p + (k * processNumber)];
                            }
                    }
                }
            }

            if (bigPartNumber != 0)
            {
                for (int p = 0; p < bigPartNumber; ++p)
                {
                    if (rank == p)
                    {
                        #pragma omp for
                            for (int j = N; j > p + (rowsInPart * processNumber); --j)
                            {
                                AB[rowsInPart*(N+1) +j] /= AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)];
                            }
                        #pragma omp single
                        {
                            AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)] /= AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)];
                        }
                        #pragma omp for nowait
                            for (int s = p + 1; s < processNumber; ++s)
                            {
                                MPI_Send(&AB[rowsInPart*(N+1)], N + 1, MPI_DOUBLE, s, 1, MPI_COMM_WORLD);
                            }
                    }
                    else if (rank > p && rank < bigPartNumber)
                    {
                        #pragma omp single
                        {
                            MPI_Recv(buf, N + 1, MPI_DOUBLE, p, 1, MPI_COMM_WORLD, &status);
                        }
                        #pragma omp for
                            for (int j = N; j > p + (rowsInPart * processNumber); --j)
                            {
                                AB[rowsInPart*(N+1) +j] -= AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)] * buf[j];
                            }
                        #pragma omp single nowait
                        {
                            AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)] -= AB[rowsInPart*(N+1) +p + (rowsInPart * processNumber)] * buf[p + (rowsInPart * processNumber)];
                        }
                    }
                }
                for (int p = bigPartNumber - 1; p >= 0; --p)
                {
                    if (rank == p)
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&AB[rowsInPart*(N+1) +N], 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for nowait
                            for (int i = rowsInPart - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= AB[rowsInPart*(N+1) +N] * AB[i*(N+1) +p + (rowsInPart * processNumber)];
                            }
                    }
                    else if (rank < p)
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&R, 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for //nowait
                            for (int i = rowsInPart; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= R * AB[i*(N+1) +p + (rowsInPart * processNumber)];
                            }
                    }
                    else
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&R, 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = rowsInPart - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= R * AB[i*(N+1) +p + (rowsInPart * processNumber)];
                            }
                    }
                }
            }

            for (int k = countLine[processNumber - 1] - 1; k >= 0; --k)
            {
                for (int p = processNumber - 1; p >= 0; --p)
                {
                    if (rank == p)
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&AB[k*(N+1) +N], 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = k - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= AB[k*(N+1) +N] * AB[i*(N+1) +p + (k * processNumber)];
                            }
                    }
                    else if (rank < p)
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&R, 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = k; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= R * AB[i*(N+1) +p + (k * processNumber)];
                            }
                    }
                    else
                    {
                        #pragma omp single
                        {
                            MPI_Bcast(&R, 1, MPI_DOUBLE, p, MPI_COMM_WORLD);
                        }
                        #pragma omp for
                            for (int i = k - 1; i >= 0; --i)
                            {
                                AB[i*(N+1) +N] -= R * AB[i*(N+1) +p + (k * processNumber)];
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
                L1 += fabs(AB[(i/ processNumber)*(N+1) +N] - 1);
                L2 += fabs((AB[(i/ processNumber)*(N+1) +N] - 1)*(AB[(i/ processNumber)*(N+1) +N] - 1));
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

