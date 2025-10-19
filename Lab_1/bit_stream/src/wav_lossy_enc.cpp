#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>
#include <fstream>
#include <fftw3.h>
#include "bit_stream.h"

using namespace std;

size_t BLOCK_SIZE = 1024;

int main(int argc, char *argv[]) {
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

    if(argc >=5 ) {
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
    obs.write_n_bits(BLOCK_SIZE, 16);
    obs.write_n_bits(1, 8);

    int channels = sndFile.channels();
    vector<short> input(BLOCK_SIZE * channels);
    vector<double> block(BLOCK_SIZE);

    fftw_plan plan_dct = fftw_plan_r2r_1d(
        BLOCK_SIZE, block.data(), block.data(),
        FFTW_REDFT10, FFTW_ESTIMATE);

    size_t nFrames;
    int shiftBits = 16 - qbits;

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

       // scale DCT coefficients before quantization
        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            double scaled = block[i] / (BLOCK_SIZE * 2);
            int32_t unsigned_value = static_cast<int32_t>(scaled + 32768);
            uint16_t quantized_value = unsigned_value >> shiftBits;
            obs.write_n_bits(quantized_value, qbits);
        }
    }

    fftw_destroy_plan(plan_dct);
    obs.close();
    ofs.close();

    cout << "Encoding done. Stereo → mono + DCT quantized to " << qbits << " bits." << endl;
    return 0;
}
