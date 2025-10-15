#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_wav>"
    exit 1
fi

INPUT_WAV="$1"
CHANNEL=0

# Paths to binaries
WAV_QUANT="../bin/wav_quant"
WAV_HIST="../bin/wav_hist"

# Check binaries exist
if [ ! -x "$WAV_QUANT" ]; then
    echo "Error: $WAV_QUANT not found or not executable"
    exit 1
fi
if [ ! -x "$WAV_HIST" ]; then
    echo "Error: $WAV_HIST not found or not executable"
    exit 1
fi

# Quantization loop 15 → 1
for Q in {15..1}; do
    OUT_WAV="${INPUT_WAV%.wav}_chall_q${Q}.wav"
    OUT_DAT="${INPUT_WAV%.wav}_chall_q${Q}.dat"
    PNG="${INPUT_WAV%.wav}_chall_q${Q}.png"

    echo "Quantizing $INPUT_WAV to $Q bits → $OUT_WAV"
    "$WAV_QUANT" "$INPUT_WAV" "$OUT_WAV" "$Q"

    echo "Generating histogram for $OUT_WAV"
    "$WAV_HIST" "$OUT_WAV" "$CHANNEL" "1" > "$OUT_DAT"

    echo "Plotting histogram → $PNG"
    gnuplot -e "infile='${OUT_DAT}'; outfile='${PNG}'" hist_plot.gp
done
