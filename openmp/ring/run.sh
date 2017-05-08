#!/usr/bin/bash
# run.sh
#
# Script for running Ring task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

make all

echo

echo "Pthread signal:"
time ./pthreadring.exe 16 2

echo

echo "MPI integral:"
mpirun -n 16 ./mpiring.exe 1 2> /dev/null
