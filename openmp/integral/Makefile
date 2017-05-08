# Makefile for Integral task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

CC=gcc
CFLAGS=-O3 -Wall -Werror -pedantic -std=c99
SOURCE=integral.c

openmpintegral:
	$(CC) $(CFLAGS) $(SOURCE) -o $@ -fopenmp

mpiintegral:
	$(CC) $(CFLAGS) $(SOURCE) -o $@ -DUSE_MPI -lmpi

all: openmpintegral mpiintegral

clean:
	rm openmpintegral mpiintegral -f
