
#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "wav_quant.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

int main(int argc, char *argv[]) {

	if(argc < 4) {
		cerr << "Usage: " << argv[0] << " <input file> <output file> <quant bits>\n";
		return 1;
	}

	SndfileHandle sndFile { argv[1] };
	if(sndFile.error()) {
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

	SndfileHandle sfhOut { argv[2], SFM_WRITE, sndFile.format(),
	  sndFile.channels(), sndFile.samplerate() };
	if(sfhOut.error()) {
		cerr << "Error: invalid output file\n";
		return 1;
    }

	int bin = stoi(argv[3]);
	if (bin <= 0 || bin > 16) {
		cerr << "Error: Number of quantization bits must be between 1 and 16\n";
	    return 1;
	}

	size_t nFrames;
	vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());
	WAVQuant quant {bin };
	while((nFrames = sndFile.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
		samples.resize(nFrames * sndFile.channels());
        quant.quantization(samples);
        
        const vector<short>& quantizedSamples = quant.getQuantizedSamples();
        sfhOut.writef(quantizedSamples.data(), nFrames);
    }

	return 0;
}
