#include "PluginEditor.h"

static void styleSlider(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e5c3));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff4f7fb));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff171a21));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff343a46));
}

JamoAudioProcessorEditor::JamoAudioProcessorEditor(JamoAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 520);
    title.setText("JAMO", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(34.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xff00e5c3));
    addAndMakeVisible(title);
    subtitle.setText("VOCAL TUNING  /  REAL-TIME PITCH CONTROL", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffa5adbb));
    addAndMakeVisible(subtitle);

    for (auto* s : { &retune, &humanize, &flex, &formant, &mix }) { styleSlider(*s); addAndMakeVisible(*s); }
    retune.setName("RETUNE SPEED"); humanize.setName("HUMANIZE"); flex.setName("FLEX TUNE");
    formant.setName("FORMANT"); mix.setName("MIX");
    for (auto* c : { &keyBox, &scaleBox, &modeBox }) addAndMakeVisible(*c);
    keyBox.addItemList({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 1);
    scaleBox.addItemList(JamoScale::scaleNames(), 1);
    modeBox.addItemList({ "Natural","Hard Tune","Robot" }, 1);
    keyBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff171a21));
    scaleBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff171a21));
    modeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff171a21));

    keyAttachment = std::make_unique<JamoAudioProcessor::APVTS::ComboBoxAttachment>(processor.parameters, "root", keyBox);
    scaleAttachment = std::make_unique<JamoAudioProcessor::APVTS::ComboBoxAttachment>(processor.parameters, "scale", scaleBox);
    modeAttachment = std::make_unique<JamoAudioProcessor::APVTS::ComboBoxAttachment>(processor.parameters, "mode", modeBox);
    retuneAttachment = std::make_unique<JamoAudioProcessor::APVTS::SliderAttachment>(processor.parameters, "retune", retune);
    humanizeAttachment = std::make_unique<JamoAudioProcessor::APVTS::SliderAttachment>(processor.parameters, "humanize", humanize);
    flexAttachment = std::make_unique<JamoAudioProcessor::APVTS::SliderAttachment>(processor.parameters, "flex", flex);
    formantAttachment = std::make_unique<JamoAudioProcessor::APVTS::SliderAttachment>(processor.parameters, "formant", formant);
    mixAttachment = std::make_unique<JamoAudioProcessor::APVTS::SliderAttachment>(processor.parameters, "mix", mix);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff252b35));
    bypassButton.onClick = [this] { processor.parameters.getParameter("bypass")->setValueNotifyingHost(bypassButton.getToggleState() ? 1.0f : 0.0f); };
    addAndMakeVisible(bypassButton);
    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchLabel.setFont(juce::Font(juce::FontOptions(23.0f, juce::Font::bold)));
    pitchLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(pitchLabel);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00e5c3));
    addAndMakeVisible(statusLabel);
    startTimerHz(12);
}

JamoAudioProcessorEditor::~JamoAudioProcessorEditor() { stopTimer(); }

void JamoAudioProcessorEditor::timerCallback()
{
    const float hz = processor.getDetectedHz();
    const float midi = processor.getCorrectedMidi();
    if (hz > 0.0f)
    {
        const int note = JamoScale::frequencyToMidi(hz);
        const int octave = note / 12 - 1;
        pitchLabel.setText(juce::String(JamoScale::noteNames[(note % 12 + 12) % 12]) + juce::String(octave)
                           + "  ·  " + juce::String(hz, 1) + " Hz", juce::dontSendNotification);
        if (midi >= 0)
            statusLabel.setText("TARGET  " + juce::String(JamoScale::noteNames[((int)midi % 12 + 12) % 12]) + juce::String((int)midi / 12 - 1), juce::dontSendNotification);
    }
    else { pitchLabel.setText("--  ·  LISTENING", juce::dontSendNotification); statusLabel.setText("NO STABLE PITCH", juce::dontSendNotification); }
}

void JamoAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0f14));
    g.setColour(juce::Colour(0xff1b2029)); g.fillRoundedRectangle(18.0f, 92.0f, 724.0f, 110.0f, 12.0f);
    g.setColour(juce::Colour(0xff00e5c3)); g.fillRoundedRectangle(18.0f, 92.0f, 4.0f, 110.0f, 2.0f);
    g.setColour(juce::Colour(0xff252b35)); g.drawRoundedRectangle(18.0f, 220.0f, 724.0f, 276.0f, 12.0f, 1.0f);
    g.setColour(juce::Colour(0xffa5adbb)); g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("KEY", 32, 72, 90, 18, juce::Justification::centredLeft);
    g.drawText("SCALE", 143, 72, 110, 18, juce::Justification::centredLeft);
    g.drawText("STYLE", 276, 72, 120, 18, juce::Justification::centredLeft);
    const juce::String labels[] = { "RETUNE SPEED", "HUMANIZE", "FLEX TUNE", "FORMANT", "DRY / WET" };
    for (int i = 0; i < 5; ++i) g.drawText(labels[i], 34 + i * 143, 235, 120, 18, juce::Justification::centred);
    g.setColour(juce::Colour(0xff555e6e)); g.drawText("JAMO AUDIO  •  V0.1.0  •  LOW-LATENCY MODE", 24, 500, 710, 14, juce::Justification::centredLeft);
}

void JamoAudioProcessorEditor::resized()
{
    title.setBounds(24, 12, 180, 42); subtitle.setBounds(26, 50, 360, 20);
    bypassButton.setBounds(638, 24, 100, 34);
    keyBox.setBounds(32, 92, 95, 30); scaleBox.setBounds(143, 92, 120, 30); modeBox.setBounds(276, 92, 150, 30);
    pitchLabel.setBounds(440, 106, 270, 38); statusLabel.setBounds(440, 145, 270, 24);
    juce::Slider* sliders[] = { &retune, &humanize, &flex, &formant, &mix };
    for (int i = 0; i < 5; ++i) sliders[i]->setBounds(22 + i * 143, 258, 136, 210);
}
