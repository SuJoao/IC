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
#include <functional>

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
    std::vector<echo_params*> list;
    int list_size = 0;
    
    int channels = 0;
    int sampleRate = 44100;
    bool isSetup = false;
    long long int sampleIndex = 0;
    
    short duration = 0;
    
    float gain = 0.0f;
    
    long long int echoStartIndex = 0;
    bool isEchoing = false;
    
    long long int fadeInEndIndex = 0;
    long long int fadeOutStartIndex = 0;
    long long int endTrackIndex = 0;
    
    bool hold = false;
    bool finished = false;
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

    void audio_setup(int sampleRate, int channels, echo_params* customParams = nullptr) {
        customParams->channels = channels;
        customParams->delayBuffer.resize(sampleRate * channels); // 1 second buffer
        customParams->decayBuffer.resize(sampleRate * channels); // 1 second buffer
    }

    void echo_setup(float delay, float gain, int decay, echo_params* customParams = nullptr) {
        std::cout << "Echo setup parameters:" << std::endl;
        std::cout << "  delay: " << delay << std::endl;
        std::cout << "  gain: " << gain << std::endl;
        std::cout << "  decay: " << decay << std::endl;
        if (customParams) {
            std::cout << "  customParams->sampleRate: " << customParams->sampleRate << std::endl;
            std::cout << "  customParams->channels: " << customParams->channels << std::endl;
        }
        
        customParams->duration = delay;
        customParams->gain = gain;
        customParams->echoedSamples.resize(customParams->channels);
        customParams->echoStartIndex = delay * customParams->sampleRate * customParams->channels;
		size_t delaySamples = static_cast<size_t>(customParams->echoStartIndex);
        customParams->delayBuffer.resize(delaySamples * 1.25f);
        customParams->decayBuffer.resize(decay * customParams->sampleRate);
        customParams->isSetup = true;
        customParams->echoStartIndex = (delay) * (float)customParams->sampleRate * (float)customParams->channels;

        std::cout << "Echo setup complete:" << std::endl;
        std::cout << "  Delay: " << customParams->duration << " seconds" << std::endl;
        std::cout << "  Gain: " << customParams->gain << std::endl;
        std::cout << "  Decay: " << decay << " seconds" << std::endl;
        std::cout << "  Sample rate: " << customParams->sampleRate << " Hz" << std::endl;
        std::cout << "  Channels: " << customParams->channels << std::endl;
        std::cout << "  Echo start index: " << customParams->echoStartIndex << std::endl;
        std::cout << "  Delay buffer size: " << customParams->delayBuffer.size() << std::endl;
        std::cout << "  Decay buffer size: " << customParams->decayBuffer.size() << std::endl;
        std::cout << "  isEchoing: " << customParams->isEchoing << std::endl;
        std::cout << "------------------------" << std::endl;
    }

    void multi_echo_setup(std::vector<float> delays, std::vector<float> gains, int decay, echo_params* customParams = nullptr) {
        customParams->list_size = delays.size();
        customParams->list.resize(customParams->list_size);
        for (int i = 0; i < customParams->list_size; ++i) {
            customParams->list[i] = new echo_params();

            audio_setup(customParams->sampleRate, customParams->channels, customParams->list[i]);

            echo_setup(delays[i], gains[i], decay, customParams->list[i]);
        }
    }

    void fade_setup(short duration, echo_params* customParams = nullptr) {
        customParams->duration = duration;
    }

    std::vector<short> echo_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        circular_buffer decayBufferClone = customParams->decayBuffer.clone();
        circular_buffer delayBufferClone = customParams->delayBuffer.clone();

        for(const short& sample : samples) {
            short echoedSample = echo_sample(sample, customParams);
            output.push_back(echoedSample);
        }

        if (customParams->hold)
        {
            customParams->decayBuffer = decayBufferClone;
            customParams->delayBuffer = delayBufferClone;
        }
        
        if (customParams->finished && samples.empty())
        {
            return empty_buffer([this](short sample, echo_params* params) { return echo_sample(sample, params); }, customParams);
        }
        
        
        return output;
    }


    std::vector<short> empty_buffer(std::function<short(short, echo_params*)> sample_effect, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(customParams->delayBuffer.size());
        
        std::cout << "Flushing delay buffer..." << std::endl;
        while(!customParams->delayBuffer.empty()) {
            short affected_sample = sample_effect(0, customParams);
            output.push_back(affected_sample);
        }
        
        return output;
    }

    std::vector<short> delay_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        circular_buffer decayBufferClone = customParams->decayBuffer.clone();
        circular_buffer delayBufferClone = customParams->delayBuffer.clone();

        for(const short& sample : samples) {
            short echoedSample = delay_sample(sample, customParams);
            output.push_back(echoedSample);
        }

        if (customParams->hold)
        {
            customParams->decayBuffer = decayBufferClone;
            customParams->delayBuffer = delayBufferClone;
        }


        if (customParams->finished && samples.empty())
        {
            return empty_buffer([this](short sample, echo_params* params) { return delay_sample(sample, params); }, customParams);
        }
        
        
        
        return output;
    }

    std::vector<short> multi_echo_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        
        for(const short& sample : samples) {
            short echoedSample = multi_echo_sample(sample, customParams);
            output.push_back(echoedSample);
        }
        return output;
    }

    std::vector<short> gain_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());

        for(const short& sample : samples) {
            short echoedSample = gain_sample(sample, customParams->gain);
            output.push_back(echoedSample);
        }
        
        return output;
    }

    std::vector<short> fade_in_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        customParams->fadeInEndIndex = customParams->duration * 44100 * customParams->channels;

        for(const short& sample : samples) {
            short echoedSample = fadein_sample(sample, customParams);
            output.push_back(echoedSample);
        }
        
        return output;
    }

    std::vector<short> fade_out_buffer(const std::vector<short>& samples, echo_params* customParams = nullptr) {
        std::vector<short> output;
        output.reserve(samples.size());
        customParams->fadeOutStartIndex = customParams->duration * 44100 * customParams->channels;
        customParams->endTrackIndex = customParams->fadeOutStartIndex + (customParams->duration * 44100 * customParams->channels);

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

    short multi_echo_sample(const short sample, echo_params* customParams = nullptr) {
        short output = sample;
        long long acumulator = 0;
        
        for (size_t i = 0; i < customParams->list_size; i++)
        {   
            customParams->list[i]->gain = 1;
            acumulator += echo_sample(output, customParams->list[i]);
        }
        acumulator /= customParams->list_size;

        // float tetha = (float)customParams->list_size / customParams->gain;

        // float g = (float)customParams->list_size - tetha / (float)customParams->list_size;

        acumulator -= (int)((float)sample / (2 * customParams->gain));

        return static_cast<short>(acumulator);
    }

    short echo_sample(const short sample, echo_params* customParams = nullptr) {
        short output;

        // Add sample to delay buffer

        
        if(!customParams->finished) customParams->delayBuffer.put(sample);
        

        if (customParams ? customParams->decayBuffer.is_full() : params.decayBuffer.is_full())   customParams->decayBuffer.get();

        customParams->decayBuffer.put(sample);
        customParams->sampleIndex++;

        float echo_gain = customParams->gain > 0 ? customParams->gain : 0;

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
            if (customParams->sampleIndex >= customParams->echoStartIndex)
            {
                customParams->isEchoing = true;
                std::cout << "echoing: on" << std::endl;
                std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << std::endl;
                std::cout << "echo start: " << customParams->echoStartIndex << std::endl;
                std::cout << "------------------------" << std::endl;
            }
            output = sample;
        }
        return output;
	}

    short delay_sample(const short sample, echo_params* customParams = nullptr) {
        short output;

        // Add sample to delay buffer

        
        if(!customParams->finished)customParams->delayBuffer.put(sample);
        
        customParams->sampleIndex++;

        // Apply echo effect
        if (customParams->isEchoing)
        {
            short delayedSample = customParams->delayBuffer.get();
            short newSample = static_cast<short>(delayedSample);
            output = newSample;
            std::cout << "Sample Index: " << (customParams->sampleIndex) << " | Sample: " << sample << " | Delayed Sample: " << delayedSample << " | New Sample: " << newSample << std::endl;

        }else {
            if (customParams->sampleIndex >= customParams->echoStartIndex)
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
