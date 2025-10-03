#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "wav_hist.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames
constexpr size_t PCM_MAX_INTERVAL = 32768;   // Max Value for a PCM Short

short quantization(short sample, int bits) {
    int levels = 1 << bits;                       // Level Amount (2^bits)
    int step = (2 * PCM_MAX_INTERVAL) / levels;   // Interval Size
    int quantized = (sample + PCM_MAX_INTERVAL) / step; // PCM Sample from [-32768, 32767] to [0, 65535] for Each Step

    if (quantized >= levels) {
        quantized = levels - 1; // Ensure Quantized Value Doesn't Exceed Maximum Level
    }

    int dequantized = (quantized * step) + (step/2 - PCM_MAX_INTERVAL); // Mapping Quantized Level Back to Original Range Placing Value at the Center of Quantization Interval
    return static_cast<short>(dequantized);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input file> <output file> <bits>\n";
        return 1;
    }

    int bits { stoi(argv[3]) };
    if (bits <= 0 || bits > 16) {
        cerr << "Error: invalid bit amount requested\n";
        return 1;
    }

    SndfileHandle sndFile { argv[1] };
    if (sndFile.error()) {
        cerr << "Error: invalid input file\n";
        return 1;
    }

    if ((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format\n";
        return 1;
    }

    if ((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format\n";
        return 1;
    }

    SndfileHandle sfhOut { argv[2], SFM_WRITE, sndFile.format(), sndFile.channels(), sndFile.samplerate() };
    if (sfhOut.error()) {
        cerr << "Error: invalid output file\n";
        return 1;
    }

    size_t nFrames;
    vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());

    while ((nFrames = sndFile.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
        samples.resize(nFrames * sndFile.channels());
        for (size_t i = 0; i < samples.size(); i++) {
            samples[i] = quantization(samples[i], bits);
        }
        sfhOut.writef(samples.data(), nFrames);
    }

    return 0;
}
