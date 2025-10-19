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

int main(int argc, char *argv[]) {
    auto start_time = chrono::high_resolution_clock::now();

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
    size_t block_size = static_cast<size_t>(ibs.read_n_bits(16));
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

    fftw_plan plan = fftw_plan_r2r_1d(static_cast<int>(block_size), block.data(), block.data(), FFTW_REDFT01, FFTW_ESTIMATE);

    uint32_t mask = (1u << qbits) - 1u;
    uint32_t signbit = 1u << (qbits - 1);

    bool finished = false;
    while (!finished) {
        
        // read 32-bit scale bit pattern (float)
        int tmp = ibs.read_n_bits(32);
        if (tmp < 0) break;
        uint32_t scale_bits = static_cast<uint32_t>(tmp);
        float scale_f;
        memcpy(&scale_f, &scale_bits, 4);
        double scale = static_cast<double>(scale_f);
        if (scale == 0.0) scale = 1.0;

        // read one block of quantized coefficients
        for (size_t i = 0; i < block_size; ++i) {
            int v = ibs.read_n_bits(qbits);
            if (v < 0) { finished = true; break; }
            uint32_t u = static_cast<uint32_t>(v) & mask;
            
            // sign-extend
            int32_t sval;
            if (u & signbit) {
                sval = static_cast<int32_t>(u | (~mask));
            } else {
                sval = static_cast<int32_t>(u);
            }
            
            double coef = static_cast<double>(sval) / scale;
            block[i] = coef;
        }

        if (finished) break;

        // inverse DCT
        fftw_execute(plan);

        // round and clamp to 16-bit PCM
        for (size_t i = 0; i < block_size; ++i) {
            double val = round(block[i]);
            if (val > 32767.0) val = 32767.0;
            if (val < -32768.0) val = -32768.0;
            samples[i] = static_cast<short>(val);
        }

        sndFileOut.writef(samples.data(), static_cast<sf_count_t>(block_size));
    }

    fftw_destroy_plan(plan);
    ibs.close();
    ifs.close();

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    cout << "Decoding done." << endl;
    cout << "Time elapsed: " << duration.count() << " ms" << endl;
    return 0;
}
