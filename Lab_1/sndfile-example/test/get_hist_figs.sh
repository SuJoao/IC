#!/bin/bash

WAV=$1

# channels: 0-left, 1-right, 2-mid, 3-right
for ch in 0 1 2 3; do
  for bin in 1 2 4 8; do
    dat="ch${ch}_bin${bin}.dat"
    png="ch${ch}_bin${bin}.png"

    # generate histogram data
    ../bin/wav_hist "$WAV" "$ch" "$bin" > "$dat"

    # plot with gnuplot
    gnuplot -e "infile='${dat}'; outfile='${png}'" plot.gp
  done
done
