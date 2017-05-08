@echo off

if "%1"=="heat" goto :heat
if "%1"=="merge" goto :merge
goto :error

:merge

set ORIGINAL=qsorted_testfile
set PARALLEL=sorted_testfile

echo [test] Generating test list...
tests\gen 1 testfile

echo [test] Sorting list with QuickSort...
tests\qsort testfile

call run %2 -rb merge %3 testfile

erase testfile

goto :compare

:heat
set ORIGINAL=serial_result.txt
set PARALLEL=result_kryukov.txt

echo [test] Sovling with serial algorithm...
tests\serial_heat 2.0 100 1.0 1.0

call run %2 -rb heat %3 2.0 100 1.0 1.0

:compare

echo [test] Converting file to DOS format...
perl -pi.bak -e 's/\n/\r\n/g' %PARALLEL%

echo [test] Compare...
echo n | comp %ORIGINAL% %PARALLEL%
echo.

echo [test] Cleanup...
erase %ORIGINAL% %PARALLEL% %PARALLEL%.bak

goto :end

:error

echo [test] Syntax error

:end
