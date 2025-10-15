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
#ifndef WAV_FX
#define WAV_FX

#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>
#include <cmath>
#include <vector>

class circular_buffer
{
private:
    std::vector<short> buffer;
    size_t head;
    size_t tail;
    size_t maxSize;
    size_t read_only_iterator;
    bool full;
public:
    circular_buffer(size_t size) : buffer(size), maxSize(size), head(0), tail(0), read_only_iterator(0), full(false) {}
    void put(short item) {
        buffer[head] = item;
        if(full) {
            tail = (tail + 1) % maxSize;
        }
        head = (head + 1) % maxSize;
        read_only_iterator = head;
        full = head == tail;
    }
    short get() {
        if(empty()) throw std::runtime_error("Buffer is empty");
        short item = buffer[tail];
        full = false;
        tail = (tail + 1) % maxSize;
        return item;
    }
    bool empty() const {
        return !full && (head == tail);
    }
    void resize(size_t size) {
        buffer.resize(size);
        maxSize = size;
        head = 0;
        read_only_iterator = head;
        tail = 0;
        full = false;
    }
    size_t size() const {
        return maxSize;
    }
    bool is_full() const {
        return full;
    }
    short read_head() const {
        return buffer[head];
    }
    short read_tail() const {
        return buffer[tail];
    }
    circular_buffer clone() const {
        circular_buffer cloned(maxSize);
        std::copy(buffer.begin(), buffer.end(), cloned.buffer.begin());
        cloned.head = head;
        cloned.tail = tail;
        cloned.read_only_iterator = read_only_iterator;
        cloned.full = full;

        return cloned;
    }
    
    ~circular_buffer();
};


circular_buffer::~circular_buffer()
{
}

struct echo_params
{
    std::vector<short> echoedSamples;
    circular_buffer delayBuffer;
    circular_buffer decayBuffer;
    long long int sampleIndex = 0;
    long long int echoStartIndex = 0;
    long long int fadeInEndIndex = 0;
    long long int fadeOutStartIndex = 0;
    long long int endTrackIndex = 0;
    int channels = 0;
    bool isSetup = false;
    bool isEchoing = false;
    echo_params() : delayBuffer(1), decayBuffer(1) {}
    echo_params* clone() {
        echo_params* cloned = new echo_params();
        cloned->echoedSamples = echoedSamples;
        cloned->delayBuffer = delayBuffer.clone();
        cloned->decayBuffer = decayBuffer.clone();
        cloned->sampleIndex = sampleIndex;
        cloned->echoStartIndex = echoStartIndex;
        cloned->channels = channels;
        cloned->isSetup = isSetup;
        cloned->isEchoing = isEchoing;
        return cloned;
    }
};

class WAVfx {
  private:
	size_t sfh_channels;
	
  public:
	WAVfx(const SndfileHandle& sfh) {
		sfh_channels = sfh.channels();
	}


    echo_params params;

    echo_params echo_setup(float delay, float gain, int decay, int sampleRate, int channels) {
        
        
        // make output buffer
        params.echoedSamples.resize(channels);
        params.channels = channels;
        params.echoStartIndex = delay * sampleRate * channels;
		size_t delaySamples = static_cast<size_t>(params.echoStartIndex);
        params.delayBuffer.resize(delaySamples * 1.25f);
        params.decayBuffer.resize(decay * sampleRate);
        params.isSetup = true;
        params.echoStartIndex = (delay ) * sampleRate * channels;

        echo_params output_params;

        output_params.echoedSamples.resize(channels);
        output_params.channels = channels;
        output_params.echoStartIndex = delay * sampleRate * channels;
		delaySamples = static_cast<size_t>(output_params.echoStartIndex);
        output_params.delayBuffer.resize(delaySamples * 1.25f);
        output_params.decayBuffer.resize(decay * sampleRate);
        output_params.isSetup = true;
        output_params.echoStartIndex = (delay ) * sampleRate * channels;

        std::cout << "Echo setup complete:" << std::endl;
        std::cout << "  Delay: " << delay << " seconds" << std::endl;
        std::cout << "  Gain: " << gain << std::endl;
        std::cout << "  Decay: " << decay << " seconds" << std::endl;
        std::cout << "  Sample rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "  Channels: " << channels << std::endl;
        std::cout << "  Echo start index: " << params.echoStartIndex << std::endl;
        std::cout << "  Delay buffer size: " << params.delayBuffer.size() << std::endl;
        std::cout << "  Decay buffer size: " << params.decayBuffer.size() << std::endl;

        return output_params;
    }

    std::vector<short> echo_buffer(const std::vector<short>& samples, float gain, echo_params* customParams = nullptr, bool next=true) {
        std::vector<short> output;
        output.reserve(samples.size());
        circular_buffer decayBufferClone = customParams->decayBuffer.clone();
        circular_buffer delayBufferClone = customParams->delayBuffer.clone();

        for(const short& sample : samples) {
            short echoedSample = echo_sample(sample, gain, customParams);
            output.push_back(echoedSample);
        }

        if (!next)
        {
            customParams->decayBuffer = decayBufferClone;
            customParams->delayBuffer = delayBufferClone;
        }
        
        
        
        return output;
    }

    std::vector<short> delay_buffer(const std::vector<short>& samples, float gain, echo_params* customParams = nullptr, bool next=true) {
        std::vector<short> output;
        output.reserve(samples.size());
        circular_buffer decayBufferClone = customParams->decayBuffer.clone();
        circular_buffer delayBufferClone = customParams->delayBuffer.clone();

        for(const short& sample : samples) {
            short echoedSample = delay_sample(sample, customParams);
            output.push_back(echoedSample);
        }

        if (!next)
        {
            customParams->decayBuffer = decayBufferClone;
            customParams->delayBuffer = delayBufferClone;
        }
        
        
        
        return output;
    }

    std::vector<short> multi_echo_buffer(const std::vector<short>& samples, float gain, echo_params* customParams = nullptr, bool next=true) {
        std::vector<short> output;
        output.reserve(samples.size());
        
        for(const short& sample : samples) {
            short echoedSample = echo_sample(sample, gain, customParams);
            output.push_back(echoedSample);
        }

        std::vector<echo_params> echo_buffers[3];
        for (size_t i = 0; i < 3; i++)
        {   
            echo_params* newParams = echo_setup(customParams->echoStartIndex / (i+1) / customParams->channels, gain, 3, 44100, customParams->channels).clone();
            echo_buffers[i].push_back(*newParams);
            delete newParams;
        }
        
        
        
        return output;
    }

    std::vector<short> gain_buffer(const std::vector<short>& samples, float gain) {
        std::vector<short> output;
        output.reserve(samples.size());

        for(const short& sample : samples) {
            short echoedSample = gain_sample(sample, gain);
            output.push_back(echoedSample);
        }
        
        return output;
    }

    std::vector<short> fadein_buffer(const std::vector<short>& samples, float tiker, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        customParams->fadeInEndIndex = tiker * 44100 * customParams->channels;

        for(const short& sample : samples) {
            short echoedSample = fadein_sample(sample, customParams);
            output.push_back(echoedSample);
        }
        
        return output;
    }

    std::vector<short> fadeout_buffer(const std::vector<short>& samples, float tiker, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        customParams->fadeOutStartIndex = tiker * 44100 * customParams->channels;
        customParams->endTrackIndex = customParams->fadeOutStartIndex + (tiker * 44100 * customParams->channels);

        for(const short& sample : samples) {
            short echoedSample = fadeout_sample(sample, customParams);
            output.push_back(echoedSample);
        }
        
        return output;
    }

    std::vector<short> echo_multi_channel(const std::vector<short>& samples, float gain) {
        std::vector<short> output(params.channels);

        // Add samples to delay buffer
        for(size_t i = 0; i < samples.size(); i++) {
            params.delayBuffer.put(samples[i]);
            if (params.decayBuffer.is_full())   params.decayBuffer.get();
            params.decayBuffer.put(samples[i]);
            params.sampleIndex+=params.channels;
        }
        
        float echo_gain = gain > 0 ? gain : 0;

        float alpha = echo_gain / (1 + echo_gain);
        float alpha_complement = alpha < 1 ? 1 - alpha : 1;

        // Apply echo effect
        if (params.isEchoing)
        {
            for(size_t i = 0; i < samples.size(); i++) {
                short delayedSample = params.delayBuffer.get();
                short decayedSample = params.decayBuffer.empty() ? 0 : params.decayBuffer.get();
                short newSample = static_cast<short>(alpha_complement * samples[i] + alpha * delayedSample);
                output.push_back(newSample);
                
            }
        }else {
            if (params.sampleIndex >= params.echoStartIndex)
            {
                params.isEchoing = true;
            }
        }
        return output;
	}


    short echo_sample(const short sample, float gain, echo_params* customParams = nullptr) {
        short output;

        // Add sample to delay buffer

        
        customParams->delayBuffer.put(sample);
        

        if (customParams ? customParams->decayBuffer.is_full() : params.decayBuffer.is_full())   customParams->decayBuffer.get();

        customParams->decayBuffer.put(sample);
        customParams->sampleIndex++;
        
        float echo_gain = gain > 0 ? gain : 0;

        float alpha = echo_gain / (1 + echo_gain);
        float alpha_complement = alpha < 1 ? 1 - alpha : 1;

        // Apply echo effect
        if (customParams->isEchoing)
        {
            short delayedSample = customParams->delayBuffer.get();
            short decayedSample = customParams->decayBuffer.empty() ? 0 : customParams->decayBuffer.get();
            short newSample = static_cast<short>(alpha_complement * sample + alpha * delayedSample);
            output = newSample;
            std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << " | Delayed Sample: " << delayedSample << " | New Sample: " << newSample << std::endl;

        }else {
            if (customParams->sampleIndex >= params.echoStartIndex)
            {
                customParams->isEchoing = true;
            }
            output = sample;
        }
        return output;
	}

    short delay_sample(const short sample, echo_params* customParams = nullptr) {
        short output;

        // Add sample to delay buffer

        
        customParams->delayBuffer.put(sample);
        
        customParams->sampleIndex++;

        // Apply echo effect
        if (customParams->isEchoing)
        {
            short delayedSample = customParams->delayBuffer.get();
            short newSample = static_cast<short>(delayedSample);
            output = newSample;
            std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << " | Delayed Sample: " << delayedSample << " | New Sample: " << newSample << std::endl;

        }else {
            if (customParams->sampleIndex >= params.echoStartIndex)
            {
                customParams->isEchoing = true;
            }
            output = 0;
        }
        return output;
	}

    short gain_sample(const short sample, float gain) {

        std::cout << " | Sample: " << sample << std::endl;
        
        return static_cast<short>( sample * gain);
	}

    short fadein_sample(const short sample, echo_params* customParams = nullptr) {
        short output;
        if (customParams->sampleIndex < customParams->fadeInEndIndex)
        {
            float fadeFactor = (static_cast<float>(customParams->sampleIndex)) / (static_cast<float>(customParams->fadeInEndIndex));
            output = static_cast<short>(sample * fadeFactor);
            customParams->sampleIndex++;
        } else {
            output = sample;
        }
        
        std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << " | New Sample: " << output << std::endl;
        
        return output;
    }
    short fadeout_sample(const short sample, echo_params* customParams = nullptr) {
        short output;
        if (customParams->sampleIndex >= customParams->fadeOutStartIndex)
        {
            float fadeFactor = 1.0f - (static_cast<float>(customParams->sampleIndex - customParams->fadeOutStartIndex) / static_cast<float>(customParams->endTrackIndex - customParams->fadeOutStartIndex));
            fadeFactor = fadeFactor < 0 ? 0 : fadeFactor;
            output = static_cast<short>(sample * fadeFactor);
            customParams->sampleIndex++;
        } else {
            output = sample;
            customParams->sampleIndex++;
        }
        
        std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << " | New Sample: " << output << std::endl;
        
        return output;
    }
};

#endif
