#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_wav>"
    exit 1
fi

INPUT_WAV="$1"
CHANNEL=0

# Paths to binaries
WAV_ENC="../bin/wavquantenc"
WAV_DEC="../bin/wavquantdec"
WAV_HIST="../../sndfile-example/bin/wav_hist"
GNUPLOT_SCRIPT="../../sndfile-example/test/hist_plot.gp"

# Check binaries exist
for BIN in "$WAV_ENC" "$WAV_DEC" "$WAV_HIST"; do
    if [ ! -x "$BIN" ]; then
        echo "Error: $BIN not found or not executable"
        exit 1
    fi
done

# Quantization loop: 15 → 1 bits
for Q in {15..1}; do
    OUT_BIN="${INPUT_WAV%.wav}_q${Q}.bin"
    OUT_WAV="${INPUT_WAV%.wav}_q${Q}.wav"
    OUT_DAT="${INPUT_WAV%.wav}_q${Q}.dat"
    OUT_PNG="${INPUT_WAV%.wav}_q${Q}.png"

    echo "=== Quantizing $INPUT_WAV to $Q bits ==="
    echo "Encoding → $OUT_BIN"
    "$WAV_ENC" "$INPUT_WAV" "$OUT_BIN" "$Q"

    echo "Decoding → $OUT_WAV"
    "$WAV_DEC" "$OUT_BIN" "$OUT_WAV"

    echo "Generating histogram for $OUT_WAV → $OUT_DAT"
    "$WAV_HIST" "$OUT_WAV" "$CHANNEL" "1" > "$OUT_DAT"

    echo "Plotting histogram → $OUT_PNG"
    gnuplot -e "infile='${OUT_DAT}'; outfile='${OUT_PNG}'" "$GNUPLOT_SCRIPT"

    echo
done

echo "All quantization levels processed successfully."
