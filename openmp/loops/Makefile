# Makefile for Loop task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

CC=gcc
CFLAGS=-O3 -Wall -Werror -pedantic -ansi -lm
DIAGMODE=-fopenmp -DDIAGMODE
SOURCE=loops.c

oldloop%: $(SOURCE)
	$(CC) $(CFLAGS) -DLOOP=$* $(SOURCE) -o $@

newloop%: $(SOURCE)
	$(CC) $(CFLAGS) -DLOOP=$* $(SOURCE) -o $@ $(DIAGMODE)

all: oldloop1 oldloop2 oldloop3 newloop1 newloop2 newloop3

clean:
	rm oldloop* newloop* oldresult* newresult* -f
