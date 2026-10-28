#pragma once
#include <JuceHeader.h>

class PitchDetector
{
public:
    void prepare(double sampleRate, int windowSize = 2048);
    void reset();
    // Returns fundamental frequency in Hz, or 0 when confidence is too low.
    float detect(const float* samples, int numSamples) const;

private:
    double sr = 44100.0;
    int window = 2048;
    float minHz = 65.0f;
    float maxHz = 1000.0f;
};
