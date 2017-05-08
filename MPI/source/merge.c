/**
 * merge.c
 *
 * Merge sort with MPI
 *
 * @author pikryukov
 * @version 3.2
 * 
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */
 
#include <stdio.h>  /* fprintf, fscanf, fopen, fclose, rewind, fgets */
#include <stdlib.h> /* malloc, free */
#include <string.h> /* strcat */

#include <mpi.h>

#define ERRORPRINT(x) {if (!rank) fprintf(stderr, x); MPI_Finalize(); exit(1);}
 
/**
 * Scatters grid to threads with minimal recip
 * @param N grid size
 * @param sendcl size of grid sent to every thread
 * @param destcl offset for every thread
 * @param gathercl size of grid that will be collected from every thread
 * @param size mpi size
 */
void scatter(unsigned N, int* sendcl, int* destcl, int* gathercl, int size)
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
    
    size_t i = 0;
    for (; i < resRank; ++i) /* Give rectangled */
        *sendcl++ = amount;
    ++amount;
    for (; i < size; ++i)    /* Split recip to the bottom threads */
        *sendcl++ = amount;
    sendcl -= size;

    /* Fill offsets for every thread */
    *(destcl++) = 0;
    for (i = 1; i < size; ++i, ++destcl)
        *destcl = *(destcl - 1) + *(sendcl++);
    sendcl -= (size - 1);

    /* Fill returning sizes */
    for (i = size - 1; i != -1; --i)
    {
        size_t level = 1;
        gathercl[i] = sendcl[i];
        while (level < size && !(i % (level << 1)))
        {
            if (i + level < size)
                gathercl[i] += gathercl[i + level];
            level <<= 1;
        }
    }
}

/**
 * Counts amount of lines in file
 * @param file file pointer
 * @return amount of lines in file
 */
unsigned countLines(FILE* file)
{
    unsigned lines = 0;
    char buf[256];
    while(fgets(buf, 256, file) != NULL)
        lines++;
    rewind(file);
    return lines;
}

/**
 * Prints array to file
 * @param data array pointer
 * @param N size of array
 * @param filename file name
 */
void print(const int* data, size_t N, const char* filename)
{
    char file[256] = "sorted_";
    size_t i;
    FILE* fp = NULL;
    
    strcat(file, filename);
    fp = fopen(file, "w");

    for (i = 0; i < N; ++i)
        fprintf(fp, "%d\n", *(data++));

    fclose(fp);
}

/**
 * Merge of two parts of one array
 * @param fst array pointer
 * @param fs size of 1st part
 * @param ss size of 2nd part
 * @param temp temporary memory
 * @return size fs + ss
 */
size_t merge(int* fst, size_t fs, size_t ss, int* temp)
{
    size_t ts = fs + ss;
    const size_t ts_ret = ts;
    const int* snd = fst + fs;
    while (fs > 0 && ss > 0)
        *(temp + --ts) = *(fst + fs - 1) > *(snd + ss - 1)
            ? *(fst + --fs)
            : *(snd + --ss);
    while (ss > 0)
        *(temp + --ts) = *(snd + --ss);
    for (ts = fs; ts < ts_ret; ++ts)
        *(fst + ts) = *(temp + ts);
    return ts_ret;
}

/**
 * Recursive merge sort
 * @param buf sorting buffer
 * @param size buffer size
 * @param temp temporary memory
 */
void mergeSort(int* buf, size_t size, int* temp) {
    size_t split = size / 2;

    if (split > 1)
        mergeSort(buf, split, temp);

    if (size - split > 1)
        mergeSort(buf + split, size - split, temp);
        
    merge(buf, split, size - split, temp);
}

/*
 * Sample of aggregation+merge scheme
 *       0   1   2   3   4   5   6   7   8   9   A
 * lv 1: |<-<|   |<-<|   |<-<|   |<-<|   |<-<|   |
 * lv 2: |<-----<|       |<-----<|       |<-----<|
 * lv 3: |<-------------<|               |
 * lv 4: |<-----------------------------<|
 */
/**
 * Aggregation with merge
 * @param myAmount amount of entries on this thread
 * @param gather amounts of entries to receive
 * @param buf data of this thread
 * @param temp temporary memory
 * @param rank mpi rank
 * @param size mpi size
 * @return number of thread send data to
 */
size_t receive(size_t myAmount, int* gather, int* buf, int* temp, int rank, int size)
{
    size_t level = 1;
    while (level < size && !(rank % (level << 1)))
    {
        size_t recvRank = rank + level;
        if (recvRank < size)
        {
            int recvAmount = gather[recvRank];
            MPI_Recv(buf + myAmount, recvAmount, MPI_INT, recvRank, 
                MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            myAmount = merge(buf, myAmount, recvAmount, temp);
        }
        level <<= 1;
    }
    return rank - level;
}

/**
 * Reads file to array of integer numbers
 * @param filename filename
 * @param array output array
 */
void readArray(FILE* fp, int* array)
{
    while ( fscanf(fp, "%d\n", array++) != EOF);
}

/**
 * Parallel sort
 * @param filename name of file with sorting array
 * @param rank mpi rank
 * @param size mpi size
 */
void mpisort(const char* filename, int rank, int size)
{
    /* N is amount of lines in file */
    unsigned N;
    
    int* sendcnts  = (int*)malloc(sizeof(int) * size); /* Amount to send to */
                                                       /* thread */
    int* displs    = (int*)malloc(sizeof(int) * size); /* Displacement */
    int* gathercnt = (int*)malloc(sizeof(int) * size); /* Amount to collect from */
                                                       /* thread */

    size_t amount, gather, send_to;
    int *data, *temp;
    
    /* Zero thread will read the file */
    FILE* fp = NULL;
    if (!rank)
    {
        fp = fopen(filename, "r");
        N = countLines(fp);
    }

    /* Broadcasting amount of numbers in file */
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    /* Scatter split */
    scatter(N, sendcnts, displs, gathercnt, size);

    /* amount of lines to sort on current thread */
    amount = sendcnts[rank];
    
    /* total amount of lines that can be allocated on current thread */
    gather = gathercnt[rank];

    data = (int*)malloc(sizeof(int) * gather);
    temp = (int*)malloc(sizeof(int) * gather);
    
    if (!rank)
    {
        readArray(fp, temp);
        fclose(fp);
    }

    MPI_Scatterv(temp, sendcnts, displs, MPI_INT, 
                 data, amount, MPI_INT,
                 0, MPI_COMM_WORLD);
                 
    if (amount > 1)
        mergeSort(data, amount, temp);

    /* Receive and merge data from younger thread */
    send_to = receive(amount, gathercnt, data, temp, rank, size);

    if (rank)
        /* Send data to elder thread */
        MPI_Send(data, gather, MPI_INT, send_to, 0, MPI_COMM_WORLD);
    else
        /* The eldest thread collected all necessary data and won't send it */
        print(data, N, filename);
    
    free(data);
    free(temp);
    free(gathercnt);
    free(sendcnts);
    free(displs);
}

/**
 * Entry point
 * @param argc argument counter, should be 2
 * @param argv argument list (filename)
 */
int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        double time = -MPI_Wtime();
    
        /* Communicator constants */
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    
        if (argc != 2)
            ERRORPRINT("Syntax error!\n Only argument is file name\n");
    
        mpisort(argv[1], rank, size);
    
        if (!rank)
        fprintf(stdout, "Time is %.15f\n", time += MPI_Wtime());
    }
    MPI_Finalize();
    return 0;
}
