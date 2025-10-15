#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <fstream>
#include "bit_stream.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536;

int main(int argc, char *argv[]) {

    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input bin file> <output wav file>\n";
        return 1;
    }

    string input_bin = argv[1];
    string output_wav = argv[2];

    fstream ifs { input_bin, ios::in | ios::binary };
    if (not ifs.is_open()) {
        cerr << "Error opening input file: " << input_bin << endl;
        return 1;
    }

    BitStream ibs { ifs, STREAM_READ };

    // Prepare output WAV file
    SF_INFO sfinfo;
    int qbits = ibs.read_n_bits(8);
	int nchannels = ibs.read_n_bits(8);
	sfinfo.channels = nchannels;
    sfinfo.samplerate = ibs.read_n_bits(32);
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SndfileHandle sndFileOut(output_wav, SFM_WRITE, sfinfo.format, sfinfo.channels, sfinfo.samplerate);
    if (sndFileOut.error()) {
        cerr << "Error creating WAV file: " << output_wav << endl;
        return 1;
    }

    // Debug file for decoded samples
    //string debug_filename = output_wav + "_debug.txt";
    //fstream debug_file { debug_filename, ios::out };
    
    cout << "Decoding: " << input_bin << " -> " << output_wav << endl;
    cout << "Quantization bits: " << qbits << ", Channels: " << nchannels << ", Sample rate: " << sfinfo.samplerate << endl;

    vector<short> decoded_samples;
    decoded_samples.reserve(FRAMES_BUFFER_SIZE * nchannels);
	int shift_bits = 16 - qbits;

	int quantized_value;
	while ((quantized_value = ibs.read_n_bits(qbits)) != EOF) {
					
		unsigned short quantized_unsigned = ((unsigned short)quantized_value) << shift_bits;
		int signed_sample = (int)quantized_unsigned - 32768;
		
		decoded_samples.push_back((short)signed_sample);
		
		//debug_file << "Read: " << quantized_value << " -> " << quantized_unsigned << " -> " << signed_sample << "\n";
		//debug_file << signed_sample << "\n";

		if (decoded_samples.size() >= FRAMES_BUFFER_SIZE * nchannels) {
			sndFileOut.writef(decoded_samples.data(), decoded_samples.size() / nchannels);
			decoded_samples.clear();
		}
	}

    if (!decoded_samples.empty()) {
        sndFileOut.writef(decoded_samples.data(), decoded_samples.size() / nchannels);
    }
	
	//debug_file.close();
    ibs.close();
    return 0;
}
