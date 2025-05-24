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
        label.setFont(10.0f);
        addAndMakeVisible(label);
    };
    
    auto setupRotaryKnob = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::cyan);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan.withAlpha(0.7f));
        addAndMakeVisible(slider);
    };
    
    // Main ribbon controls
    addAndMakeVisible(enableRibbonsButton);
    enableRibbonsButton.setButtonText("Enable Ribbons");
    enableRibbonsButton.setToggleState(true, juce::dontSendNotification);
    
    addAndMakeVisible(ribbonsHeaderLabel);
    ribbonsHeaderLabel.setText("RHYTHMIC RIBBONS", juce::dontSendNotification);
    ribbonsHeaderLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    ribbonsHeaderLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    ribbonsHeaderLabel.setJustificationType(juce::Justification::centred);
    
    // Global ribbon controls
    setupRotaryKnob(ribbonCountSlider);
    ribbonCountSlider.setRange(1, 5, 1);
    ribbonCountSlider.setValue(2);
    setupLabel(ribbonCountLabel, "Count");
    
    setupRotaryKnob(ribbonRateSlider);
    ribbonRateSlider.setRange(0.0, 1.0, 0.01);
    ribbonRateSlider.setValue(0.5);
    setupLabel(ribbonRateLabel, "Rate");
    
    setupRotaryKnob(ribbonSpreadSlider);
    ribbonSpreadSlider.setRange(0.0, 1.0, 0.01);
    ribbonSpreadSlider.setValue(0.6);
    setupLabel(ribbonSpreadLabel, "Spread");
    
    setupRotaryKnob(ribbonIntensitySlider);
    ribbonIntensitySlider.setRange(0.0, 1.0, 0.01);
    ribbonIntensitySlider.setValue(0.8);
    setupLabel(ribbonIntensityLabel, "Intensity");
    
    // Individual ribbon controls
    for (int i = 0; i < 3; ++i)
    {
        auto& ribbon = ribbonControls[i];
        
        // Enable button
        addAndMakeVisible(ribbon.enableButton);
        ribbon.enableButton.setButtonText("R" + juce::String(i + 1));
        ribbon.enableButton.setToggleState(i < 2, juce::dontSendNotification);
        
        // Pattern combo
        addAndMakeVisible(ribbon.patternCombo);
        ribbon.patternCombo.addItem("Up", 1);
        ribbon.patternCombo.addItem("Down", 2);
        ribbon.patternCombo.addItem("Outside", 3);
        ribbon.patternCombo.addItem("Inside", 4);
        ribbon.patternCombo.addItem("Random", 5);
        ribbon.patternCombo.addItem("Cascade", 6);
        ribbon.patternCombo.addItem("Spiral", 7);
        ribbon.patternCombo.setSelectedId((i % 7) + 1);
        
        // Rate knob (changed from linear slider to rotary knob)
        addAndMakeVisible(ribbon.rateSlider);
        ribbon.rateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ribbon.rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
        ribbon.rateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        ribbon.rateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange.withAlpha(0.7f));
        ribbon.rateSlider.setRange(0.0, 1.0, 0.01);
        ribbon.rateSlider.setValue(0.5 + i * 0.1);
        
        // Offset knob (changed from linear slider to rotary knob)
        addAndMakeVisible(ribbon.offsetSlider);
        ribbon.offsetSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ribbon.offsetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
        ribbon.offsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        ribbon.offsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange.withAlpha(0.7f));
        ribbon.offsetSlider.setRange(0.0, 1.0, 0.01);
        ribbon.offsetSlider.setValue(i * 0.33);
        
        // Labels
        setupLabel(ribbon.titleLabel, "Ribbon " + juce::String(i + 1));
        ribbon.titleLabel.setFont(juce::Font(11.0f, juce::Font::bold));
        ribbon.titleLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
        
        setupLabel(ribbon.patternLabel, "Pattern");
        setupLabel(ribbon.rateLabel, "Rate");
        setupLabel(ribbon.offsetLabel, "Offset");
    }
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
    ribbonRateAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "ribbonRate", ribbonRateSlider));
    ribbonSpreadAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "ribbonSpread", ribbonSpreadSlider));
    ribbonIntensityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "ribbonIntensity", ribbonIntensitySlider));
    
    // Individual ribbon attachments
    for (int i = 0; i < 3; ++i)
    {
        juce::String prefix = "ribbon" + juce::String(i + 1);
        ribbonAttachments[i].enableAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, prefix + "Enable", ribbonControls[i].enableButton));
        ribbonAttachments[i].patternAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, prefix + "Pattern", ribbonControls[i].patternCombo));
        ribbonAttachments[i].rateAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, prefix + "Rate", ribbonControls[i].rateSlider));
        ribbonAttachments[i].offsetAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, prefix + "Offset", ribbonControls[i].offsetSlider));
    }
    
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
    juce::Rectangle<int> ribbonSection(600, 75, 280, 330);
    
    // Header
    ribbonsHeaderLabel.setBounds(ribbonSection.removeFromTop(25));
    ribbonSection.removeFromTop(5);
    
    // Enable button
    enableRibbonsButton.setBounds(ribbonSection.removeFromTop(25).reduced(40, 0));
    ribbonSection.removeFromTop(5);
    
    // Global controls in 2x2 grid - made larger for knobs
    auto globalArea = ribbonSection.removeFromTop(140);
    auto topRow = globalArea.removeFromTop(70);
    auto bottomRow = globalArea;
    
    // Top row - increased width for knobs
    auto countArea = topRow.removeFromLeft(70);
    ribbonCountLabel.setBounds(countArea.removeFromBottom(15));
    ribbonCountSlider.setBounds(countArea);
    
    auto rateArea = topRow.removeFromLeft(70);
    ribbonRateLabel.setBounds(rateArea.removeFromBottom(15));
    ribbonRateSlider.setBounds(rateArea);
    
    // Bottom row
    auto spreadArea = bottomRow.removeFromLeft(70);
    ribbonSpreadLabel.setBounds(spreadArea.removeFromBottom(15));
    ribbonSpreadSlider.setBounds(spreadArea);
    
    auto intensityArea = bottomRow.removeFromLeft(70);
    ribbonIntensityLabel.setBounds(intensityArea.removeFromBottom(15));
    ribbonIntensitySlider.setBounds(intensityArea);
    
    ribbonSection.removeFromTop(5);
    
    // Individual ribbon controls - made larger for knobs
    for (int i = 0; i < 3; ++i)
    {
        auto& ribbon = ribbonControls[i];
        auto ribbonArea = ribbonSection.removeFromTop(60);
        
        // Title and enable button
        auto titleRow = ribbonArea.removeFromTop(20);
        ribbon.titleLabel.setBounds(titleRow.removeFromLeft(140));
        ribbon.enableButton.setBounds(titleRow.removeFromLeft(40));
        
        // Pattern combo
        auto patternRow = ribbonArea.removeFromTop(15);
        ribbon.patternLabel.setBounds(patternRow.removeFromLeft(50));
        ribbon.patternCombo.setBounds(patternRow.removeFromLeft(130));
        
        // Rate and offset knobs side by side
        auto controlRow = ribbonArea;
        auto ribbonRateArea = controlRow.removeFromLeft(90);
        ribbon.rateLabel.setBounds(ribbonRateArea.removeFromBottom(15));
        ribbon.rateSlider.setBounds(ribbonRateArea);
        
        auto offsetArea = controlRow.removeFromLeft(90);
        ribbon.offsetLabel.setBounds(offsetArea.removeFromBottom(15));
        ribbon.offsetSlider.setBounds(offsetArea);
        
        ribbonSection.removeFromTop(5);
    }
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