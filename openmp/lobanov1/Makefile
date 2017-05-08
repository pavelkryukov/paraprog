# Makefile for Lobanov task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

CC=gcc
CFLAGS=-O2 -Wall -Werror -pedantic -std=c99 -fopenmp -g
SOURCE=lobanov1.c
MAKEFILE=Makefile

lobanov1-%: $(SOURCE) $(MAKEFILE)
	$(CC) $(CFLAGS) $(SOURCE) -o $@ -fopenmp -DPROBLEM=$* -DTHREAD_NUM=1

all: lobanov1-1 lobanov1-2 lobanov1-3 lobanov-4

clean:
	rm lobanov1-* -f
