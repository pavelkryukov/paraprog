rem Only argument is the name of program

@echo off
cd %1
rm *.exe
cd ..
pscp -r -p -P 22805 %1\ s91915@openmp.mipt.ru:%1
