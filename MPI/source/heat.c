/**
 * heat.c
 *
 * Solving heat equation with MPI
 *
 * @author pikryukov
 * @version 4.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */

#include <stdlib.h> /* strtod, strtoul, malloc, free */
#include <string.h> /* memcpy */
#include <stdio.h>  /* fprintf, fopen, fclose */
#include <math.h>   /* exp */

#include <mpi.h>

#define FILENAME "result_kryukov.txt"

#define ERRORPRINT(x) {if (!rank) fprintf(stderr, x); MPI_Finalize(); exit(1);}

/*
 * The whole grid will be split at chunks to every thread.
 * Structure of the chunk is following:
 *
 * rank == 0:
 *
 * |@@@@@@@@@@@@|
 * |@   DATA   @|
 * |@__________@| @ is for edges, which are not counted
 *  ************   <- vector from next thread
 *
 * rank != 0 && rank != size - 1:
 *
 *  ************  <- vector from previous thread
 * |@          @|
 * |@   DATA   @|
 * |@          @|
 *  ************  <- vector from next thread
 *
 * rank == size - 1:
 *
 *  ************  <- vector from previous thread
 * |@          @|
 * |@   DATA   @|
 * |@@@@@@@@@@@@|
 *
 * As you can see, in every chunk we should make calculations only in inner
 * rectangle.
 */

/**
 * Scatters grid to threads with minimal recip
 * @param N grid size
 * @param sendcl size of grid sent to every thread
 * @param destcl offset for every thread
 * @param size mpi size
 */
void scatter(unsigned N, int* sendcl, int* destcl, int size)
{
    /* Grid is scattered to threads in this way:
    `*                    |-> amount == 6
     * 0:  0  1  2  3  4  5
     * 1:  6  7  8  9 10 11
     * 2: 12 13 14 15 16 17
     * 3: 18 19 20 21 22 23 24 <- resRank
     * 4: 25 26 27 28 29 30 31
     */
    size_t resRank = size - N % size;
    size_t amount = N / size;
    
    /* Give rectangled */
    size_t i = 0;
    for (; i < resRank; ++i)
        *sendcl++ = N * amount;

    /* Split recip to the bottom threads */
    ++amount;
    for (; i < size; ++i)
        *sendcl++ = N * amount;

    sendcl -= size;
    /* Fill offsets for every thread */
    *(destcl++) = 0;
    for (i = 1; i < size; ++i, ++destcl)
        *destcl = *(destcl - 1) + *(sendcl++);
}

/**
 * Prints square matrix from linear data array
 * @param f printed field
 * @param N size of row
 * @param rows amount of rows
 */
void print(const double* f, size_t N, size_t rows)
{
    FILE* fp = fopen(FILENAME, "w");
    size_t x, y;

    for (y = 0; y < rows; ++y)
    {
        for (x = 0; x < N; ++x)
            fprintf(fp, "%8.3f\t", *(f++));
        fprintf(fp, "\n");
    }
    fclose(fp);
}

/**
 * Fills field with initial values
 * @param f filling field
 * @param a alpha parameter of input function
 * @param b beta parameter of input function
 * @param N grid density
 */
void fill(double* f, double a, double b, size_t N)
{
    size_t x, y;
    /* Exponent multiplier */
    double multiplier = ((N - 1) * a);
    multiplier *= multiplier;
    multiplier = - 1. / multiplier;

    for (y = 0; y < N; ++y)
        for (x = 0; x < N; ++x)
            *(f++) = exp(multiplier * (x * x - 2 * b * x * y + y * y));
}

/**
 * Copy edges from one field to another
 * N.B. that edges are not changed after this copy.
 * @param old old field
 * @param new new field
 * @param N size of row
 * @param rows amount of rows
 */
void copyEdges(double* old, double* new, size_t N, size_t rows)
{
    /* Side edges */
    double* dest = new;
    const ptrdiff_t diff = old - new;
    size_t y;

    for (y = 0; y < rows; ++y)
    {
        /* Left edge */
        *dest = *(dest + diff);
        dest += N - 1;

        /* Right edge */
        *dest = *(dest + diff);
        ++dest;
    }

    /* Top edge */
    memcpy(new, new + diff, N * sizeof(double));
    
    /* Bottom edge */
    new += (rows - 1) * N,
    memcpy(new, new + diff, N * sizeof(double));
}

 /* Thread-look exchage scheme:
  *     ODD          EVEN
  *   <-|0|          |0|-<         
  *   >-|1|          |1|->
  *     |2|->      >-|2|
  *     |3|-<      <-|3|
  */

/* Vectors exchange scheme
 *
 * |            |
 * |____________| <->  ************
 *  ************  <-> |            |
 *                    |            |
 */
/**
 * Exchange 
 * @param f exchanging field
 * @param N size of row
 * @param rows amount of rows
 * @param rank mpi rank
 * @param size mpi size
 */
void exchange(double* f, size_t N, size_t rows, int rank, int isBottom) {
    if (rank % 2)
    {
        MPI_Send(f + N, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(f    , N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (!isBottom)
        {
            f += (rows - 2) * N;
            MPI_Send(f    , N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(f + N, N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else {
        if (!isBottom)
        {
            f += (rows - 2) * N;
            MPI_Recv(f + N, N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(f    , N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);            
            f -= (rows - 2) * N;
        }
        if (rank != 0)
        {
            MPI_Recv(f    , N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(f + N, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        }
    }
}
 
/**
 * Counts values in new layer from old layer
 * @param th2 tau div sqr h
 * @param old old layer
 * @param new new layer
 * @param N size of row
 * @param rows amount of rows
*/
void count(double th2, double* old, double* new, size_t N, size_t rows)
{
    /* ________________|_##############_|_##############_|________________ */
    size_t x, y;
    const size_t rowsT = rows - 2;
    const size_t columnsT = N - 2;    
    const ptrdiff_t diff = new - old; /* optimized memcpy style */
    old += N + 1;                     /* first point */

    for (y = 0; y < rowsT; ++y, old += 2)    /* skip edges */
        for (x = 0; x < columnsT; ++x, ++old)
            *(old + diff) = *old +
            (- 4 * *old + *(old - 1) + *(old + 1) + *(old - N) + *(old + N)) * th2;
}

/**
 * Heat equation solver
 * @param a alpha parameter of basis function
 * @param b beta parameter of basis function
 * @param T time to count to
 * @param N grid density
 * @param rank mpi rank
 * @param size mpi size
 */
void heat(double a, double b, double T, unsigned N, int rank, int size)
{
    /* Coordinate and time steps */
    const double h   = 1. / (N - 1);
    const double t   = h * h / 4.;
    const double th2 = t / (h * h);
    
    /* amount of steps to run */
    const unsigned needSteps = (ceil(T / t) + 1) / 2;
    
    /* Counting */
    unsigned steps = 0;

    /* io is used only on 0 thread to split data to all threads and  */
    /* gather it after calculations */
    double* io = NULL;
    
    /* We will work in two areas, 'old' and 'new' */
    double* old;
    double* new;
    
    /* Scatter sizes */
    int* const sendcl = (int*)malloc(sizeof(int) * size);
    int* const destcl = (int*)malloc(sizeof(int) * size);
    size_t rows;
    
    /* Fill data with initial values */
    if (!rank) {
        io = (double*)malloc(sizeof(double) * N * N);
        fill(io, a, b, N);
    }
    
    scatter(N, sendcl, destcl, size);
    
    rows = (sendcl[rank] / N) + 2 /* Our data and neighbour threads' data */
        - (!rank || rank == (size - 1))  /* No neighbour for edge threas */
        - (size == 1);                   /* No neighbours for one thread */
    
    old = (double*)malloc(sizeof(double) * N * rows);
    
    /* Scattering */
    MPI_Scatterv(io, sendcl, destcl, MPI_DOUBLE, old + (!!rank) * N, sendcl[rank], MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    /* Doubling field */
    new = (double*)malloc(sizeof(double) * N * rows);
    copyEdges(old, new, N, rows);

    while (steps++ < needSteps)
    {
        exchange(old, N, rows, rank, rank == size - 1);
        count(th2, old, new, N, rows);
        exchange(new, N, rows, rank, rank == size - 1);
        count(th2, new, old, N, rows);
    }
    
    /* Gathering */
    MPI_Gatherv(old + (!!rank) * N, sendcl[rank], MPI_DOUBLE, io, sendcl, destcl, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    /* Print gathered data */
    if (!rank)
        print(io, N, N);
    
    free(io);
    free(sendcl);
    free(destcl);
    free(new);
    free(old);
}

/**
 * Entry point
 * @param argc argument counter, should be equal 5
 * @param argv argument list (T, N, a, b)
 */
int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        double T, a, b;
        unsigned N;
        double time = -MPI_Wtime();

        /* Communicator constants */        
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        /* Arguments parsing */
        if (argc != 5)
            ERRORPRINT("Syntax error!\nArguments are following: a, b, T, N\n");
            
        T = strtod(argv[1], NULL);
        a = strtod(argv[3], NULL);
        b = strtod(argv[4], NULL);
        N = strtoul(argv[2], NULL, 0);

        if (size > N)
            ERRORPRINT("Communicator size is less than amount of grid rows!\n");
    
        heat(a, b, T, N, rank, size);

        if (!rank)
            fprintf(stdout, "Time is %.15f\n", time += MPI_Wtime());
    }
    MPI_Finalize();
    return 0;
}
