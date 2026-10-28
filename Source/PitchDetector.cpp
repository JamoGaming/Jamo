#include "PitchDetector.h"
#include <cmath>
#include <vector>

void PitchDetector::prepare(double sampleRate, int windowSize)
{
    sr = sampleRate;
    window = juce::jmax(256, windowSize);
}

void PitchDetector::reset() {}

float PitchDetector::detect(const float* samples, int numSamples) const
{
    if (samples == nullptr || numSamples < 256)
        return 0.0f;

    const int n = juce::jmin(numSamples, window);
    const int minLag = juce::jmax(2, (int) std::floor(sr / maxHz));
    const int maxLag = juce::jmin(n / 2, (int) std::ceil(sr / minHz));
    if (maxLag <= minLag + 2)
        return 0.0f;

    // Remove DC and apply a Hann window. This YIN-inspired difference function
    // is intentionally allocation-free in the audio thread.
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += samples[i];
    mean /= (double) n;

    float bestScore = 1.0e30f;
    int bestLag = -1;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double diff = 0.0;
        for (int i = 0; i < n - lag; ++i)
        {
            const double a = (samples[i] - mean);
            const double b = (samples[i + lag] - mean);
            const double w = 0.5 - 0.5 * std::cos(juce::MathConstants<double>::twoPi * i / (n - 1));
            const double d = (a - b) * w;
            diff += d * d;
        }
        const float score = (float) (diff / (n - lag));
        if (score < bestScore)
        {
            bestScore = score;
            bestLag = lag;
        }
    }

    if (bestLag < 0 || bestScore > 0.025f)
        return 0.0f;

    // Parabolic refinement around the best lag.
    auto differenceAt = [&](int lag)
    {
        double d = 0.0;
        for (int i = 0; i < n - lag; ++i)
        {
            const double a = samples[i] - mean;
            const double b = samples[i + lag] - mean;
            const double delta = a - b;
            d += delta * delta;
        }
        return d / (n - lag);
    };

    const double left = bestLag > minLag ? differenceAt(bestLag - 1) : bestScore;
    const double center = differenceAt(bestLag);
    const double right = bestLag < maxLag ? differenceAt(bestLag + 1) : bestScore;
    const double denom = left - 2.0 * center + right;
    const double shift = std::abs(denom) > 1.0e-12 ? 0.5 * (left - right) / denom : 0.0;
    const double refinedLag = (double) bestLag + juce::jlimit(-0.5, 0.5, shift);
    const float hz = (float) (sr / refinedLag);
    return (hz >= minHz && hz <= maxHz) ? hz : 0.0f;
}
