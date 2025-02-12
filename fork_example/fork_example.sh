#!/bin/bash
gcc fork_example.c
./a.out output.txt
cat output.txt
rm a.out output.txt
