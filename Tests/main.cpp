#include "../Source/PitchDetector.h"
#include "../Source/Scale.h"
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    constexpr double sr = 48000.0;
    std::vector<float> signal(2048);
    for (size_t i = 0; i < signal.size(); ++i)
        signal[i] = 0.5f * std::sin(2.0 * juce::MathConstants<double>::pi * 220.0 * (double)i / sr);

    PitchDetector detector;
    detector.prepare(sr, (int) signal.size());
    const float hz = detector.detect(signal.data(), (int) signal.size());
    if (std::abs(hz - 220.0f) > 4.0f)
    {
        std::cerr << "Pitch detection failed: " << hz << " Hz\n";
        return 1;
    }
    const auto aMinor = JamoScale::maskFor(9, "Minor");
    const int target = JamoScale::nearestAllowedMidi(69, aMinor);
    if (target != 69) { std::cerr << "Scale mapping failed\n"; return 2; }
    std::cout << "Jamo tests passed; detected " << hz << " Hz\n";
    return 0;
}
