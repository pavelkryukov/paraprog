#!/usr/bin/bash
# run.sh
#
# Script for running Loop task
#
# @author pikryukov
#
# e-mail: kryukov@frtk.ru
#
# Copyright (C) Kryukov Pavel 2012
# for MIPT Parallel Algorithms course.

if [ "$1" != "1" -a "$1" != "2" -a "$1" != "3" ];
then
    echo "Syntax error! First argument is number of cycle!"
    exit
fi

make oldloop$1 newloop$1

echo

echo -n "Loop$1 OLD"
time ./oldloop$1
echo -n "Hash is "
md5sum oldresult$1 | cut -d ' ' -f 1

echo

echo -n "Loop$1 NEW"
time ./newloop$1
echo -n "Hash is "
md5sum newresult$1 | cut -d ' ' -f 1
