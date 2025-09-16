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
#ifndef WAVHIST_H
#define WAVHIST_H

#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>

class WAVHist {
  private:
	std::vector<std::map<short, size_t>> counts;
	size_t binFactor;
	size_t sfh_channels;
	
  public:
	WAVHist(const SndfileHandle& sfh, size_t bin = 1) {
		sfh_channels = sfh.channels();
		binFactor = bin;
		counts.resize(sfh_channels + 2);
	}

	void update(const std::vector<short>& samples) {
		size_t n { };
		short bin = binFactor - 1;
		for(size_t i = 0; i < samples.size(); i++) {
			short idx = samples[i] >> bin;
			counts[n++ % sfh_channels][idx]++;

			if(i > 0 && i % 2 != 0 && (sfh_channels == 2)) {
				short left = samples[i-1] >> bin;
				short right = samples[i] >> bin;

				counts[2][(left + right) / 2]++;
				counts[3][(left - right) / 2]++;
			}
		}	
	}

	void dump(const size_t channel) const {
		for(auto [value, counter] : counts[channel])
			std::cout << value << '\t' << counter << '\n';
	}
};

#endif
