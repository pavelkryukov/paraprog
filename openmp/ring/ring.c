/**
 * ring.c
 *
 * Send/Recv messages with MPI in ring
 *
 * @author pikryukov
 * @version 1.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */

#include <stdlib.h>  /* strtoul */
#include <stdio.h>   /* fprintf, fflush */

#include <mpi.h>

#define ERRORPRINT(x) {if (rank == 0) fprintf(stderr, x); MPI_Finalize(); exit(1);}

/*
 *    0    1    2    3
 *    |>-->|   >|   >|
 *   >|    +   >|   >|
 *   >|    |>-->|   >|
 *   >|   >|    +   >|
 *   >|   >|    |>-->|
 *   >|   >|   >|    +
 *  ->|   >|   >|    |>-
 *    +   >|   >|   >|
 *    |>-->|   >|   >|
 *   >|    +   >|   >|
 *   >|    |>-->|   >|
*/
static void ring(int rank, int size, unsigned counter)
{
    int next = rank == size - 1 ? 0 : rank + 1;
    int prev = rank == 0 ? size - 1 : rank - 1;
    int buf;    
    int i;

    /* Create pulse */
    if (rank == 0)
    {
        buf = 0;
        MPI_Send(&buf, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    }

    /* Forwarding pulse around the threads */
    for (i = 0; i < counter; ++i)
    {
        MPI_Recv(&buf, 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        ++buf;
        fprintf(stdout, "[%2d] Received data in %d time\n", rank, buf);
        fflush(stdout);
        MPI_Send(&buf, 1, MPI_INT, next, 0, MPI_COMM_WORLD);        
    }

    /* Catching pulse */
    if (rank == 1)
    {
        MPI_Recv(&buf, 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        ++buf;
        fprintf(stdout, "[%2d] Received data in last time\n", rank);        
        fflush(stdout);
    }
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        /* Communicator constants */
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        /* Parsing arguments */
        if (argc != 2)
            ERRORPRINT("Syntax error.\n Only argument is send/receive counter!\n");

        ring(rank, size, strtoul(argv[1], NULL, 0));
    }
    MPI_Finalize();
    return 0;
}
