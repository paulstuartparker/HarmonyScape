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
    
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
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
    setupLabel(chordDensityLabel, "Chord Density");
    setupLabel(chordDensityDescLabel, "More chord tones and extensions");
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
    
    // ADSR visualizer
    addAndMakeVisible(adsrVisualizer);
    
    // Set up new ribbon controls
    setupRibbonControls();
    
    // Set up enhanced spatial controls
    setupSpatialControls();
    
    // Set up spatial visualizer
    addAndMakeVisible(spatialVisualizer);
    
    // Create all parameter attachments
    createParameterAttachments();

    // Start timer to update the keyboard display
    startTimerHz(24); // Refresh at 24Hz for smooth display
    
    // Set editor size
    setSize (1200, 800);  // Increased size for new controls
}

HarmonyScapeAudioProcessorEditor::~HarmonyScapeAudioProcessorEditor()
{
    stopTimer();
}

void HarmonyScapeAudioProcessorEditor::setupRibbonControls()
{
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(11.0f);
        addAndMakeVisible(label);
    };
    
    auto setupMasterKnob = [this](juce::Slider& slider, juce::Colour color) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setColour(juce::Slider::thumbColourId, color);
        slider.setColour(juce::Slider::rotarySliderFillColourId, color.withAlpha(0.7f));
        slider.setRange(0.0, 1.0, 0.01);
        addAndMakeVisible(slider);
    };
    
    // Header
    addAndMakeVisible(ribbonsHeaderLabel);
    ribbonsHeaderLabel.setText("RHYTHMIC RIBBONS", juce::dontSendNotification);
    ribbonsHeaderLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    ribbonsHeaderLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    ribbonsHeaderLabel.setJustificationType(juce::Justification::centred);
    
    // Enable and count controls
    addAndMakeVisible(enableRibbonsButton);
    enableRibbonsButton.setButtonText("Enable Ribbons");
    enableRibbonsButton.setToggleState(true, juce::dontSendNotification);
    enableRibbonsButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::cyan);
    
    setupMasterKnob(ribbonCountSlider, juce::Colours::cyan);
    ribbonCountSlider.setRange(1, 5, 1);
    ribbonCountSlider.setValue(3);
    setupLabel(ribbonCountLabel, "Count");
    
    // MASTER CONTROLS - These are the magic! 🎵
    setupMasterKnob(pulseSlider, juce::Colours::orange);
    pulseSlider.setValue(0.5);
    setupLabel(pulseLabel, "Pulse");
    
    setupMasterKnob(variationSlider, juce::Colours::yellow);
    variationSlider.setValue(0.5);
    setupLabel(variationLabel, "Variation");
    
    setupMasterKnob(wobbleSlider, juce::Colours::lime);
    wobbleSlider.setValue(0.3);
    setupLabel(wobbleLabel, "Wobble");
    
    setupMasterKnob(swingSlider, juce::Colours::magenta);
    swingSlider.setValue(0.0);
    setupLabel(swingLabel, "Swing");
    
    setupMasterKnob(shimmerSlider, juce::Colours::lightblue);
    shimmerSlider.setValue(0.2);
    setupLabel(shimmerLabel, "Shimmer");
}

void HarmonyScapeAudioProcessorEditor::setupSpatialControls()
{
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(10.0f);
        addAndMakeVisible(label);
    };
    
    auto setupRotaryKnob = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::yellow);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::yellow.withAlpha(0.7f));
        addAndMakeVisible(slider);
    };
    
    // Enable movement
    addAndMakeVisible(enableMovementButton);
    enableMovementButton.setButtonText("Enable Movement");
    enableMovementButton.setToggleState(true, juce::dontSendNotification);
    
    addAndMakeVisible(movementLabel);
    movementLabel.setText("SPATIAL MOVEMENT", juce::dontSendNotification);
    movementLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    movementLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    movementLabel.setJustificationType(juce::Justification::centred);
    
    // Movement controls
    setupRotaryKnob(movementRateSlider);
    movementRateSlider.setRange(0.0, 1.0, 0.01);
    movementRateSlider.setValue(0.5);
    setupLabel(movementRateLabel, "Rate");
    
    setupRotaryKnob(movementDepthSlider);
    movementDepthSlider.setRange(0.0, 1.0, 0.01);
    movementDepthSlider.setValue(0.3);
    setupLabel(movementDepthLabel, "Depth");
    
    setupRotaryKnob(heightSlider);
    heightSlider.setRange(0.0, 1.0, 0.01);
    heightSlider.setValue(0.5);
    setupLabel(heightLabel, "Height");
    
    setupRotaryKnob(depthSlider);
    depthSlider.setRange(0.0, 1.0, 0.01);
    depthSlider.setValue(0.5);
    setupLabel(depthLabel, "Depth");
}

void HarmonyScapeAudioProcessorEditor::createParameterAttachments()
{
    // Core attachments
    chordDensityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "chordDensity", chordDensitySlider));
    spatialWidthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "spatialWidth", spatialWidthSlider));
    waveformAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "waveform", waveformCombo));
    volumeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "volume", volumeSlider));
    
    // ADSR attachments
    attackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "attack", attackSlider));
    decayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "decay", decaySlider));
    sustainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "sustain", sustainSlider));
    releaseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "release", releaseSlider));
    
    // Ribbon attachments
    enableRibbonsAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "enableRibbons", enableRibbonsButton));
    ribbonCountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "ribbonCount", ribbonCountSlider));
    pulseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "pulse", pulseSlider));
    variationAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "variation", variationSlider));
    wobbleAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "wobble", wobbleSlider));
    swingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "swing", swingSlider));
    shimmerAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "shimmer", shimmerSlider));
    
    // Enhanced spatial attachments
    enableMovementAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "enableMovement", enableMovementButton));
    movementRateAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "movementRate", movementRateSlider));
    movementDepthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "movementDepth", movementDepthSlider));
    heightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "height", heightSlider));
    depthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "depth", depthSlider));
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
    
    // Draw version number
    g.setFont(12.0f);
    g.setColour(juce::Colours::lightgrey);
    g.drawText(HARMONYSCAPE_VERSION_STRING, getWidth() - 100, 10, 75, 15, juce::Justification::right, false);
    
    // Draw colored square
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
    
    // Ribbon controls section
    juce::Rectangle<int> ribbonSection(590, 50, 300, 360);
    g.drawRoundedRectangle(ribbonSection.toFloat(), 5.0f, 1.0f);
    g.drawText("Rhythmic Ribbons", ribbonSection.removeFromTop(25), juce::Justification::centred);
    
    // Spatial controls section
    juce::Rectangle<int> spatialSection(900, 50, 290, 360);
    g.drawRoundedRectangle(spatialSection.toFloat(), 5.0f, 1.0f);
    g.drawText("Spatial Movement", spatialSection.removeFromTop(25), juce::Justification::centred);
    
    // ADSR section
    juce::Rectangle<int> adsrSection(10, 220, 570, 160);
    g.drawRoundedRectangle(adsrSection.toFloat(), 5.0f, 1.0f);
    g.drawText("ADSR Envelope", adsrSection.removeFromTop(25), juce::Justification::centred);
    
    // Keyboard section
    juce::Rectangle<int> keyboardSection(10, 420, 1180, 100);
    g.drawRoundedRectangle(keyboardSection.toFloat(), 5.0f, 1.0f);
    
    // Draw keyboard legend
    g.setFont(14.0f);
    juce::Rectangle<int> legendArea(20, 420, 580, 25);
    g.setColour(juce::Colours::blue.withAlpha(0.7f));
    g.fillRect(legendArea.removeFromLeft(20).reduced(5, 5));
    g.setColour(juce::Colours::white);
    g.drawText("User Input", legendArea.removeFromLeft(100), juce::Justification::centredLeft);
    
    g.setColour(juce::Colours::green.withAlpha(0.7f));
    g.fillRect(legendArea.removeFromLeft(20).reduced(5, 5));
    g.setColour(juce::Colours::white);
    g.drawText("Generated Notes", legendArea.removeFromLeft(150), juce::Justification::centredLeft);
    
    g.setColour(juce::Colours::orange.withAlpha(0.7f));
    g.fillRect(legendArea.removeFromLeft(20).reduced(5, 5));
    g.setColour(juce::Colours::white);
    g.drawText("Ribbon Notes", legendArea.removeFromLeft(150), juce::Justification::centredLeft);
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
    chordDensityDescLabel.setBounds(densityColumn.removeFromTop(20));
    
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
    juce::Rectangle<int> adsrSection(10, 245, 570, 145);
    
    // ADSR Visualizer on the left
    adsrVisualizer.setBounds(adsrSection.removeFromLeft(200).reduced(5));
    adsrSection.removeFromLeft(marginX);
    
    // ADSR controls
    juce::Rectangle<int> adsrLabels = adsrSection.removeFromTop(labelHeight);
    attackLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    decayLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    sustainLabel.setBounds(adsrLabels.removeFromLeft(90));
    adsrLabels.removeFromLeft(5);
    releaseLabel.setBounds(adsrLabels.removeFromLeft(90));
    
    attackSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    decaySlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    sustainSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    adsrSection.removeFromLeft(5);
    releaseSlider.setBounds(adsrSection.removeFromLeft(90).removeFromTop(sliderHeight));
    
    // Ribbon controls section
    layoutRibbonControls();
    
    // Spatial controls section
    layoutSpatialControls();
    
    // Keyboard section
    customKeyboard.setBounds(10, 445, 1180, 75);
}

void HarmonyScapeAudioProcessorEditor::layoutRibbonControls()
{
    juce::Rectangle<int> ribbonSection(600, 75, 300, 330);
    
    // Header
    ribbonsHeaderLabel.setBounds(ribbonSection.removeFromTop(25));
    ribbonSection.removeFromTop(10);
    
    // Enable button
    enableRibbonsButton.setBounds(ribbonSection.removeFromTop(25).reduced(40, 0));
    ribbonSection.removeFromTop(10);
    
    // Count control (centered)
    auto countArea = ribbonSection.removeFromTop(80);
    auto countCenter = countArea.withSizeKeepingCentre(70, 80);
    ribbonCountLabel.setBounds(countCenter.removeFromBottom(15));
    ribbonCountSlider.setBounds(countCenter);
    
    ribbonSection.removeFromTop(10);
    
    // Master controls in 3x2 grid
    auto masterArea = ribbonSection.removeFromTop(170);
    auto topRow = masterArea.removeFromTop(85);
    auto bottomRow = masterArea;
    
    // Top row: Pulse, Variation, Wobble  
    auto pulseArea = topRow.removeFromLeft(90);
    pulseLabel.setBounds(pulseArea.removeFromBottom(15));
    pulseSlider.setBounds(pulseArea);
    
    auto variationArea = topRow.removeFromLeft(90);
    variationLabel.setBounds(variationArea.removeFromBottom(15));
    variationSlider.setBounds(variationArea);
    
    auto wobbleArea = topRow.removeFromLeft(90);
    wobbleLabel.setBounds(wobbleArea.removeFromBottom(15));
    wobbleSlider.setBounds(wobbleArea);
    
    // Bottom row: Swing, Shimmer (centered)
    auto swingArea = bottomRow.removeFromLeft(120);
    swingLabel.setBounds(swingArea.removeFromBottom(15));
    swingSlider.setBounds(swingArea.reduced(15, 0));
    
    auto shimmerArea = bottomRow.removeFromLeft(120);
    shimmerLabel.setBounds(shimmerArea.removeFromBottom(15));
    shimmerSlider.setBounds(shimmerArea.reduced(15, 0));
}

void HarmonyScapeAudioProcessorEditor::layoutSpatialControls()
{
    juce::Rectangle<int> spatialSection(910, 75, 270, 330);
    
    // Header
    movementLabel.setBounds(spatialSection.removeFromTop(25));
    spatialSection.removeFromTop(5);
    
    // Enable button
    enableMovementButton.setBounds(spatialSection.removeFromTop(25).reduced(40, 0));
    spatialSection.removeFromTop(5);
    
    // Spatial visualizer - made slightly smaller to accommodate knobs
    spatialVisualizer.setBounds(spatialSection.removeFromTop(80).reduced(10));
    spatialSection.removeFromTop(5);
    
    // Control knobs in 2x2 grid - made larger for proper knob display
    auto controlArea = spatialSection.removeFromTop(140);
    auto topRow = controlArea.removeFromTop(70);
    auto bottomRow = controlArea;
    
    // Top row
    auto rateArea = topRow.removeFromLeft(65);
    movementRateLabel.setBounds(rateArea.removeFromBottom(15));
    movementRateSlider.setBounds(rateArea);
    
    auto depthArea = topRow.removeFromLeft(65);
    movementDepthLabel.setBounds(depthArea.removeFromBottom(15));
    movementDepthSlider.setBounds(depthArea);
    
    // Bottom row
    auto heightArea = bottomRow.removeFromLeft(65);
    heightLabel.setBounds(heightArea.removeFromBottom(15));
    heightSlider.setBounds(heightArea);
    
    auto depthSliderArea = bottomRow.removeFromLeft(65);
    depthLabel.setBounds(depthSliderArea.removeFromBottom(15));
    depthSlider.setBounds(depthSliderArea);
    
    // Use remaining space for additional info/controls if needed
    auto remainingSpace = spatialSection;
    if (remainingSpace.getHeight() > 20)
    {
        // Could add tempo sync indicator, pattern info, etc.
        // For now, leave space for future features
    }
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
    customKeyboard.setRibbonNotes(clampNotes(audioProcessor.getRibbonNotes()));
    
    // Update ADSR visualizer
    float attack = *valueTreeState.getRawParameterValue("attack");
    float decay = *valueTreeState.getRawParameterValue("decay");
    float sustain = *valueTreeState.getRawParameterValue("sustain");
    float release = *valueTreeState.getRawParameterValue("release");
    adsrVisualizer.setADSR(attack, decay, sustain, release);
    
    // Update spatial visualizer
    float spatialWidth = *valueTreeState.getRawParameterValue("spatialWidth");
    float height = *valueTreeState.getRawParameterValue("height");
    float depth = *valueTreeState.getRawParameterValue("depth");
    bool movement = *valueTreeState.getRawParameterValue("enableMovement") > 0.5f;
    spatialVisualizer.setSpatialParams(spatialWidth, height, depth, movement);
    
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