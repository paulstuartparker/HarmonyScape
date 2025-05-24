#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Version.h"

//==============================================================================
HarmonyScapeAudioProcessorEditor::HarmonyScapeAudioProcessorEditor (HarmonyScapeAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), 
      audioProcessor (p), 
      valueTreeState (vts),
      midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      customKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set up slider styles
    auto setupRotarySlider = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);
    };
    
    auto setupLinearSlider = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);
    };
    
    // Main parameter sliders - chord density as horizontal slider for better feedback
    setupLinearSlider(chordDensitySlider);
    setupRotarySlider(spatialWidthSlider);
    
    // Waveform combobox
    waveformCombo.addItem("Sine", 1);
    waveformCombo.addItem("Saw", 2);
    waveformCombo.addItem("Square", 3);
    waveformCombo.addItem("Triangle", 4);
    waveformCombo.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waveformCombo);
    
    // Volume slider
    setupRotarySlider(volumeSlider);
    
    // ADSR sliders
    setupRotarySlider(attackSlider);
    setupRotarySlider(decaySlider);
    setupRotarySlider(sustainSlider);
    setupRotarySlider(releaseSlider);
    
    // Set up labels
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };
    
    // Main parameter labels
    setupLabel(chordDensityLabel, "Chord Density");
    setupLabel(chordDensityDescLabel, "Minimal harmony");
    setupLabel(spatialWidthLabel, "Spatial Width");
    
    // Configure chord density description label with smaller font
    chordDensityDescLabel.setFont(12.0f);
    chordDensityDescLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    // Synth control labels
    setupLabel(waveformLabel, "Waveform");
    setupLabel(volumeLabel, "Volume");
    
    // ADSR labels
    setupLabel(attackLabel, "Attack");
    setupLabel(decayLabel, "Decay");
    setupLabel(sustainLabel, "Sustain");
    setupLabel(releaseLabel, "Release");
    
    // Set up keyboard display
    customKeyboard.setAvailableRange(36, 96); // 5 octaves
    customKeyboard.setOctaveForMiddleC(4);
    customKeyboard.setLowestVisibleKey(48); // C3
    addAndMakeVisible(customKeyboard);
    
    // Create attachments to parameters
    chordDensityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "chordDensity", chordDensitySlider));
    spatialWidthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "spatialWidth", spatialWidthSlider));
    waveformAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        valueTreeState, "waveform", waveformCombo));
    volumeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "volume", volumeSlider));
    
    // ADSR attachments
    attackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "attack", attackSlider));
    decayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "decay", decaySlider));
    sustainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "sustain", sustainSlider));
    releaseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        valueTreeState, "release", releaseSlider));

    // Add ADSR visualizer
    addAndMakeVisible(adsrVisualizer);

    // Start timer to update the keyboard display
    startTimerHz(24); // Refresh at 24Hz for smooth display
    
    // Set editor size
    setSize (600, 500);
}

HarmonyScapeAudioProcessorEditor::~HarmonyScapeAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void HarmonyScapeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll (juce::Colours::darkgrey);

    // Draw the plugin title
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("HarmonyScape", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    
    // Draw version number with more space before the colored square
    g.setFont(12.0f);
    g.setColour(juce::Colours::lightgrey);
    g.drawText(HARMONYSCAPE_VERSION_STRING, getWidth() - 100, 10, 75, 15, juce::Justification::right, false);
    
    // Draw colored square with more space from version number
    g.setColour(juce::Colour::fromFloatRGBA(BUILD_COLOR_R, BUILD_COLOR_G, BUILD_COLOR_B, 1.0f));
    g.fillRect(getWidth() - 20, 10, 15, 15);
    g.setColour(juce::Colours::white);
    g.drawRect(getWidth() - 20, 10, 15, 15, 1);
    
    // Draw section borders
    g.setColour(juce::Colours::lightgrey);
    
    // Main controls section
    juce::Rectangle<int> mainSection(10, 50, 280, 160);
    g.drawRoundedRectangle(mainSection.toFloat(), 5.0f, 1.0f);
    g.setFont(16.0f);
    g.drawText("Main Controls", mainSection.removeFromTop(25), juce::Justification::centred);
    
    // Synth controls section
    juce::Rectangle<int> synthSection(300, 50, 280, 160);
    g.drawRoundedRectangle(synthSection.toFloat(), 5.0f, 1.0f);
    g.drawText("Synth Controls", synthSection.removeFromTop(25), juce::Justification::centred);
    
    // ADSR section
    juce::Rectangle<int> adsrSection(10, 220, 570, 160);
    g.drawRoundedRectangle(adsrSection.toFloat(), 5.0f, 1.0f);
    g.drawText("ADSR Envelope", adsrSection.removeFromTop(25), juce::Justification::centred);
    
    // Keyboard section
    juce::Rectangle<int> keyboardSection(10, 390, 580, 100);
    g.drawRoundedRectangle(keyboardSection.toFloat(), 5.0f, 1.0f);
    
    // Draw keyboard legend with only user and generated notes
    g.setFont(14.0f);
    juce::Rectangle<int> legendArea(20, 390, 580, 25);
    g.setColour(juce::Colours::blue.withAlpha(0.7f));
    g.fillRect(legendArea.removeFromLeft(20).reduced(5, 5));
    g.setColour(juce::Colours::white);
    g.drawText("User Input", legendArea.removeFromLeft(100), juce::Justification::centredLeft);
    
    g.setColour(juce::Colours::green.withAlpha(0.7f));
    g.fillRect(legendArea.removeFromLeft(20).reduced(5, 5));
    g.setColour(juce::Colours::white);
    g.drawText("Generated Notes", legendArea.removeFromLeft(150), juce::Justification::centredLeft);
}

void HarmonyScapeAudioProcessorEditor::resized()
{
    const int labelHeight = 20;
    const int sliderHeight = 80;
    const int itemWidth = 120;
    const int marginX = 20;
    const int marginY = 10;
    
    // Main controls section
    juce::Rectangle<int> mainSection(10, 75, 280, 125);
    
    // Labels row
    chordDensityLabel.setBounds(mainSection.removeFromLeft(itemWidth).removeFromTop(labelHeight));
    mainSection.removeFromLeft(marginX);
    spatialWidthLabel.setBounds(mainSection.removeFromLeft(itemWidth).removeFromTop(labelHeight));
    
    // Sliders row
    mainSection = juce::Rectangle<int>(10, 75 + labelHeight, 280, 125 - labelHeight);
    auto densityColumn = mainSection.removeFromLeft(itemWidth);
    chordDensitySlider.setBounds(densityColumn.removeFromTop(sliderHeight));
    chordDensityDescLabel.setBounds(densityColumn.removeFromTop(20)); // Description below slider
    
    mainSection.removeFromLeft(marginX);
    spatialWidthSlider.setBounds(mainSection.removeFromLeft(itemWidth).removeFromTop(sliderHeight));
    
    // Synth controls section
    juce::Rectangle<int> synthSection(300, 75, 280, 125);
    
    waveformLabel.setBounds(synthSection.removeFromLeft(itemWidth).removeFromTop(labelHeight));
    synthSection.removeFromLeft(marginX);
    volumeLabel.setBounds(synthSection.removeFromLeft(itemWidth).removeFromTop(labelHeight));
    
    synthSection = juce::Rectangle<int>(300, 75 + labelHeight, 280, 125 - labelHeight);
    waveformCombo.setBounds(synthSection.removeFromLeft(itemWidth).removeFromTop(30).reduced(10, 0));
    synthSection.removeFromLeft(marginX);
    volumeSlider.setBounds(synthSection.removeFromLeft(itemWidth).removeFromTop(sliderHeight));
    
    // ADSR section
    juce::Rectangle<int> adsrSection(10, 245, 580, 145);
    
    // ADSR Visualizer on the left
    adsrVisualizer.setBounds(adsrSection.removeFromLeft(200).reduced(5));
    adsrSection.removeFromLeft(marginX);
    
    // First row - labels
    juce::Rectangle<int> adsrLabels = adsrSection.removeFromTop(labelHeight);
    attackLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    decayLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    sustainLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    releaseLabel.setBounds(adsrLabels.removeFromLeft(90));
    
    // Second row - sliders
    attackSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    decaySlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    sustainSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    releaseSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    
    // Keyboard section
    customKeyboard.setBounds(10, 415, 580, 75);
}

void HarmonyScapeAudioProcessorEditor::timerCallback()
{
    // Clamp notes to visible keyboard range
    auto clampNotes = [](const juce::Array<int>& notes) {
        juce::Array<int> clamped;
        for (auto n : notes) {
            if (n >= 36 && n <= 96) clamped.add(n);
        }
        return clamped;
    };
    customKeyboard.setUserNotes(clampNotes(audioProcessor.getUserInputNotes()));
    customKeyboard.setGeneratedNotes(clampNotes(audioProcessor.getGeneratedNotes()));
    
    // Update ADSR visualizer
    float attack = *valueTreeState.getRawParameterValue("attack");
    float decay = *valueTreeState.getRawParameterValue("decay");
    float sustain = *valueTreeState.getRawParameterValue("sustain");
    float release = *valueTreeState.getRawParameterValue("release");
    adsrVisualizer.setADSR(attack, decay, sustain, release);
    
    // Update chord density description based on current value
    float density = *valueTreeState.getRawParameterValue("chordDensity");
    if (density < 0.3f)
        chordDensityDescLabel.setText("Minimal harmony", juce::dontSendNotification);
    else if (density < 0.6f)
        chordDensityDescLabel.setText("Basic chords", juce::dontSendNotification);
    else if (density < 0.8f)
        chordDensityDescLabel.setText("Rich harmony", juce::dontSendNotification);
    else
        chordDensityDescLabel.setText("Full voicing", juce::dontSendNotification);
} 