#include "PluginProcessor.h"
#include "PluginEditor.h"

JamoAudioProcessor::APVTS::ParameterLayout JamoAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto choice = [](const juce::String& id, const juce::String& name, juce::StringArray choices, int def)
    {
        return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { id, 1 }, name, choices, def);
    };
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "retune", 1 }, "Retune Speed", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 35.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "humanize", 1 }, "Humanize", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "flex", 1 }, "Flex Tune", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 15.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "formant", 1 }, "Formant Shift", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "mix", 1 }, "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "input", 1 }, "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "output", 1 }, "Output Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "bypass", 1 }, "Bypass", false));
    p.push_back(choice("root", "Key", { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 0));
    p.push_back(choice("scale", "Scale", JamoScale::scaleNames(), 2));
    p.push_back(choice("mode", "Mode", { "Natural","Hard Tune","Robot" }, 0));
    p.push_back(choice("midimode", "MIDI Mode", { "Scale","MIDI Notes" }, 0));
    return { p.begin(), p.end() };
}

JamoAudioProcessor::JamoAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "JAMO_STATE", createParameterLayout())
{
}

void JamoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    detector.prepare(sampleRate, 2048);
    analysisRing.assign(4096, 0.0f);
    analysisWrite = analysisCount = 0;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax(1, samplesPerBlock), (juce::uint32) getTotalNumOutputChannels() };
    pitchShifter.prepare(spec);
    pitchShifter.reset();
    smoothedRatio = 1.0f;
}

void JamoAudioProcessor::releaseResources() { pitchShifter.reset(); }

bool JamoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void JamoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    auto* bypass = parameters.getRawParameterValue("bypass");
    if (*bypass > 0.5f) return;

    const float inputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("input")->load());
    const float outputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    const float mix = parameters.getRawParameterValue("mix")->load() * 0.01f;
    const int root = (int) parameters.getRawParameterValue("root")->load();
    const int scaleIndex = (int) parameters.getRawParameterValue("scale")->load();
    const int mode = (int) parameters.getRawParameterValue("mode")->load();
    const int midiMode = (int) parameters.getRawParameterValue("midimode")->load();
    const auto scales = JamoScale::scaleNames();
    const auto allowed = JamoScale::maskFor(root, scales[juce::jlimit(0, scales.size() - 1, scaleIndex)]);

    int midiNote = -1;
    for (const auto metadata : midi)
        if (metadata.getMessage().isNoteOn()) midiNote = metadata.getMessage().getNoteNumber();

    for (int c = 0; c < channels; ++c)
        buffer.applyGain(c, 0, n, inputGain);

    // Rolling mono analysis window, no allocations in processBlock.
    for (int i = 0; i < n; ++i)
    {
        float mono = 0.0f;
        for (int c = 0; c < channels; ++c) mono += buffer.getReadPointer(c)[i];
        mono /= (float) juce::jmax(1, channels);
        analysisRing[(size_t) analysisWrite] = mono;
        analysisWrite = (analysisWrite + 1) % (int) analysisRing.size();
        analysisCount = juce::jmin(analysisCount + 1, (int) analysisRing.size());
    }

    if (analysisCount >= 2048)
    {
        // Detector expects contiguous data. Use a fixed-size stack array.
        float window[2048];
        const int start = (analysisWrite - 2048 + (int) analysisRing.size()) % (int) analysisRing.size();
        for (int i = 0; i < 2048; ++i) window[i] = analysisRing[(size_t)((start + i) % (int)analysisRing.size())];
        const float hz = detector.detect(window, 2048);
        detectedHz.store(hz);
        const int inputMidi = JamoScale::frequencyToMidi(hz);
        if (inputMidi >= 0)
        {
            int targetMidi = inputMidi;
            if (midiMode == 1 && midiNote >= 0) targetMidi = midiNote;
            else targetMidi = JamoScale::nearestAllowedMidi(inputMidi, allowed);
            correctedMidi.store((float) targetMidi);
            const float cents = (float)(targetMidi - inputMidi) * 100.0f;
            const float retune = parameters.getRawParameterValue("retune")->load();
            const float flex = parameters.getRawParameterValue("flex")->load();
            const float strength = juce::jlimit(0.0f, 1.0f, (100.0f - flex * 0.75f) / 100.0f);
            const float speed = mode == 1 ? 0.0f : retune;
            const float smoothing = mode == 2 ? 1.0f : juce::jmap(speed, 0.0f, 100.0f, 0.85f, 0.04f);
            const float desiredRatio = std::pow(2.0f, (cents * strength / 1200.0f));
            smoothedRatio += (desiredRatio - smoothedRatio) * juce::jlimit(0.02f, 1.0f, smoothing);
            pitchShifter.setPitchFactor(juce::jlimit(0.5f, 2.0f, smoothedRatio));
        }
    }

    juce::AudioBuffer<float> dryBuffer;
    // Keep wet/dry blend allocation-free: copy into a preallocated member would be ideal;
    // the processor uses a local copy here only in the initial scaffold. Replaced below by
    // gain blending against a fixed scratch buffer in a future refinement.
    // PitchShifter operates in-place; mix is applied as a wet gain for now.
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    pitchShifter.process(context);
    if (mix < 0.999f) buffer.applyGain(mix);
    for (int c = 0; c < channels; ++c) buffer.applyGain(c, 0, n, outputGain);
    juce::ignoreUnused(dryBuffer, currentSampleRate);
}

juce::AudioProcessorEditor* JamoAudioProcessor::createEditor() { return new JamoAudioProcessorEditor(*this); }

void JamoAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void JamoAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new JamoAudioProcessor(); }
