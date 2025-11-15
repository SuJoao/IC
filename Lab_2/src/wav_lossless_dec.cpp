#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>
#include <fstream>
#include <chrono>
#include "bit_stream/src/bit_stream.h"
#include "GolombUtils.h"

using namespace std;

inline int predict_from_order(const std::vector<short> &samples, size_t idx, int order)
{
    switch (order)
    {
    case 0:
        return 0;
    case 1:
        return static_cast<int>(samples[idx - 1]);
    case 2:
    {
        int a = samples[idx - 1];
        int b = samples[idx - 2];
        return 2 * a - b;
    }
    case 3:
    {
        int a = samples[idx - 1];
        int b = samples[idx - 2];
        int c = samples[idx - 3];
        return 3 * a - 3 * b + c;
    }
    default:
        return static_cast<int>(samples[idx - 1]);
    }
}

int main(int argc, char *argv[]) {
    auto start_time = chrono::high_resolution_clock::now();

    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input bin file> <output wav file>\n";
        return 1;
    }

    fstream ifs{argv[1], ios::in | ios::binary};
    if (!ifs.is_open()) {
        cerr << "Cannot open input file\n";
        return 1;
    }

    BitStream ibs{ifs, STREAM_READ};

    // Read header
    int samplerate = ibs.read_n_bits(32);
    size_t total_frames = static_cast<size_t>(ibs.read_n_bits(32));
    size_t BLOCK_SIZE = static_cast<size_t>(ibs.read_n_bits(16));
    int channels = ibs.read_n_bits(8);
    int predictor_order = ibs.read_n_bits(8);
    NegativeHandling method = static_cast<NegativeHandling>(ibs.read_n_bits(8));
    bool use_dynamic_m = ibs.read_n_bits(1) == 1;

    uint32_t static_m_value = 0;
    if (!use_dynamic_m) {
        static_m_value = ibs.read_n_bits(32);
    }

    if (channels != 2) {
        cerr << "Only stereo (2 channels) supported\n";
        return 1;
    }

    // Create output WAV file
    SndfileHandle sndFileOut(argv[2], SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, channels, samplerate);
    if (sndFileOut.error()) {
        cerr << "Error creating WAV file\n";
        return 1;
    }

    vector<short> block_samples(BLOCK_SIZE * channels);
    vector<short> mid(BLOCK_SIZE);
    vector<short> side(BLOCK_SIZE);

    size_t frames_written = 0;
    int mid_m, side_m;

    mid_m = static_m_value;
    side_m = static_m_value;

    try {
        while (frames_written < total_frames) {

            if (use_dynamic_m) {
                mid_m = ibs.read_n_bits(32);
                if(mid_m == EOF) break;
                if(channels == 2){
                    side_m = ibs.read_n_bits(32);
                    if(side_m == EOF) break;
                }
            }

            GolombUtils golomb_mid(mid_m, method);
            GolombUtils golomb_side(side_m, method);

            size_t frames_to_decode = BLOCK_SIZE;
            if (frames_written + BLOCK_SIZE > total_frames) {
                frames_to_decode = total_frames - frames_written;
            }

            // Determine warmup
            size_t warmup = static_cast<size_t>(predictor_order);
            if (warmup > frames_to_decode) warmup = frames_to_decode;
            if (warmup < 1) warmup = 1;

            // Decode warmup samples
            for (size_t i = 0; i < warmup; i++) {
                mid[i] = golomb_mid.golomb_decode(&ibs);
                if (channels == 2) {
                    side[i] = golomb_side.golomb_decode(&ibs);
                }
            }

            // Decode remaining samples as residuals and reconstruct
            for (size_t i = warmup; i < frames_to_decode; i++) {
                int predicted_mid = predict_from_order(mid, i, predictor_order);
                int residual_mid = golomb_mid.golomb_decode(&ibs);
                mid[i] = predicted_mid + residual_mid;

                if (channels == 2) {
                    int predicted_side = predict_from_order(side, i, predictor_order);
                    int residual_side = golomb_side.golomb_decode(&ibs);
                    side[i] = predicted_side + residual_side;
                }
            }

            // Reconstruct samples
            if (channels == 1) {
                // Mono: mid channel is the audio
                for (size_t i = 0; i < frames_to_decode; i++) {
                    block_samples[i] = static_cast<short>(mid[i]);
                }
            } else {
                // Stereo: reconstruct L and R from mid/side
                // Encoder: mid = (L+R)/2, side = L-R
                for (size_t i = 0; i < frames_to_decode; i++) {
                    int m = mid[i];
                    int s = side[i];

                    // L = mid + (side+1)/2, R = mid - side/2
                    int L = m + ((s + 1) >> 1);
                    int R = m - (s >> 1);

                    block_samples[i * 2 + 0] = static_cast<short>(L);
                    block_samples[i * 2 + 1] = static_cast<short>(R);
                }
            }

            sndFileOut.writef(block_samples.data(), static_cast<sf_count_t>(frames_to_decode));
            frames_written += frames_to_decode;

            if (frames_written >= total_frames) {
                break;
            }
        }
    } catch (...) {
        cerr << "Decoding error occurred\n";
        return 1;
    }

    ifs.close();

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    cout << "Decoding done." << endl;
    cout << "Time elapsed: " << duration.count() << " ms" << endl;
    return 0;
}
