set title 'Latency and Throughput vs. Number of Threads'
set xlabel 'Number of Threads'
set ylabel 'Latency (µs)'
set y2label 'Throughput (accesses/sec)'
set ytics nomirror
set y2tics
set grid
set terminal pngcairo size 800,600
set output 'read_latency_throughput.png'
plot 'read_latency_throughput.dat' using 1:2 with lines title 'Latency' lw 2 axis x1y1, \
     'read_latency_throughput.dat' using 1:3 with lines title 'Throughput' lw 2 axis x1y2
