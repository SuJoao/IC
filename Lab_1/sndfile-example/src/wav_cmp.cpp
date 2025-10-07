#include <iostream>
#include <vector>
#include <cmath>
#include <sndfile.hh>

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <first sample> <second sample>\n";
        return 1;
    }

    SndfileHandle sndFile1{ argv[1] };
    SndfileHandle sndFile2{ argv[2] };

    if (sndFile1.error() || sndFile2.error()) {
        cerr << "Error: invalid input file(s)\n";
        return 1;
    }

    if ((sndFile1.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV ||
        (sndFile2.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: files must be WAV format\n";
        return 1;
    }

    if ((sndFile1.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 ||
        (sndFile2.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: files must be 16-bit PCM\n";
        return 1;
    }

    if (sndFile1.channels() != sndFile2.channels()) {
        cerr << "Error: files must have the same number of channels\n";
        return 1;
    }

    int numChannels = sndFile1.channels();
    size_t numFrames = static_cast<size_t>(min(sndFile1.frames(), sndFile2.frames()));

    vector<short> buffer1(FRAMES_BUFFER_SIZE * numChannels);
    vector<short> buffer2(FRAMES_BUFFER_SIZE * numChannels);

    vector<double> rmse(numChannels, 0.0);
    vector<double> maxErr(numChannels, 0.0);
    vector<double> snr(numChannels, 0.0);

    vector<double> errEnergy(numChannels, 0.0);
    vector<double> sigEnergy(numChannels, 0.0);

    // buffer = [L0, R0, L1, R1, L2, R2]
    // frame = [L0, R0], [L1, R1], [L2, R2]
    // sample = L0, R0, L1, R1, L2, R2

    while (true) {
        sf_count_t framesRead1 = sndFile1.readf(buffer1.data(), FRAMES_BUFFER_SIZE);
        sf_count_t framesRead2 = sndFile2.readf(buffer2.data(), FRAMES_BUFFER_SIZE);

        if (framesRead1 == 0 || framesRead2 == 0)
            break;

        size_t framesToProcess = static_cast<size_t>(min(framesRead1, framesRead2));

        for (size_t i = 0; i < framesToProcess; ++i) {
            for (int ch = 0; ch < numChannels; ++ch) {
                size_t idx = i * numChannels + ch;
                double s1 = buffer1[idx];
                double s2 = buffer2[idx];
                double diff = s1 - s2;

                errEnergy[ch] += diff * diff;
                sigEnergy[ch] += s1 * s1;
                maxErr[ch] = max(maxErr[ch], fabs(diff));
            }
        }
    }
    

    // RMSE and SNR
    for (int ch = 0; ch < numChannels; ++ch) {
        rmse[ch] = sqrt(errEnergy[ch] / numFrames);
        snr[ch] = (errEnergy[ch] == 0.0) ? INFINITY : 10 * log10(sigEnergy[ch] / errEnergy[ch]);
    }

    // Averages
    double avgRMSE = 0.0, avgMaxErr = 0.0, avgSNR = 0.0;
    for (int ch = 0; ch < numChannels; ++ch) {
        avgRMSE += rmse[ch];
        avgMaxErr += maxErr[ch];
        avgSNR += snr[ch];
    }
    avgRMSE /= numChannels;
    avgMaxErr /= numChannels;
    avgSNR /= numChannels;


    for (int ch = 0; ch < numChannels; ++ch) {
        cout << "Channel " << ch + 1 << " RMSE: " << rmse[ch] << endl;
        cout << "Channel " << ch + 1 << " Max error: " << maxErr[ch] << endl;
        cout << "Channel " << ch + 1 << " SNR: " << snr[ch] << " dB" << endl;
    }

    cout << "Average RMSE: " << avgRMSE << endl;
    cout << "Average Max error: " << avgMaxErr << endl;
    cout << "Average SNR: " << avgSNR << " dB" << endl;

    return 0;
}
