#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    SndfileHandle sndOrig { argv[1] };
    if (sndOrig.error()) {
        cerr << "Error: invalid input file\n";
        return 1;
    }

    SndfileHandle sndProc { argv[2] };
    if (sndProc.error()) {
        cerr << "Error: invalid output file\n";
        return 1;
    }

    if (sndOrig.channels() != sndProc.channels() || sndOrig.frames() != sndProc.frames()) {
        cerr << "Error: files must have the same number of channels and frames\n";
        return 1;
    }

    size_t channels = sndOrig.channels();
    size_t frames = sndOrig.frames();

    vector<long long> MSE(channels, 0);       // Mean Squared Error Vector
    vector<short> maxError(channels, 0);       // Max Absolute Error Vector
    vector<long long> signalEnergy(channels, 0); // Original Signal Energy Vector

    vector<short> originalSamples(frames * channels);
    vector<short> processedSamples(frames * channels);

    sndOrig.readf(originalSamples.data(), frames);
    sndProc.readf(processedSamples.data(), frames);

    for (size_t i = 0; i < frames; i++) {
        long long sumChannels = 0;
        for (size_t channel = 0; channel < channels; channel++) {
            size_t idx = (i * channels) + channel;
            short diff = originalSamples[idx] - processedSamples[idx];
            MSE[channel] += diff * diff;
            if (abs(diff) > maxError[channel]) maxError[channel] = abs(diff);
            signalEnergy[channel] += originalSamples[idx] * originalSamples[idx];
            sumChannels += diff * diff;
        }
    }

    for (size_t channel = 0; channel < channels; channel++) {
        double mseValue = (static_cast<double>(MSE[channel]) / frames);
        double snr = (10.0 * log10(static_cast<double>(signalEnergy[channel]) / MSE[channel]));
        cout << "Channel " << (channel + 1) << ":\n";
        cout << "MSE: " << mseValue << "\n";
        cout << "Max Error: " << maxError[channel] << "\n";
        cout << "SNR: " << snr << "\n";
    }

    long long mseMean = 0;
    long long maxErrorMean = 0;
    long long signalEnergyMean = 0;
    for (size_t channel = 0; channel < channels; channel++) {
        mseMean += MSE[channel];
        signalEnergyMean += signalEnergy[channel];
        if (maxError[channel] > maxErrorMean) maxErrorMean = maxError[channel];
    }

    double mseValMean = mseMean / double(frames * channels);
    double snrMean = 10.0 * log10(double(signalEnergyMean) / mseMean);

    cout << "Average of All Channels:\n";
    cout << "MSE: " << mseValMean << "\n";
    cout << "Max Error: " << maxErrMean << "\n";
    cout << "SNR: " << snrMean << "\n";

    return 0;
}
