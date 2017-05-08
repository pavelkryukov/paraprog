/**
 * lobanov1.c
 *
 * Counting integral with MPI or OpenMP
 *
 * @author pikryukov
 * @version 1.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT parallel computing course.
 */

/* C generic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* OpenMP */
#include <omp.h>

/* Defines check */
#ifndef PROBLEM
#   define PROBLEM 1
#endif

#ifndef THREAD_NUM
#   define THREAD_NUM 1
#endif

/* Problem defines section */
#if PROBLEM == 1
#   define FUNC(Y)  exp(Y)
#   define DFUNC(Y) exp(Y)
#   define X0 0
#   define X1 1
#elif PROBLEM == 2
#   define FUNC(Y)  exp(-Y)
#   define DFUNC(Y) (-exp(-Y))
#   define X0 0
#   define X1 1
#elif PROBLEM == 3
#   define FUNC(Y)  A * (Y - Y * Y * Y)
#   define DFUNC(Y) A * (1 - 3 * Y * Y)
#   define X0 (-10)
#   define X1 10
#elif PROBLEM == 4
#   define FUNC(Y)  A * (Y * Y * Y - 1)
#   define DFUNC(Y) A * (3 * Y * Y - 1)
#   define X0 (-10)
#   define X1 10
#else
#   error Invalid task number!
#endif

/* Amount of points */
#define N ((1ull << 20) + 1)

/* Minimum difference between two solutions */
#define H_MIN 1e-9

#define MAX_ITERS 1000

/**
 * Function parameter for tasks 3-4
 */
static double A;

/**
 * 3-diagonal matrix with free members column
 */
typedef struct
{
    double a[N], b[N], c[N], f[N];
} Matrix;

/**
 * Prints 3-diag matrix
 * @param fp output file pointer
 * @param m printed matrix pointer
 */
void PrintMatrix(FILE* fp, const Matrix* m)
{
    size_t i = 1;
    for (; i < N - 1; ++i)
        fprintf(fp, "%8.15f x[%d] + %8.15f x[%d] + %8.15f x[%d] = %8.15f\n",
                   m->a[i], i - 1, m->b[i],  i,   m->c[i], i + 1,  m->f[i]);
}

/**
 * Checks two solutions equality by norm
 * @param x first solution
 * @param y second solution
 * @return 1 if equal, 0 if not
 */
int IsEqualByNorm(const double* x, const double* y)
{
    size_t i;
    for (i = 0; i < N; ++i)
    {
        assert(x[i] == x[i]);
        assert(x[i] != INFINITY);
        if (fabs(x[i] - y[i]) > H_MIN)
            return 0;
    }
    return 1;
}

/**
 * Prints solution
 * @param y solution
 */
void Print(FILE* fp, const double* y)
{
    double begin = X0;
    size_t i;
    static const double h = (double)(X1 - X0) / (N - 1);
    for (i = 0; i < N; ++i)
    {
        fprintf(fp, "#%d#\t%8.15f\t%8.15f\n", i, begin, y[i]);
        begin += h;
    }
}

/**
 * Initializes solution with line, y(X0) = a, y(X1) = b
 * @param y initializing array
 * @param a y(0) value
 * @param b y(1) value
 */
void Init(double* y, double a, double b)
{
    size_t i;
    const double inc = (b - a) * (X1 - X0) / (N - 1);
    double val = a;
    for (i = 0; i < N; ++i)
    {
        y[i] = val;
        val += inc;
    }
    y[N - 1] = b;
}

/*
 * Numerov scheme.
 * 
 * To solve the equation y'' = f(y) we are using Numerov scheme:
 * y[n+1] - 2 * y[n] + y[n-1] = h^2 * (f(y[n]) + (f(y[n+1]) - 2 * f(y[n]) + f(y[n-1])) / 12)
 *
 * Rewriting right part: h^2 * (f(y[n]) * 10 / 12 + f(y[n+1]) / 12 + f(y[n-1]) / 12)
 *
 * Let y0 be the first approximation of the solution, and y1 the next one.
 * Using Newton's approximation rule, we get:
 * f(y[n]) = f(y0[n]) + f'(y0[n]) * (y1[n] - y0[n])
 *
 * After substitution, the result will be a one equation of 3-diag matrix
 * For better readability, two parts were multiplied by 12
 * ( 12/h^2 - f'(y0[n-1])   ) * y1[n - 1] +
 * (-24/h^2 - 10 * f'(y0[n])) * y1[n]     +
 * ( 12/h^2 - f'(y0[n+1])   ) * y1[n + 1] =
 * f(y0[n-1]) - f'(y0[n-1]) * y0[n-1] + f(y0[n+1]) - f'(y0[n+1]) * y0[n+1] + 10 * f(y0[n]) - 10 * f'(y0[n]) * y0[n]
 *  
 */
/**
 * Generates 3-diag matrix for current solution
 * @param m matrix pointer
 * @param y solution
 */
void GenMatrix(Matrix* m, const double* y)
{
    size_t i;
    static const double ih2 = (N - 1) * (N - 1);
    for (i = 1; i < N - 1; ++i)
    {
        m->a[i]  = 12 * ih2 - DFUNC(y[i - 1]);
        m->c[i]  = 12 * ih2 - DFUNC(y[i + 1]);
        m->b[i]  = -24 * ih2 - DFUNC(y[i]) * 10;
        m->f[i]  = FUNC(y[i - 1]) - DFUNC(y[i - 1]) * y[i - 1];
        m->f[i] += (FUNC(y[i]) - DFUNC(y[i]) * y[i]) * 10;
        m->f[i] += FUNC(y[i + 1]) - DFUNC(y[i + 1]) * y[i + 1];
    }
}

/*
 * Common idea of Lobanov Reduction algorithm
 * We've got 2^p + 1 equations. Let p be 4.
 * So there are 17 unknown variables.
 * Two of them are defined on edge, so we may drop these equations
 * 
 * 3 equations can be recombined into one with only 3 variables.
 * So, we can halve amount of equations on one step. Making more steps, we can leave only 1.
 * See the chart below:
 *
 *    x0 x1 -> drop
 * x0 x1 x2 -| (8)
 * x1 x2 x3 -|- x0 x2 x4 -| (4)
 * x2 x3 x4 -| (9)        |
 * x3 x4 x5 -|- x2 x4 x6 -|- x0 x4 x8 -| (2)
 * x4 x5 x6 -| (A)        |            |
 * x5 x6 x7 -|- x4 x6 x8 -| (5)        |
 * x6 x7 x8 -| (B)        |            |
 * x7 x8 x9 -|- x6 x8 xA -|- x4 x8 xC -|- x0 x8 xF (1)
 * x8 x9 xA -| (C)        |            |
 * x9 xA xB -|- x8 xA xC -| (6)        |
 * xA xB xC -| (D)        |            |
 * xB xC xD -|- xA xC xE -|- x8 xC xF -| (3)
 * xC xD xE -| (E)        |
 * xD xE xF -|- xC xE xF -| (7)
 * xE xF xG -| (F)
 * xF xG    -> drop
 *
 * In the end, we've got equation with x0, x8 and xF. We can easily restore x8.
 * Then we continue to restore xN in order mentioned in brackets. Soon we've got all numbers! 
 */
/**
 * Reducts 3-diag matrix with Lobanov algorithm
 * @param m matrix pointer
 */
void Reduct(Matrix* m)
{
    const size_t ls = N - 1;
    size_t step;

    for (step = 2; step < ls; step <<= 1)
    {
        size_t i;
        const size_t s = step >> 1;
#pragma omp parallel for private (i) num_threads(THREAD_NUM)
        for (i = step; i < ls; i += step)
        {
            m->f[i] -= (m->a[i] * m->f[i - s] / m->b[i - s]) + (m->c[i] * m->f[i + s] / m->b[i + s]);
            m->b[i] -= (m->a[i] * m->c[i - s] / m->b[i - s]) + (m->c[i] * m->a[i + s] / m->b[i + s]);
            m->a[i] *= - m->a[i - s] / m->b[i - s];
            m->c[i] *= - m->c[i + s] / m->b[i + s];
       //   putchar('A' + i);
        }
 //     putchar(' ');
    }
}

/**
 * Gets the solution of Lobanov reducted matrix
 * @param m matrix pointer
 * @param y solution pointer
 */
void Deduct(const Matrix* m, double* y)
{
    const size_t ls = N - 1;
    size_t step;
    for (step = ls << 1; step > 0; step >>= 1)
    {
        size_t i;
        for (i = step; i < ls; i += step << 1)
        {
            y[i]  = m->f[i] - m->a[i] * y[i - step] - m->c[i] * y[i + step];
            y[i] /= m->b[i];
        }
    }
}

#define NEW(A, TYPE) ((TYPE*)malloc((A)*sizeof(TYPE)))

/**
 * Solves equation y''=f(y) with y(0)=1, y(1)=b1
 * @param b1 y(1)
 * @param y solution pointer
 * @return amount of iterations
 */
size_t Solve(double param, double* y)
{
    size_t i = 0;
    double* y1 = NEW(N, double);
    double* y2 = NEW(N, double);
    Matrix* m  = NEW(1, Matrix);

#if PROBLEM <= 2
    Init(y1, 1, param);
    A = 0;
#else
    Init(y1, sqrt(2.), sqrt(2.));
    A = param;
#endif
    memcpy(y2, y1, N * sizeof(double));
    do {
        double* c;
        GenMatrix(m, y1);
        Reduct(m);        
  //    PrintMatrix(stdout, m);
        Deduct(m, y2);
        c = y1; y1 = y2; y2 = c;
        ++i;
        putchar('.');
    } while (!IsEqualByNorm(y1, y2) && (i <= MAX_ITERS));
 
    memcpy(y, y1, N * sizeof(double));

    free(y1);
    free(y2);
    free(m);
    
    return i;
}

/**
 * Program core
 * @param arg problem parameter
 * @param filename output file name
 */
void Core(char* arg, char* filename)
{
    size_t iters;
    double param;
    double* y;
    FILE* fp;
    double t = omp_get_wtime();

    param = strtod(arg, NULL);

    y = NEW(N, double);

    iters = Solve(param, y);
    if (iters < MAX_ITERS)
        printf("\nIterations: %d\nTime is %f", iters, omp_get_wtime() - t);
    else
        printf("\nInterrupt. Time is %f", omp_get_wtime() - t);

    fp = fopen(filename, "w");
    Print(fp, y);
    fclose(fp);

    free(y);
}

/**
 * Entry point
 * @param argc
 * @param argv
 * @return 0 on success
 */
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Syntax error! First parameter is %s.\n", PROBLEM < 2 ? "y(1)" : "a");
        fprintf(stderr, "Second parameter is output file name.");
        return 1;
    }
    assert(((N - 1) & (N - 2)) == 0);
    Core(argv[1], argv[2]);
    return 0;
}
