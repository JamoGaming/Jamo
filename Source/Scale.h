#pragma once
#include <JuceHeader.h>

namespace JamoScale
{
    static constexpr const char* noteNames[12] =
        { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    juce::StringArray scaleNames();
    std::array<bool, 12> maskFor(int root, const juce::String& scale);
    int nearestAllowedMidi(int midiNote, const std::array<bool, 12>& allowed);
    int frequencyToMidi(float hz);
    float midiToFrequency(int midiNote);
}
