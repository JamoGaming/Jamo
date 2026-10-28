#pragma once
#include <JuceHeader.h>
#include "PitchDetector.h"
#include "Scale.h"

class JamoAudioProcessor : public juce::AudioProcessor
{
public:
    JamoAudioProcessor();
    ~JamoAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Jamo"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    float getDetectedHz() const noexcept { return detectedHz.load(); }
    float getCorrectedMidi() const noexcept { return correctedMidi.load(); }

private:
    static APVTS::ParameterLayout createParameterLayout();
    PitchDetector detector;
    juce::AudioBuffer<float> analysisBuffer;
    int analysisWrite = 0;
    int analysisCount = 0;
    std::vector<float> analysisRing;
    juce::dsp::PitchShifter<float> pitchShifter;
    double currentSampleRate = 44100.0;
    std::atomic<float> detectedHz { 0.0f };
    std::atomic<float> correctedMidi { -1.0f };
    float smoothedRatio = 1.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JamoAudioProcessor)
};
