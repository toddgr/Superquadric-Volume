#!/bin/bash

g++ main.cpp -o main -lm -fopenmp

#number of threads:
for t in 1 2 4 8
do
	#number of nodes:
	for s in 10 50 100 250 500 750 1000 5000
	do
		./main $t $s
	done
done