#!/bin/bash
cd p1
g++ cache_latency_1.cpp -o cache_latency.exe
./cache_latency.exe
cd ..

cd p2
g++ bandwidth_2.cpp -o bandwidth.exe -O3 -std=c++11 -pthread
./bandwidth.exe
cd ..

cd p3
g++ queuing_theory_3.cpp -O3 -o queuing_theory.exe -std=c++11 -pthread
./queuing_theory.exe
cd ..

cd p4
g++ cache_miss_4.cpp -o cache_miss.exe
./cache_miss.exe
cd ..

cd p5
source ./TB.sh
cd ..