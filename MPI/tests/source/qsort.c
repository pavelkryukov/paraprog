/**
 * qsort.c
 *
 * QSort of file
 *
 * @author pikryukov
 * @version 1.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
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
 * Reads file to array of integer numbers
 * @param filename filename
 * @param array output array
 */
void readArray(FILE* fp, int* array)
{
    while ( fscanf(fp, "%d\n", array++) != EOF);
}

/**
 * Compares to ints via pointers
 * @param a fst int pointer
 * @param b snd int pointer
 * @return difference between a and b values
 */
int cmp(const void* a, const void* b)
{
    return *(int*)a - *(int*)b;
}

/**
 * Prints array to file
 * @param data array pointer
 * @param N size of array
 * @param filename file name
 */
void print(const int* data, size_t N, const char* filename)
{
    char file[256] = "qsorted_";
    strcat(file, filename);
    FILE* fp = fopen(file, "w");

    for (size_t i = 0; i < N; ++i)
        fprintf(fp, "%d\n", *(data++));

    fclose(fp);
}
 
/**
 * Entry point
 * @param argc argument counter, should be 2
 * @param argv argument list (filename)
 */
int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Syntax error!\n Only argument is file name\n");
        return 1;
    }
        
    FILE* fp = fopen(argv[1], "r");
    unsigned N = countLines(fp);
    
    int* data = (int*)malloc(sizeof(int) * N);
    readArray(fp, data);
    
    qsort(data, N, sizeof(int), &cmp);
    
    print(data, N, argv[1]);
    
    return 0;
}
