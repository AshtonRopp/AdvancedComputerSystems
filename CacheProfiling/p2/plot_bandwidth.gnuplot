set title 'Bandwidth vs. Read Ratio'
set xlabel 'Read Ratio'
set ylabel 'Bandwidth (GB/s)'
set key outside right top
set grid
set style line 1 lt 1 lc rgb '#FF0000' lw 2
set style line 2 lt 1 lc rgb '#00FF00' lw 2
set style line 3 lt 1 lc rgb '#0000FF' lw 2
set style line 4 lt 1 lc rgb '#FF00FF' lw 2
set style line 5 lt 1 lc rgb '#00FFFF' lw 2
plot \
'bandwidth_data.dat' index 0 using 1:2 with lines linestyle 1 title 'Access size: 64 bytes', \
'bandwidth_data.dat' index 1 using 1:2 with lines linestyle 2 title 'Access size: 256 bytes', \
'bandwidth_data.dat' index 2 using 1:2 with lines linestyle 3 title 'Access size: 512 bytes', \
'bandwidth_data.dat' index 3 using 1:2 with lines linestyle 4 title 'Access size: 1024 bytes', \
'bandwidth_data.dat' index 4 using 1:2 with lines linestyle 5 title 'Access size: 2048 bytes'
