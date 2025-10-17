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

void help_instructions() {
	cout << "Usage: wav_fx <input file> <output file> <filter> <params ...>\n";
	cout << "filters:\n";
	cout << "\techo <delay in seconds> <gain>\n ";
	cout << "\tmulti_echo <delay1> <delay2> <delay3> ...\n";
	cout << "\tfade_in <duration in seconds>\n";
	cout << "\tfade_out <duration in seconds>\n";
}

int main(int argc, char *argv[]) {

	if(argc < 5) {
		help_instructions();
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

	string filterType = argv[3];

	float duration = stof(argv[4]);

	size_t nFrames;
	vector<short> samples(FRAMES_BUFFER_SIZE * sndFile.channels());
	WAVfx fx { sndFile };
	long long totalSamples = sndFile.frames() * sndFile.channels();
	cout << "Total samples in file: " << totalSamples << endl;
	cout << "filterType: " << filterType << endl;

	echo_params *echo1 = new echo_params;
	echo_params *echo2 = new echo_params;
	echo_params *echo3 = new echo_params;

	fx.audio_setup(sndFile.samplerate(), sndFile.channels(), echo1);

	echo2 = echo1->clone();
	echo3 = echo1->clone();

	std::function<std::vector<short>(const std::vector<short>&, echo_params*)> process_function;

    if (filterType == "echo") {
        process_function = [&fx](const std::vector<short>& samples, echo_params* params) { return fx.echo_buffer(samples, params); };
		if (argc > 5)
		{
			float gain = stof(argv[5]);
			fx.echo_setup(duration, gain, 3, echo1);
		}else {
			help_instructions();
			return 1;
		}
		
	}else if (filterType == "delay") {
		process_function = [&fx](const std::vector<short>& samples, echo_params* params) { return fx.delay_buffer(samples, params); };
		fx.echo_setup(duration, 2.0f, 3, echo1);
	} else if (filterType == "multiEcho") {
        process_function = [&fx](const std::vector<short>& samples, echo_params* params) { return fx.multi_echo_buffer(samples, params); };
		std::cout << "argc: " << argc << std::endl;
		if (argc > 5)
		{
			std::vector<float> delays;
			std::vector<float> gains;

			// Number of delay values is total args minus: program, in, out, filter, and final gain
			int numDelays = argc - 5; // e.g. for argv: prog in out multiEcho d1 d2 d3 gain => numDelays=3
			if (numDelays <= 0) {
				help_instructions();
				return 1;
			}

			// Parse delays from argv[4] .. argv[4 + numDelays - 1]
			for (int i = 0; i < numDelays; ++i) {
				const char* arg = argv[4 + i];
				if (!arg) continue;
				std::cout << "argv[" << (4 + i) << "]: " << arg << std::endl;
				delays.push_back(static_cast<float>(atof(arg)));
			}

			// Last argument is the common gain for multi-echo
			const char* gainArg = argv[argc - 1];
			if (!gainArg) {
				help_instructions();
				return 1;
			}
			float commonGain = stof(gainArg);
			// Fill gains vector with the same gain for each delay
			gains.assign(delays.size(), commonGain);

			fx.multi_echo_setup(delays, gains, 3, echo1);
		}else {
			help_instructions();
			return 1;
		}
    } else if (filterType == "fade_in") {
        process_function = [&fx](const std::vector<short>& samples, echo_params* params) { return fx.fade_in_buffer(samples, params); };
    } else if (filterType == "fade_out") {
        process_function = [&fx](const std::vector<short>& samples, echo_params* params) { return fx.fade_out_buffer(samples, params); };
    }



	vector<short> base;
	while((nFrames = sndFile.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
		samples.resize(nFrames * sndFile.channels());
        
        
        base = process_function(samples, echo1);
    

        sfhOut.writef(base.data(), nFrames);
	}	
	echo1->finished = true;
	base = process_function({}, echo1);
	sfhOut.writef(base.data(), echo1->delayBuffer.size() / sndFile.channels());
	

	return 0;
}

