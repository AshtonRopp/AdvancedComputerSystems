set title 'Matrix Multiplication Optimizations'
set xlabel 'Matrix Size'
set ylabel 'Time'
set ytics
set grid
set key inside left
set terminal pngcairo size 800,600
set output 'plot.png'
plot 'plot.dat' using 1:2 with lines title 'No Optimizations' lw 2 axis x1y1, \
     'plot.dat' using 1:3 with lines title 'Cache Optimization' lw 2 axis x1y1, \
     'plot.dat' using 1:4 with lines title 'Multithread Optimization' lw 2 axis x1y1, \
     'plot.dat' using 1:5 with lines title 'SIMD Optimization' lw 2 axis x1y1, \
     'plot.dat' using 1:6 with lines title 'All Optimizations' lw 2 axis x1y1
