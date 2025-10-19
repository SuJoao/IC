#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>
#include <fstream>
#include <fftw3.h>
#include <cstring>
#include <chrono>
#include "bit_stream.h"

using namespace std;

size_t BLOCK_SIZE = 1024;

int main(int argc, char *argv[]) {
    auto start_time = chrono::high_resolution_clock::now();

    if (argc < 4) {
        cerr << "Usage: " << argv[0]
             << " <input wav> <output bin> <quant_bits> <block_size>\n";
        return 1;
    }

    SndfileHandle sndFile{argv[1]};
    if (sndFile.error()) {
        cerr << "Error: invalid input file\n";
        return 1;
    }
    if ((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not WAV format\n";
        return 1;
    }
    if ((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not PCM_16 format\n";
        return 1;
    }

    int qbits = stoi(argv[3]);
    if (qbits <= 0 || qbits > 16) {
        cerr << "Error: quantization bits must be between 1 and 16\n";
        return 1;
    }

    if (argc >= 5) {
        BLOCK_SIZE = static_cast<size_t>(stoi(argv[4]));
        if (BLOCK_SIZE == 0) {
            cerr << "Error: block size must be positive\n";
            return 1;
        }
    }

    fstream ofs{argv[2], ios::out | ios::binary};
    if (!ofs.is_open()) {
        cerr << "Error opening output file\n";
        return 1;
    }

    BitStream obs{ofs, STREAM_WRITE};

    // header
    obs.write_n_bits(qbits, 8);
    obs.write_n_bits(sndFile.samplerate(), 32);
    obs.write_n_bits(static_cast<uint32_t>(BLOCK_SIZE), 16);
    obs.write_n_bits(1, 8);

    int channels = sndFile.channels();
    vector<short> input(BLOCK_SIZE * channels);
    vector<double> block(BLOCK_SIZE);

    fftw_plan plan_dct = fftw_plan_r2r_1d(
        static_cast<int>(BLOCK_SIZE), block.data(), block.data(),
        FFTW_REDFT10, FFTW_ESTIMATE);

    size_t nFrames;
    while ((nFrames = sndFile.readf(input.data(), BLOCK_SIZE))) {
        // get mono channel
        for (size_t i = 0; i < nFrames; i++) {
            double sum = 0.0;
            for (int ch = 0; ch < channels; ++ch)
                sum += input[i * channels + ch];
            block[i] = sum / channels;
        }

        // add padding if needed
        for (size_t i = nFrames; i < BLOCK_SIZE; i++)
            block[i] = 0.0;

        // apply DCT
        fftw_execute(plan_dct);

        // normalize DCT coefficients and get scale
        double inv_norm = 1.0 / (2.0 * static_cast<double>(BLOCK_SIZE));
        double maxabs = 0.0;
        for (size_t i = 0; i < BLOCK_SIZE; ++i){
            block[i] *= inv_norm;
            maxabs = max(maxabs, fabs(block[i]));
        }

        // get [-max_q, max_q] and calculate scale
        int32_t max_q = (1 << (qbits - 1)) - 1;
        double scale = (maxabs > 0.0) ? (static_cast<double>(max_q) / maxabs) : 1.0;

        // save scale as 32-bit float 
        uint32_t scale_bits;
        float scale_f = static_cast<float>(scale);
        memcpy(&scale_bits, &scale_f, 4);
        obs.write_n_bits(scale_bits, 32);

        // quantize dct coefficients and write
        uint32_t mask = (1u << qbits) - 1u;
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            double scaled = block[i] * scale;
            int32_t q = static_cast<int32_t>(round(scaled));
            if (q > max_q) q = max_q;
            if (q < -max_q - 1) q = -max_q - 1;
            uint32_t packed = static_cast<uint32_t>(q) & mask;
            obs.write_n_bits(packed, qbits);
        }
    }

    fftw_destroy_plan(plan_dct);
    obs.close();
    ofs.close();

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    cout << "Encoding done. Stereo → mono + DCT quantized to " << qbits << " bits." << endl;
    cout << "Time elapsed: " << duration.count() << " ms" << endl;
    return 0;
}
