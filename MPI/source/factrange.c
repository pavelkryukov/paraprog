/**
 * factrange.c
 *
 * Counting sum of 1/n! range with MPI
 *
 * @author pikryukov
 * @version 4.2
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */

#include <stdlib.h>  /* strtoul */
#include <stdio.h>   /* fprintf */

#include <mpi.h>

#define ERRORPRINT(x) {if (rank == 0) fprintf(stderr, x); MPI_Finalize(); exit(1);}

/**
 * Counts sum of 1/n! range
 * @param N upper limit
 * @param rank mpi rank
 * @param size mpi size
 */
void range(unsigned N, int rank, int size)
{
    /* We try to split numbers like this: */
    /*  1  2  3  4  5   (5 == amount)
        6  7  8  9 10
       11 12 13 14 15
       16 17 18 19 20 21 <- resRank
       22 23 24 25 26 27
    */
    const unsigned resRank = size - N % size;
    const unsigned amount  = N / size;
   
    /* Buffer for saving results of every thread */
    register double rest = 0.0;
    register double fact = 1.0;

    register double i = rank * amount + 1;
    register double final_value;    
    
    double buf[2];
    if (rank > resRank) i += rank - resRank;

    final_value = i + amount;
    if (rank >= resRank) ++final_value;

    /* This loop counts ratios like (1/7, 1/7*8, 1/7*8*9) */
    /* These ratios are added to 'result', the last one is in 'fact' */
    while (i < final_value)
        rest += (fact /= i++); /* OMG. */

    if (rank) {
        /* Accumulating results from younger thread */
        MPI_Recv(buf, 2, MPI_DOUBLE, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        /* Multiplying our sum on fact from younger thread */
        rest *= *(buf + 1);

        /* Multiplying our fact on same number */
        fact *= *(buf + 1);

        /* Concratinating sums */
        rest += *buf;
    }

    if (rank != size - 1) {
        *buf = rest;
        *(buf + 1) = fact;
        /* Sending our results to elder thread */
        MPI_Send(buf, 2, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
    }
    else {
        /* If we're the eldest thread, print results */
        fprintf(stdout, "Result is %.15f\n", rest);
    }
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        double t = -MPI_Wtime(); /* thnx to Andrey Turetsky */

        /* Communicator constants */
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        /* Parsing arguments */
        if (argc != 2)
            ERRORPRINT("Syntax error.\n Only argument is limit number!\n");

        range(strtoul(argv[1], NULL, 0), rank, size);

        if (rank == size - 1)
            fprintf(stdout, "Time is %f s\n", t += MPI_Wtime());

    }
    MPI_Finalize();
    return 0;
}
