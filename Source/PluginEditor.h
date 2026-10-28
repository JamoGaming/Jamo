#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class JamoAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit JamoAudioProcessorEditor(JamoAudioProcessor&);
    ~JamoAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    JamoAudioProcessor& processor;
    juce::Label title, subtitle, pitchLabel, statusLabel;
    juce::ComboBox keyBox, scaleBox, modeBox;
    juce::Slider retune, humanize, flex, formant, mix;
    juce::TextButton bypassButton { "BYPASS" };
    juce::OwnedArray<juce::Component> attachmentsKeepAlive;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment, scaleAttachment, modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttachment, humanizeAttachment, flexAttachment, formantAttachment, mixAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JamoAudioProcessorEditor)
};
