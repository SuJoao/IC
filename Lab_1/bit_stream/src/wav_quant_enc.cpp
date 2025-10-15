#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <fstream>
#include "bit_stream.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536;

int main(int argc, char *argv[]) {

    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input wav file> <output bin file> <quant bits>\n";
        return 1;
    }

    SndfileHandle sndFile{ argv[1] };
    if (sndFile.error()) {
        cerr << "Error: invalid input file\n";
        return 1;
    }

	if((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format\n";
		return 1;
	}

	if((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format\n";
		return 1;
	}

    fstream ofs { argv[2], ios::out | ios::binary };
	if(not ofs.is_open()) {
		cerr << "Error opening bin file " << argv[argc-1] << endl;
		return 1;
	}

    int qbits = stoi(argv[3]);
    if (qbits <= 0 || qbits > 16) {
        cerr << "Error: quantization bits must be between 1 and 16\n";
        return 1;
    }

    // Debug file
    /*
    string debug_filename = string(argv[2]) + "_debug.txt";
    fstream debug_file { debug_filename, ios::out };
    if(not debug_file.is_open()) {
        cerr << "Error opening debug file " << debug_filename << endl;
        return 1;
    }
    */

    int shift_bits = 16 - qbits;
	BitStream obs { ofs, STREAM_WRITE };
    size_t nFrames;
	vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());

    obs.write_n_bits(qbits, 8);
    obs.write_n_bits(sndFile.channels(), 8);
    obs.write_n_bits(sndFile.samplerate(), 32);

    while((nFrames = sndFile.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
		samples.resize(nFrames * sndFile.channels());
    
		for(size_t i = 0; i < samples.size(); i++) {
            unsigned short unsigned_sample = (unsigned short)((int)samples[i] + 32768);
            unsigned short quantized_unsigned = unsigned_sample >> shift_bits;
            
            //short test = (quantized_unsigned << shift_bits) - 32768;
            //debug_file << test << "\n";
            
            obs.write_n_bits(quantized_unsigned, qbits);
        }
    }

    //debug_file.close();
	obs.close();
	return 0;
}
