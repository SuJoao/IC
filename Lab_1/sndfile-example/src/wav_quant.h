#ifndef WAVQUANT_H
#define WAVQUANT_H

#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>
#include <cmath>
    
class WAVQuant {
  private:
	int binQuant;
	std::vector<short> quantizedSamples;
	int max_binQuant = 16;

  public:
	WAVQuant(int bin = 16) {
        binQuant = bin;
	}

	void quantization(const std::vector<short>& samples) {
		quantizedSamples.clear();
		
		int bits = max_binQuant - binQuant;
		for(size_t i = 0; i < samples.size(); i++) {
			int new_sample = ((int)samples[i] >> bits) << bits;
			quantizedSamples.push_back((short)new_sample);
		}
	}

	const std::vector<short>& getQuantizedSamples() const {
		return quantizedSamples;
	}
};

#endif
