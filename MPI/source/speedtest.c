/**
 * speedtest.c
 *
 * Counting ratio of time of MPI_Send execution
 * to time of floating division
 *
 * @author pikryukov
 * @version 2.0
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */

#include <stdio.h>  /* printf */
#include <stdlib.h> /* strtoul */

#include <mpi.h>

/**
 * Send/Recv time counter (ping-thread part)
 * @param N number of operations
 * @param pong pong-thread number
 * @return time
 */
double sendrecv(unsigned N, unsigned pong) {
    unsigned i;
    double a  = 1.0;        
    double ts = -MPI_Wtime();
    for (i = 0; i < N; ++i) {
        MPI_Send(&a, 1, MPI_DOUBLE, pong, 0, MPI_COMM_WORLD);
        MPI_Recv(&a, 1, MPI_DOUBLE, pong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }        
    return ts += MPI_Wtime();
}

/**
 * Send/Recv time counter (pong-thread part)
 * @param N number of operations
 * @param ping ping-thread number
 */
void sendrecv2(unsigned N, unsigned ping) {
    unsigned i;
    double a = 1.0;
    for (i = 0; i < N; ++i) {
        MPI_Recv(&a, 1, MPI_DOUBLE, ping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&a, 1, MPI_DOUBLE, ping, 0, MPI_COMM_WORLD);
    }
}

/**
 * Floating division time counter
 * @param N number of operations
 * @return time
 */
double floating(unsigned N) {
    volatile double b = 1.0; /* I use 'volatile' for 1st time ever. */
    double td = -MPI_Wtime();
    unsigned i;
    for (i = 0; i < N; ++i)
        b /= 2.5;

    return td += MPI_Wtime();
}

#define PING 0
#define PONG (size - 1)

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        unsigned N;
        int rank, size;

        /* Parsing arguments */
        if (argc != 2)
        {
            printf("Syntax error.\n Command line is: %s <N>\n", argv[0]);
            MPI_Finalize();
            return 1;
        }
        
        N = strtoul(argv[1], NULL, 0);

        /* Communicator constants */
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        if (rank == PING) {
            double ts = sendrecv(N, PONG);
            double td = floating(N <<= 1);
        
            printf("%d Send/Recv time: %f\n%d FlDivs time: %f\nratio is %f\n", 
            N, ts, N, td, ts / td);
        }
        if (rank == PONG) {
            sendrecv2(N, PING);
        }
    }
    MPI_Finalize();
    return 0;
}
