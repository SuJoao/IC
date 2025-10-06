# Use variables passed from command line: infile and outfile
set terminal pngcairo size 800,600 enhanced font 'Arial,12'
set output outfile

set title "Plot of data from ".infile
set xlabel "X"
set ylabel "Y"
set grid

set style fill solid 0.8 border -1
set boxwidth 0.9 relative

plot infile using 1:2 with boxes lc rgb "#138cddff" notitle

set output
