//------------------------------------------------------------------------------
//
// Copyright 2025 University of Aveiro, Portugal, All Rights Reserved.
//
// These programs are supplied free of charge for research purposes only,
// and may not be sold or incorporated into any commercial product. There is
// ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
// fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
// happen to find a bug, or have modifications to suggest, please report
// the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
// and this statement of conditions must remain an integral part of each
// and every copy made of these files.
//
// Armando J. Pinho (ap@ua.pt)
// IEETA / DETI / University of Aveiro
//
#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "wav_fx.h"

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

int main(int argc, char *argv[]) {

	if(argc < 4) {
		cerr << "Usage: " << argv[0] << " <input file> <output file> <delay>\n";
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

    float delay = stof(argv[3]);

	SndfileHandle sfhOut { argv[2], SFM_WRITE, sndFile.format(),
	  sndFile.channels(), sndFile.samplerate() };
	if(sfhOut.error()) {
		cerr << "Error: invalid output file\n";
		return 1;
    }

	size_t nFrames;
	vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());
	WAVfx fx { sndFile };
	long long totalSamples = sndFile.frames() * sndFile.channels();
	cout << "Total samples in file: " << totalSamples << endl;
	
    echo_params echo1 = fx.echo_setup(delay, 0.7f, 3, sndFile.samplerate(), sndFile.channels());
    echo_params echo2 = fx.echo_setup(delay/2, 0.7f, 3, sndFile.samplerate(), sndFile.channels());
    echo_params echo3 = fx.echo_setup(delay/3, 0.7f, 3, sndFile.samplerate(), sndFile.channels());
	while((nFrames = sndFile.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
		samples.resize(nFrames * sndFile.channels());
        
        
        vector<short> base;
        base = fx.echo_buffer(samples, 0.5f, &echo1);
        base = fx.echo_buffer(samples, 0.5f, &echo2);
        base = fx.echo_buffer(samples, 0.5f, &echo3);


        // fx.echo_setup(delay+1, 0.5f, 1000, sndFile.samplerate(), sndFile.channels());
        // base = fx.echo_buffer(base, 0.2f, &echo1, false);
        // base = fx.echo_buffer(base, 0.2f, &echo1, false);
        // base = fx.echo_buffer(base, 0.2f, &echo1, false);
        // fx.echo_setup(delay+2, 0.5f, 1000, sndFile.samplerate(), sndFile.channels());
        // base = fx.echo_buffer(base, 0.5f);
        // fx.echo_setup(delay+3, 0.5f, 1000, sndFile.samplerate(), sndFile.channels());
        // base = fx.echo_buffer(base, 0.5f);
        // base = fx.echo_buffer(base, 0.5f);

        sfhOut.writef(base.data(), nFrames);
	}

	return 0;
}

