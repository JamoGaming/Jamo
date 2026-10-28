#include "Scale.h"
#include <cmath>

juce::StringArray JamoScale::scaleNames()
{
    return { "Chromatic", "Major", "Minor", "Pentatonic", "Blues", "Dorian", "Mixolydian" };
}

std::array<bool, 12> JamoScale::maskFor(int root, const juce::String& scale)
{
    static const int major[] = {0,2,4,5,7,9,11};
    static const int minor[] = {0,2,3,5,7,8,10};
    static const int penta[] = {0,2,4,7,9};
    static const int blues[] = {0,3,5,6,7,10};
    static const int dorian[] = {0,2,3,5,7,9,10};
    static const int mixo[] = {0,2,4,5,7,9,10};
    std::array<bool, 12> result{};
    result.fill(false);
    if (scale == "Chromatic") { result.fill(true); return result; }
    const int* intervals = major;
    int count = 7;
    if (scale == "Minor") intervals = minor;
    else if (scale == "Pentatonic") { intervals = penta; count = 5; }
    else if (scale == "Blues") { intervals = blues; count = 6; }
    else if (scale == "Dorian") intervals = dorian;
    else if (scale == "Mixolydian") intervals = mixo;
    for (int i = 0; i < count; ++i) result[(root + intervals[i]) % 12] = true;
    return result;
}

int JamoScale::nearestAllowedMidi(int midiNote, const std::array<bool, 12>& allowed)
{
    int best = midiNote;
    int bestDistance = 1000;
    for (int candidate = midiNote - 6; candidate <= midiNote + 6; ++candidate)
    {
        if (allowed[(candidate % 12 + 12) % 12])
        {
            const int distance = std::abs(candidate - midiNote);
            if (distance < bestDistance) { best = candidate; bestDistance = distance; }
        }
    }
    return best;
}

int JamoScale::frequencyToMidi(float hz)
{
    if (hz <= 0.0f) return -1;
    return (int) std::lround(69.0 + 12.0 * std::log2(hz / 440.0));
}

float JamoScale::midiToFrequency(int midiNote)
{
    return (float) (440.0 * std::pow(2.0, (midiNote - 69) / 12.0));
}
