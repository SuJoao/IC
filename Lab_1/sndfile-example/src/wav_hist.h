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
#include <cmath>

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
		short bin = (int)log2(binFactor);
		for(size_t i = 0; i < samples.size(); i++) {
			short idx = samples[i] >> bin;
			idx =  idx << bin;
			idx += idx >= 0 ? idx + pow(2, bin) : idx - pow(2, bin);
			counts[n++ % sfh_channels][idx]++;

			if(i > 0 && i % 2 != 0 && (sfh_channels == 2)) {
				int left = (int)samples[i-1] >> bin;
				left =  left << bin;
				left += left >= 0 ? left + pow(2, bin) : left - pow(2, bin);

				int right = (int)samples[i] >> bin;
				right =  right << bin;
				right += right >= 0 ? right + pow(2, bin) : right - pow(2, bin);

				counts[2][(short)((left + right) / 2)]++;
				counts[3][(short)((left - right) / 2)]++;
			}
		}	
	}

	void dump(const size_t channel) const {
		for(auto [value, counter] : counts[channel])
			std::cout << value << '\t' << counter << '\n';
	}
};

#endif
