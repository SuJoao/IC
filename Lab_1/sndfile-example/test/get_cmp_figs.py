#!/usr/bin/env python3
import subprocess
import sys
import re
import matplotlib.pyplot as plt

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <original.wav>")
    sys.exit(1)

orig = sys.argv[1]

bits_range = range(15, 0, -1)

results = {
    "bits": [],
    "left": {"rmse": [], "snr": [], "max": []},
    "right": {"rmse": [], "snr": [], "max": []},
    "avg": {"rmse": [], "snr": [], "max": []},
}

for bits in bits_range:
    quant_file = f"quant_{bits}bit.wav"
    print(f"Processing quantization with {bits} bits...")

    subprocess.run(["../bin/wav_quant", orig, quant_file, str(bits)], check=True)
    cmp_out = subprocess.run(
        ["../bin/wav_cmp", orig, quant_file],
        capture_output=True, text=True, check=True
    ).stdout

    def get_val(pattern):
        m = re.search(pattern, cmp_out)
        return float(m.group(1)) if m else None

    results["bits"].append(bits)
    results["left"]["rmse"].append(get_val(r"Channel 0 RMSE:\s*([\d\.Ee+-]+)"))
    results["left"]["snr"].append(get_val(r"Channel 0 SNR:\s*([\d\.Ee+-]+)"))
    results["left"]["max"].append(get_val(r"Channel 0 Max error:\s*([\d\.Ee+-]+)"))

    results["right"]["rmse"].append(get_val(r"Channel 1 RMSE:\s*([\d\.Ee+-]+)"))
    results["right"]["snr"].append(get_val(r"Channel 1 SNR:\s*([\d\.Ee+-]+)"))
    results["right"]["max"].append(get_val(r"Channel 1 Max error:\s*([\d\.Ee+-]+)"))

    results["avg"]["rmse"].append(get_val(r"Average RMSE:\s*([\d\.Ee+-]+)"))
    results["avg"]["snr"].append(get_val(r"Average SNR:\s*([\d\.Ee+-]+)"))
    results["avg"]["max"].append(get_val(r"Average Max error:\s*([\d\.Ee+-]+)"))

def make_plot(channel, metric, color, ylabel, logscale=False):
    bits = results["bits"]
    data = results[channel][metric]
    plt.figure(figsize=(8,5))
    plt.plot(bits, data, 'o-', color=color)
    plt.xlabel("Quantization Bits")
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.gca().invert_xaxis()
    if logscale:
        plt.yscale("log")
        plt.axhline(1e5, color='gray', linestyle='-', linewidth=1)
        plt.text(bits[-1] + 0.3, 1e5, r"", color='gray',
                va='center', ha='left', fontsize=10)
        plt.ylim(top=1e5)

    outname = f"{channel}_{metric}_vs_bits.png"
    plt.savefig(outname, dpi=150)
    plt.close()
    print(f"Saved {outname}")




make_plot("left", "snr", "#1f77b4", "SNR (dB)")
make_plot("left", "rmse", "#ff7f0e", "RMSE", logscale=True)
make_plot("left", "max", "#2ca02c", "Max Error", logscale=True)

make_plot("right", "snr", "#1f77b4", "SNR (dB)")
make_plot("right", "rmse", "#ff7f0e", "RMSE", logscale=True)
make_plot("right", "max", "#2ca02c", "Max Error", logscale=True)

make_plot("avg", "snr", "#1f77b4", "SNR (dB)")
make_plot("avg", "rmse", "#ff7f0e", "RMSE", logscale=True)
make_plot("avg", "max", "#2ca02c", "Max Error", logscale=True)
