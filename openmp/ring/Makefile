# Makefile for Ring task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

CC=gcc
CFLAGS=-O3 -Wall -Werror -pedantic -std=c89
CXX=g++
CXXFLAGS=-O3 -Wall -Werror -pedantic -std=c++98

pthreadring: ring.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

mpiring: ring.c
	$(CC) $(CFLAGS) $^ -o $@ -lmpi

all: pthreadring mpiring

clean:
	rm pthreadring mpiring -f
