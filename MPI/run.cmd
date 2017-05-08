@echo off
rem run.cmd
rem 
rem openmp.mipt.ru interface script
rem 
rem @author pikryukov
rem
rem Copyright (C) Kryukov Pavel 2012
rem for MIPT MPI course.

rem #############
rem DECLARATIONS
rem #############

rem Configuration
set GCCOPT=-O3 -Wall -std=c89 -pedantic -Wno-long-long -Werror -lmpi

rem configuration of remote server
set PORT=22805
set USER=s91606
set SERVER=openmp.mipt.ru
set RSA=ssh/id_rsa

rem #############
rem ARGUMENTS
rem #############

set REMOTE=0
set ENTER=0
set RUN=0
set BUILD=0

rem Options
if "%1"=="-e" (
    set ENTER=1
) else if "%1"=="-r" (
    set REMOTE=1
) else if "%1"=="-l" (
    set REMOTE=0
) else (
    goto :argserror
)

if "%2"=="-r" (
    set RUN=1
) else if "%2"=="-b" (
    set BUILD=1
) else if "%2"=="-rb" (
    set BUILD=1
    set RUN=1
) else (
    goto :argserror
)

rem Run options
set MPIEXEC=mpiexec -n %4
set RUNOPT=%5 %6 %7 %8 %9

set RESULT=file
if "%3"=="heat" (
    set RESULT=result_kryukov.txt
)
if "%3"=="merge" (
    set RESULT=sorted_%5
)

rem files
set SRC=source/%3.c
set BIN=bin/%3.out

rem #############
rem COMMANDS
rem #############

if %ENTER%==1 (
    ssh -p %PORT% -i %RSA% %USER%@%SERVER%
    goto :end
)

if %BUILD%==1 (
    echo [run] local build...
    gcc %SRC% %GCCOPT% -o %BIN%
    if errorlevel 1 goto :gccerror
)

if %REMOTE%==1 (
    echo [run] ping server...
    ping %SERVER% -n 2
    if errorlevel 1 goto :pingerror
    
    if %BUILD%==1 (
        echo [run] copying source file...
        scp -P %PORT% -i %RSA% %SRC% %USER%@%SERVER%:%SRC%
        if errorlevel 1 goto :ssherror
        
        ssh -p %PORT% -i %RSA% %USER%@%SERVER% "chmod 700 %SRC%"
        if errorlevel 1 goto :ssherror
        
        echo [run] remote build...
        ssh -p %PORT% -i %RSA% %USER%@%SERVER% "mpicc %SRC% %GCCOPT% -o %BIN%"
        if errorlevel 1 goto :ssherror
    )
    
    if %RUN%==1 (
        if "%3"=="merge" (
            echo [run] copying data file...
            scp -P %PORT% -i %RSA% %5 %USER%@%SERVER%:%5
            if errorlevel 1 goto :ssherror
        
            ssh -p %PORT% -i %RSA% %USER%@%SERVER% "chmod 700 %5"
            if errorlevel 1 goto :ssherror
        )
    
        echo [run] run...
        ssh -p %PORT% -i %RSA% %USER%@%SERVER% "%MPIEXEC% %BIN% %RUNOPT%"
        if errorlevel 1 goto :ssherror
    
        if "%RESULT%"=="file" (
            echo [run] no result collecting...
        ) else (
            echo [run] collecting data...
            scp -P %PORT% -i %RSA% %USER%@%SERVER%:%RESULT% .\%RESULT%
            if errorlevel 1 goto :ssherror
        )
    )
) else (
    if %RUN%==1 (
        echo [run] local test run...
        %MPIEXEC% %BIN% %RUNOPT%
    )
)
goto :end

rem #################
rem ERRORS
rem #################

:gccerror
echo [run] gcc returned error. Stop.
goto :end

:pingerror
echo [run] %SERVER% is not available now.
goto :end

:ssherror
echo [run] error during SSH session with %SERVER% 
goto :end

:argserror
echo [run] First parameter is type of run 
echo [run] -e   enter server only
echo [run] -r   remote 
echo [run] -l   local
echo [run] Second parameter is what to do
echo [run] -b   build
echo [run] -r   run
echo [run] -br  build and run
echo [run] Third parameter is the name of program
echo [run] Fourh parameter is the number of threads
echo [run] Fifth etc. are arguments of program
goto :end

:end
