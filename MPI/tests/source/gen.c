/**
 * gen.c
 *
 * Generating random integer numbers
 *
 * @author pikryukov
 * @version 2.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Syntax error!\n");
        fprintf(stderr, "First argument is the amount of ints, second is filename\n");
        return 1;
    }
    
    const unsigned N = atoi(argv[1]);
    unsigned i;
    
    srand(time(NULL));
    
    FILE* fp = fopen(argv[2], "w");
    
    for (i = 0; i < N; ++i)
        fprintf(fp, "%d\n", rand() & 0xFFFF);
        
    fclose(fp);
    
    return 0;
}
        