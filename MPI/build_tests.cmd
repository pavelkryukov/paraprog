@echo off
rem test.cmd
rem 
rem MIPT MPI Parallel course test programs build script
rem 
rem @author pikryukov
rem
rem Copyright (C) Kryukov Pavel 2012
rem for MIPT MPI course.

set SOURCE=tests/source
set BIN=tests
set GCCOPT=-O3 -Wall -std=c99 -pedantic

set NAME1=gen
set NAME2=qsort
set NAME3=serial_heat

gcc %GCCOPT% %SOURCE%/%NAME1%.c -o %BIN%/%NAME1%
gcc %GCCOPT% %SOURCE%/%NAME2%.c -o %BIN%/%NAME2%
gcc %GCCOPT% %SOURCE%/%NAME3%.c -o %BIN%/%NAME3%
