/**
 * integral.c
 *
 * Counting integral with MPI or OpenMP
 *
 * @author pikryukov
 * @version 1.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */
 
/* C generic */
#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* exit */
#include <math.h>   /* sin  */
#include <assert.h>

#ifdef USE_MPI /* If USE_MPI is defined, we use MPI, otherwise - OpenMP */
    #include <mpi.h>
    #define ERRORPRINT(...) \
        {if (!rank) fprintf(stderr, __VA_ARGS__); MPI_Finalize(); exit(1);}
    #define PRINT(...) {if (!rank) printf(__VA_ARGS__);}
#else
    #define ERRORPRINT(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
    #define PRINT(...) printf(__VA_ARGS__);
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define FUNC(x) (sin(1 / (x)))
#define ZERO(x) (1 / (M_PI * (x))) /* zero of FUNC. #1 is the greatest. */
 
#define N 10000 /* Number of zeroes to count integral on */
#define M 10000 /* Number of nodes between zeroes */

/**
 * Counts integral of FUNC from a to b usin
 * trapezoid method with M nodes
 * @param a left edge
 * @param b right edge
 * @return integral
 */
double monointegral(double a, double b)
{
    int i;
    double tau = (b - a) / M;
    double start = a;
    double sum = 0.;

    for (i = 0; i < M; ++i)
    {
        /* Counting with trapezoids rule */
        sum += tau * (FUNC(start + tau) + FUNC(start)) * 0.5;
        /* Go to next node */
        start += tau;
    }

    return sum;
}

#ifdef USE_MPI
/**
 * Splits N jobs to 'rank' thread
 * @param rank rank of current thread
 * @param size size of pool
 * @param start pointer to start number of job
 * @param finish pointer to finish number of job
 */
void splitfine(int rank, int size, int* start, int* finish)
{
    /* We try to split numbers like this: */
    /*  0  1  2  3  4   (5 == amount)
        5  6  7  8  9
       10 11 12 13 14
       15 16 17 18 19 20 <- resRank
       21 22 23 24 25 26
    */
    const unsigned resRank = size - N % size;
    const unsigned amount  = N / size;

    *start = rank * amount;    

    if (rank > resRank)
        *start += rank - resRank;

    *finish = *start + amount;

    if (rank >= resRank)
        ++(*finish);
}
#endif

/**
 * Counts integral of FUNC from NODE(N + 1) to NODE(1)
 * in parallel threads
 * @param rank thread rank (if MPI)
 * @param size pool size (if MPI)
 */
void multiintegral(int rank, int size)
{
    double res, sum = 0.;
    int start, finish, i;
#ifdef USE_MPI
    splitfine(rank, size, &start, &finish);
#else
    start = 0; finish = N;
    #pragma omp parallel for reduction (+: sum) private(i)
#endif /* USE_MPI */

    for (i = start; i < finish; ++i)
    {
        sum += monointegral(ZERO(i + 2), ZERO(i + 1));
    }

#ifdef USE_MPI
    MPI_Reduce(&sum, &res, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
#else
    res = sum;
#endif /* USE_MPI */
    PRINT("Integral of sin 1/x from %e to %e is %e\n", ZERO(N + 1), ZERO(1), res);
}

/**
 * Entry point of program
 * @param argc should be 1
 * @param argv no arguments needed
 * @return 0 on success, 1 on error
 */
int main(int argc, char** argv)
{
    int rank = 0, size = 0;
#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    double t = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif /* USE_MPI */

    if (argc != 1)
        ERRORPRINT("Syntax error.\n No arguments at all\n");

    multiintegral(rank, size);

#ifdef USE_MPI
    PRINT("Time is %f s\n", t += MPI_Wtime());
    MPI_Finalize();
#endif /* USE_MPI */
    return 0;
}
