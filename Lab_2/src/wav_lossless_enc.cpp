#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>
#include <numeric>
#include <fstream>
#include <cstring>
#include <chrono>
#include "bit_stream/src/bit_stream.h"
#include "GolombUtils.h"

enum PredictionMode {
    order0 = 0,
    order1 = 1,
    order2 = 2,
    order3 = 3,
};

using namespace std;

double mean_abs(const std::vector<short> &values, size_t nFrames) {
    if (values.empty())
        return 0.0;
    double sum_abs = std::accumulate(values.begin(), values.end(), 0.0,
                                     [](double acc, short v)
                                     {
                                         return acc + std::abs(static_cast<double>(v));
                                     });
    return sum_abs / static_cast<double>(nFrames);
}

inline int floor_div2(int x) {
    if (x >= 0)
        return x / 2;
    return -((-x + 1) / 2);
}

inline int predict_from_order(const std::vector<short> &samples, size_t idx, int order) {
    switch (order) {
    case 0:
        return 0;
    case 1:
        return static_cast<int>(samples[idx - 1]);
    case 2:
    {
        int a = samples[idx - 1];
        int b = samples[idx - 2];
        // 2*a - b
        return 2 * a - b;
    }
    case 3:
    {
        int a = samples[idx - 1];
        int b = samples[idx - 2];
        int c = samples[idx - 3];
        // 3*a - 3*b + c
        return 3 * a - 3 * b + c;
    }
    default:
        return static_cast<int>(samples[idx - 1]);
    }
}

size_t BLOCK_SIZE = 1024;

void print_usage(const char* prog_name) {
    cout << "Usage: " << prog_name << " <input.wav> <output.bin> [options]\n\n";
    cout << "Required:\n";
    cout << "  <input.wav>       Input WAV file (stereo, PCM_16)\n";
    cout << "  <output.bin>      Output binary file\n\n";
    cout << "Options:\n";
    cout << "  -b <block_size>   Block size for encoding (default: 1024)\n";
    cout << "  -p <order>        Predictor order 0-3 (default: 1)\n";
    cout << "  -m <method>       Negative handling method:\n";
    cout << "                    'zigzag', 'sign_magnitude'\n";
    cout << "                    (default: zigzag)\n";
    cout << "  -gd               Use dynamic Golomb m (default)\n";
    cout << "  -gs <m_value>     Use static Golomb m value\n\n";
    cout << "Examples:\n";
    cout << "  " << prog_name << " input.wav output.bin\n";
    cout << "  " << prog_name << " input.wav output.bin -b 2048 -p 2\n";
    cout << "  " << prog_name << " input.wav output.bin -m sign_magnitude\n";
    cout << "  " << prog_name << " input.wav output.bin -gs 8\n";
}

NegativeHandling parse_method(const char* method_str) {
    if (strcmp(method_str, "zigzag") == 0) {
        return ZIGZAG;
    } else if (strcmp(method_str, "sign_magnitude") == 0) {
        return SIGN_MAGNITUDE;
    } else {
        cerr << "Error: Invalid method. Use 'zigzag', 'sign_magnitude''\n";
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    auto start_time = chrono::high_resolution_clock::now();

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];
    int predictor_order = 1;
    NegativeHandling method = ZIGZAG; // default
    bool use_dynamic_m = true; // default to dynamic
    uint32_t static_m_value = 1;

    // Parse optional arguments starting from argv[3]
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            try {
                long bs = stol(argv[++i]);
                if (bs <= 0) {
                    cerr << "Error: block size must be positive\n";
                    return 1;
                }
                BLOCK_SIZE = static_cast<size_t>(bs);
            } catch (...) {
                cerr << "Error: invalid block size\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            try {
                int po = stoi(argv[++i]);
                if (po < 0 || po > 3) {
                    cerr << "Error: predictor_order must be between 0 and 3\n";
                    return 1;
                }
                predictor_order = po;
            } catch (...) {
                cerr << "Error: invalid predictor order\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = parse_method(argv[++i]);
        } else if (strcmp(argv[i], "-gd") == 0) {
            use_dynamic_m = true;
        } else if (strcmp(argv[i], "-gs") == 0 && i + 1 < argc) {
            try {
                int m = stoi(argv[++i]);
                if (m <= 0) {
                    cerr << "Error: static m value must be positive\n";
                    return 1;
                }
                static_m_value = static_cast<uint32_t>(m);
                use_dynamic_m = false;
            } catch (...) {
                cerr << "Error: invalid static m value\n";
                return 1;
            }
        } else {
            cerr << "Error: Unknown option '" << argv[i] << "'\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    SndfileHandle sndFile{input_file.c_str()};
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

    int channels = sndFile.channels();
    if (channels != 1 && channels != 2) {
        cerr << "Error: input file must be mono (1 channel) or stereo (2 channels) for mid/side.\n";
        return 1;
    }

    fstream ofs{output_file, ios::out | ios::binary};
    if (!ofs.is_open()) {
        cerr << "Error opening output file\n";
        return 1;
    }

    BitStream obs{ofs, STREAM_WRITE};

    cout << "Encoding parameters:\n";
    cout << "  Block size: " << BLOCK_SIZE << "\n";
    cout << "  Predictor order: " << predictor_order << "\n";
    cout << "  Negative handling method: " << (method == ZIGZAG ? "zigzag" : "sign_magnitude") << "\n";
    cout << "  Golomb m: " << (use_dynamic_m ? "dynamic" : to_string(static_m_value)) << "\n";
    cout << "\n";
    cout << "Encoding " << input_file << " to " << output_file << "\n";
    cout << "  Sample rate: " << sndFile.samplerate() << "\n";
    cout << "  Channels: " << channels << "\n";
    cout << "  Total frames: " << sndFile.frames() << "\n\n";

    // Debug: open residuals file
    //fstream debug_file{"residuals_debug.txt", ios::out};
    //if (debug_file.is_open()) {
    //    debug_file << "block,sample,mid_residual,side_residual\n";
    //}

    // header: samplerate (32 bits), frames (32 bits), block_size (16 bits), channels (8 bits), predictor_order (8 bits), method (8 bits), use_dynamic_m (1 bit)

    obs.write_n_bits(static_cast<uint32_t>(sndFile.samplerate()), 32);
    obs.write_n_bits(static_cast<uint32_t>(sndFile.frames()), 32);
    obs.write_n_bits(static_cast<uint32_t>(BLOCK_SIZE), 16);
    obs.write_n_bits(static_cast<uint32_t>(channels), 8);
    obs.write_n_bits(static_cast<uint32_t>(predictor_order), 8);
    obs.write_n_bits(static_cast<uint32_t>(method), 8);
    obs.write_n_bits(use_dynamic_m ? 1 : 0, 1);

    if (!use_dynamic_m) {
        obs.write_n_bits(static_m_value, 32);
    }

    vector<short> block_samples(BLOCK_SIZE * channels);
    vector<short> mid(BLOCK_SIZE);
    vector<short> side(BLOCK_SIZE);

    //size_t block_num = 0;
    size_t nFrames;
    while ((nFrames = sndFile.readf(block_samples.data(), static_cast<int>(BLOCK_SIZE)))) {

        if (channels == 1) {
            // Mono use only mid
            for (size_t i = 0; i < nFrames; ++i) {
                mid[i] = block_samples[i];
                side[i] = 0; // not used for mono
            }
        } else {
            // Stereo convert to mid/side
            for (size_t i = 0; i < nFrames; ++i) {
                // L = block_samples[i*2 + 0], R = block_samples[i*2 + 1]
                int L = block_samples[i * channels + 0];
                int R = block_samples[i * channels + 1];

                // mid = (L + R) / 2 (integer division, truncates toward zero)
                mid[i] = floor_div2(L + R);

                // side = L - R
                side[i] = L - R;
            }
        }

        // Determine warmup size
        size_t warmup = static_cast<size_t>(predictor_order);
        if (warmup > nFrames) warmup = nFrames;

        // Compute residuals for the block
        vector<short> mid_residuals(nFrames - warmup);
        vector<short> side_residuals(nFrames - warmup);

        for (size_t i = warmup; i < nFrames; ++i) {
            int predicted_mid = predict_from_order(mid, i, predictor_order);
            mid_residuals[i - warmup] = static_cast<short>(static_cast<int>(mid[i]) - predicted_mid);

            if (channels == 2) {
                int predicted_side = predict_from_order(side, i, predictor_order);
                side_residuals[i - warmup] = static_cast<short>(static_cast<int>(side[i]) - predicted_side);
            }
        }

        uint32_t mid_m, side_m;
        mid_m = static_m_value;
        side_m = static_m_value;

        if (use_dynamic_m) {
            // Calculate optimal m values from residuals
            double mid_mean = mean_abs(mid_residuals, nFrames - warmup);
            double mid_alpha = mid_mean / (mid_mean + 1.0);
            if (mid_alpha < 0.001) mid_alpha = 0.001;
            if (mid_alpha > 0.999) mid_alpha = 0.999;
            mid_m = ceil(-1 / log(mid_alpha));
            if (mid_m < 1) mid_m = 1;

            // write mid m
            obs.write_n_bits(mid_m, 32);

            if (channels == 2) {
                double side_mean = mean_abs(side_residuals, nFrames - warmup);
                double side_alpha = side_mean / (side_mean + 1.0);
                if (side_alpha < 0.001) side_alpha = 0.001;
                if (side_alpha > 0.999) side_alpha = 0.999;
                side_m = ceil(-1 / log(side_alpha));
                if (side_m < 1) side_m = 1;

                // write side m
                obs.write_n_bits(side_m, 32);
            }
        }

        GolombUtils golomb_mid(mid_m, method);
        GolombUtils golomb_side(side_m, method);

        // Encode warmup samples directly
        for (size_t i = 0; i < warmup; ++i) {
            golomb_mid.golomb_encode(&obs, mid[i]);
            if (channels == 2) {
                golomb_side.golomb_encode(&obs, side[i]);
            }
        }

        // Encode residuals
        for (size_t i = 0; i < nFrames - warmup; ++i) {
            golomb_mid.golomb_encode(&obs, mid_residuals[i]);

            if (channels == 2) {
                golomb_side.golomb_encode(&obs, side_residuals[i]);
            }

            // Debug: write residuals
            //if (debug_file.is_open()) {
            //    debug_file << block_num << "," << i << "," << residual_mid << "," << residual_side << "\n";
            //}
        }
        //block_num++;
    }

    obs.close();
    ofs.close();

    //if (debug_file.is_open()) {
    //    debug_file.close();
    //    cout << "Residuals written to residuals_debug.txt\n";
    //}

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Encoding finished in " << duration.count() << " ms\n";

    return 0;
}
