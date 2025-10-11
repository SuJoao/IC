#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <wavfile>"
    exit 1
fi

WAV="$1"

if [ ! -f "$WAV" ]; then
    echo "Error: WAV file '$WAV' not found."
    exit 1
fi

channels=(0 1 2 3)
bins=(1 2 4 8 16 32 64 128 1024 2048)

# channels: 0-left, 1-right, 2-mid, 3-right
for ch in "${channels[@]}"; do
    for bin in "${bins[@]}"; do
        dat="ch${ch}_bin${bin}.dat"
        png="ch${ch}_bin${bin}.png"

        echo "Processing channel $ch with $bin bins..."

        # generate histogram data
        ../bin/wav_hist "$WAV" "$ch" "$bin" > "$dat"

        # plot with gnuplot
        gnuplot -e "infile='${dat}'; outfile='${png}'" hist_plot.gp
    done
done
