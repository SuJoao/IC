#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>
#include <fstream>
#include <fftw3.h>
#include "bit_stream.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input bin file> <output wav file>\n";
        return 1;
    }

    fstream ifs{ argv[1], ios::in | ios::binary };
    if (!ifs.is_open()) { cerr << "Cannot open input file\n"; return 1; }

    BitStream ibs{ ifs, STREAM_READ };

    // read header
    int qbits = ibs.read_n_bits(8);
    int samplerate = ibs.read_n_bits(32);
    size_t block_size = ibs.read_n_bits(16);
    int channels = ibs.read_n_bits(8);

    if (channels != 1) { cerr << "Only mono supported\n"; return 1; }

    SF_INFO sfinfo;
    sfinfo.channels = channels;
    sfinfo.samplerate = samplerate;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SndfileHandle sndFileOut(argv[2], SFM_WRITE, sfinfo.format, sfinfo.channels, sfinfo.samplerate);
    if (sndFileOut.error()) { cerr << "Error creating WAV file\n"; return 1; }

    vector<double> block(block_size);
    vector<short> samples(block_size);
    fftw_plan plan = fftw_plan_r2r_1d(block_size, block.data(), block.data(), FFTW_REDFT01, FFTW_ESTIMATE);

    int shift_bits = 16 - qbits;
    bool finished = false;

    while (!finished) {
        // read one block
        for (size_t i = 0; i < block_size; ++i) {
            int val = ibs.read_n_bits(qbits);
            if (val == EOF) { finished = true; break; }
            // reverse quantization and signed
            unsigned int u = val << shift_bits;
            int signed_sample = static_cast<int>(u) - 32768;
            block[i] = static_cast<double>(signed_sample);
        }

        // inverse DCT
        fftw_execute(plan);

        for (size_t i = 0; i < block_size; i++)
            samples[i] = static_cast<short>(round(block[i]));

        sndFileOut.writef(samples.data(), block_size);
    }

    fftw_destroy_plan(plan);
    ibs.close();
    ifs.close();

    cout << "Decoding done." << endl;
    return 0;
}
