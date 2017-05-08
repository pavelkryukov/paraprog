/**
 * loops.c
 *
 * Counting loops with and without OpenMP
 *
 * @author pikryukov
 * @version 2.5
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT Parallel Algorithms course.
 */

#include <stdio.h>  /* fprintf, fopen, fclose */
#include <stdlib.h> /* malloc, free */
#include <math.h>   /* sin */

/* Sizes of array */
#define ISIZE 10000
#define JSIZE 10000

/* Amount of diagonals in array */
#define DSIZE (ISIZE + JSIZE - 1)

/* Size of largest diagonal */
#define HIPOTEN (JSIZE < ISIZE ? JSIZE : ISIZE)

/* Counting function */
#define FUNC(x) (sin(0.00001 * (x)))

/* Name of outputfile */
#ifdef DIAGMODE
#   define FILENAME "new"
#else
#   define FILENAME "old"
#endif

/* Check of compling flags */
#ifndef LOOP
#   error Please define loop number with LOOP define
#elif LOOP > 3 || LOOP < 1
#   error Incorrect loop number, should be from 1 to 3
#endif

/**
 * Allocates memory to 2-dim array
 * @return array pointer
 */
double** init()
{
    int i;
    double** ptr = (double**)malloc(sizeof(double*) * ISIZE);
    for (i = 0; i < ISIZE; ++i)
        ptr[i] = (double*)malloc(sizeof(double) * JSIZE);
    return ptr;
}

/**
 * Frees memory of 2-dim array
 * @param a array pointer
 */
void destroy(double** ptr)
{
    int i;
    for (i = 0; i < ISIZE; ++i)
        free(ptr[i]);
    free(ptr);
}

/**
 * Fills 2-dim array of doubles with integer numbers
 * @param a array pointer
 */
void fill(double** a)
{
    int i;
    for (i = 0; i < ISIZE; i++)
    {
        const int i10 = 10 * i;
        int j;
        for (j = 0; j < JSIZE; j++)
           a[i][j] = i10 + j;
    }
}

/**
 * Prints 2-dim array of doubles with integer numbers into file
 * @param a array pointer
 */
void print(double** a)
{
    int i;
    FILE* ff;
    char filename[20];
    sprintf(filename, "%sresult%d", FILENAME, LOOP);
    ff = fopen(filename,"w");
    for (i = 0; i < HIPOTEN; ++i)
        fprintf(ff, "%f ", a[i][i]);
 
    fclose(ff);
}

/**
 * Recounts 2-dim array of doubles with integer numbers iteratively
 * @param a array pointer
 */
void process(double** a)
{
    int i;
    /* We need to be inside the array in any case! */
    for (i = (LOOP == 2); i < ISIZE - (LOOP == 3); ++i)
    {
        int j;
        for (j = (LOOP == 3); j < JSIZE - (LOOP == 2); ++j)
#if   LOOP == 1
            a[i][j] = FUNC(a[i][j]);
#elif LOOP == 2
            a[i][j] = FUNC(a[i - 1][j + 1]);
#else /* LOOP == 3 */
            a[i][j] = FUNC(a[i + 1][j - 1]);
#endif
    }
}

#ifdef DIAGMODE
/* We are splitting array into diagonals
 *      -> i 
 *    |  #///////////  
 *  j V  #/////////// 
 *       #/////////// 
 *       ############ 
 *  
 *  Examples of diagonals:
 *  a[0][0]
 *  a[0][1] -> a[1][0]
 *  a[0][3] -> a[1][2] -> a[1][2] -> a[3][0]
 *  a[1][3] -> a[2][2] -> a[3][1] -> a[4][0]
 *  Every diagonal is counted independently, so we can separate it to threads
 *
 *  Enumerating diagonal points. 
 *  d[0] -> d[1] -> d[2] -> d[3]
 *  Now loops can be easy rewritten:
 *  loop1: d'[k] = f(d[k])
 *  loop2: d'[k] = f(d[k - 1])
 *  loop3: d'[k - 1] = f(d[k])
 */
/**
 * Recounts 2-dim array of doubles with integer numbers
 * using diagonal computing
 * @param a array pointer
 */
void diagprocess(double** a)
{
    int d;
#pragma omp parallel for shared(a) private(d)
    for (d = 0; d < DSIZE; ++d)
    {
        /* Counting coordinates of 1st point of diagonal */
        const int istart = d < JSIZE ? 0 : d - JSIZE + 1;
        const int jstart = d < JSIZE ? d : JSIZE - 1;
        int size, x;

        /* Counting size of diagonal */
        if (d < JSIZE && d < ISIZE) {
            /* Diagonals that starts on I axis and ends on J axis */
            size = d + 1;
        }
        else if ((d < JSIZE) != (d < ISIZE)) {
            /* Diagonals with ends on opposite sides of array */
            size = HIPOTEN;
        }
        else {
            /* Diagonals with ends on opposite to axis sides of array */
            size = DSIZE - d;
        }

        for (x = (LOOP != 1); x < size; ++x) {
#if   LOOP == 1
            a[istart + x][jstart - x] = FUNC(a[istart + x][jstart - x]);
#elif LOOP == 2
            a[istart + x][jstart - x] = FUNC(a[istart + x - 1][jstart - x + 1]);
#else /* LOOP == 3 */
            a[istart + x - 1][jstart - x + 1] = FUNC(a[istart + x][jstart - x]);
#endif
        }
    }
}
#endif /* DIAGMODE */

/**
 * Entry point
 * @param argc arguments counter
 * @param argv arguments. They are ignored.
 * @return 0
 */
int main(int argc, char **argv)
{
    double** a = init();
    fill(a);
#ifdef DIAGMODE
    diagprocess(a);
#else
    process(a);
#endif
    print(a);
    destroy(a);
    return 0;
}
