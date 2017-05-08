#!/usr/bin/bash
# run.sh
#
# Script for running Integral task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

make all

echo

echo "OpenMP integral:"
time ./openmpintegral

echo

echo "MPI integral:"
mpirun -n 16 ./mpiintegral 2> /dev/null
