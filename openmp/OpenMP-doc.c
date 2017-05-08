#error File has not been written to be compiled

/// 21.09.2012

// How To Build: gcc -fopenmp
#include <omp.h>

// Parallel Just:
#pragma omp parallel shared (a) private (j)
    {
        // will be executed by all threads
    }

// Parallel FOR:
    int a[] = { };
    int j = 0; // integer only
#pragma omp parallel for shared (a) private (j)
    for (j = 0; j < N; ++j) // only <, <= (for ++), >, >= (for --)
    {
        a[j] *= a[j]; // no break/continue, exit() only
    }
    
int omp_get_thread_num(); // returns thread number
OMP_NUM_THREADS; // amount of threads
  
// Reduction:
#pragma omp parallel reduction(+: sum)
    {
#pragma omp for
        for (;;)
        {
            sum += f(a[j]);
        }
    }
    return a;

// Sync
#pragma omp critical(label) // critical section
    {

    }

#pragma omp atomic // atomic operation
    {

    }
    
// Blocks
omp_lock_t lock;         // mutex
omp_init_lock(&lock);    // init mutex
omp_set_lock(&lock);     // lock mutex
omp_unset_lock(&lock);   // unlock mutex
omp_test_lock(&lock);    // check if locked
omp_destroy_lock(&lock); // destroy mutex

/// 26.10.2012

// Internal loop
#pragma omp parallel
    while (i < N)
    {
#pragma omp for
        for (j = 0; j < M; ++j)
        {

        }
#pragma omp barrier
#pragma omp single
        {
            ++i;
        }
    }

// Dynamic scheduling
#pragma omp schedule(dynamic)
    for (i = 0; i < 100; ++i)
    {

    }

// Ordered execution
#pragma omp for ordered
    for (i = 0; i < 100; ++i)
    {
#pragma omp ordered
        {
            writeresult(i);
        }
    }
    
// Section
#pragma omp parallel
    {
#pragma omp single
        {

        } // While one is working, others wait
#pragma omp section // Sections executed in parallel
        {

        }
#pragma omp section
        {

        }
    }
