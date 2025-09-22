# Use variables passed from command line: infile and outfile
set terminal pngcairo size 800,600 enhanced font 'Arial,12'
set output outfile

set title "Plot of data from ".infile
set xlabel "X"
set ylabel "Y"
set grid

plot infile using 1:2 with lines title "Data"
set output
