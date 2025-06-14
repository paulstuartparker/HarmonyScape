#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
 * HarmonyScape plugin editor with enhanced UI controls for ribbons and spatial features
 */
class HarmonyScapeAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    HarmonyScapeAudioProcessorEditor (HarmonyScapeAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~HarmonyScapeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for updating the keyboard display
    void timerCallback() override;

private:
    // Setup methods for UI components
    void setupRibbonControls();
    void setupSpatialControls();
    void createParameterAttachments();
    void layoutRibbonControls();
    void layoutSpatialControls();
    
    // Reference to our processor
    HarmonyScapeAudioProcessor& audioProcessor;
    
    // Reference to parameter tree
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    // IMPORTANT: keyboardState must be declared BEFORE any components that use it
    juce::MidiKeyboardState keyboardState;
    
    // UI Components - Keyboard display
    juce::MidiKeyboardComponent midiKeyboard;
    juce::Array<int> userNotes;     // Notes played by user
    juce::Array<int> generatedNotes; // Notes generated by chord engine
    
    // UI Components - Synth controls
    juce::ComboBox waveformCombo;
    juce::Slider volumeSlider;
    juce::Label waveformLabel;
    juce::Label volumeLabel;
    
    // UI Components - Main parameters
    juce::Slider chordDensitySlider;
    juce::Slider spatialWidthSlider;
    juce::Label chordDensityLabel;
    juce::Label chordDensityDescLabel; // Description of what chord density does
    juce::Label spatialWidthLabel;
    
    // UI Components - ADSR controls
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    
    // New UI Components - Ribbon Controls
    juce::ToggleButton enableRibbonsButton;
    juce::Label ribbonsLabel;
    juce::Label ribbonsHeaderLabel;
    juce::Slider ribbonCountSlider;
    juce::Label ribbonCountLabel;
    juce::Slider ribbonRateSlider;
    juce::Label ribbonRateLabel;
    juce::Slider ribbonSpreadSlider;
    juce::Label ribbonSpreadLabel;
    juce::Slider ribbonIntensitySlider;
    juce::Label ribbonIntensityLabel;
    
    // Individual ribbon controls (first 3)
    struct RibbonControlSet
    {
        juce::ToggleButton enableButton;
        juce::ComboBox patternCombo;
        juce::Slider rateSlider;
        juce::Slider offsetSlider;
        juce::Label titleLabel;
        juce::Label patternLabel;
        juce::Label rateLabel;
        juce::Label offsetLabel;
    };
    std::array<RibbonControlSet, 3> ribbonControls;
    
    // Enhanced spatial controls
    juce::ToggleButton enableMovementButton;
    juce::Label movementLabel;
    juce::Slider movementRateSlider;
    juce::Label movementRateLabel;
    juce::Slider movementDepthSlider;
    juce::Label movementDepthLabel;
    juce::Slider heightSlider;
    juce::Label heightLabel;
    juce::Slider depthSlider;
    juce::Label depthLabel;
    
    // Parameter attachments - Core
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chordDensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spatialWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    
    // Parameter attachments - Ribbons
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableRibbonsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ribbonCountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ribbonRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ribbonSpreadAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ribbonIntensityAttachment;
    
    // Individual ribbon attachments
    struct RibbonAttachmentSet
    {
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patternAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> offsetAttachment;
    };
    std::array<RibbonAttachmentSet, 3> ribbonAttachments;
    
    // Parameter attachments - Enhanced Spatial
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableMovementAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> movementRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> movementDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> heightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    
    // Custom MidiKeyboardComponent that shows different colors for user and generated notes
    class CustomMidiKeyboard : public juce::MidiKeyboardComponent
    {
    public:
        CustomMidiKeyboard(juce::MidiKeyboardState& state, juce::MidiKeyboardComponent::Orientation orientation)
            : juce::MidiKeyboardComponent(state, orientation)
        {
            this->setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colours::blue);
        }
        
        void setUserNotes(const juce::Array<int>& notes) 
        { 
            userNotesList = notes; 
            repaint();
        }
        
        void setGeneratedNotes(const juce::Array<int>& notes) 
        { 
            generatedNotesList = notes; 
            repaint();
        }
        
    protected:
        void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, 
                           bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override
        {
            // Choose color based on whether this is a user note or generated note
            juce::Colour fillColor = juce::Colours::white;
            
            if (isUserNote(midiNoteNumber))
                fillColor = juce::Colours::blue.withAlpha(0.7f);
            else if (isGeneratedNote(midiNoteNumber))
                fillColor = juce::Colours::green.withAlpha(0.7f);
            else if (isDown)
                fillColor = findColour(keyDownOverlayColourId);
            
            g.setColour(fillColor);
            g.fillRect(area);
            
            g.setColour(lineColour);
            g.drawRect(area);
        }
        
        void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, 
                          bool isDown, bool isOver, juce::Colour noteFillColour) override
        {
            // Choose color based on whether this is a user note or generated note
            juce::Colour fillColor = noteFillColour;
            
            if (isUserNote(midiNoteNumber))
                fillColor = juce::Colours::blue.withAlpha(0.7f);
            else if (isGeneratedNote(midiNoteNumber))
                fillColor = juce::Colours::green.withAlpha(0.7f);
            else if (isDown)
                fillColor = findColour(keyDownOverlayColourId);
            
            g.setColour(fillColor);
            g.fillRect(area);
            
            g.setColour(juce::Colours::black);
            g.drawRect(area);
        }
        
    private:
        bool isUserNote(int midiNoteNumber) const
        {
            return userNotesList.contains(midiNoteNumber);
        }
        
        bool isGeneratedNote(int midiNoteNumber) const
        {
            return generatedNotesList.contains(midiNoteNumber) && !userNotesList.contains(midiNoteNumber);
        }
        
        juce::Array<int> userNotesList;
        juce::Array<int> generatedNotesList;
    };
    
    // Implement custom keyboard to show colored keys
    CustomMidiKeyboard customKeyboard;
    
    // ADSR Visualizer Component
    class ADSRVisualizer : public juce::Component
    {
    public:
        ADSRVisualizer() {}
        
        void setADSR(float a, float d, float s, float r)
        {
            attack = a;
            decay = d;
            sustain = s;
            release = r;
            repaint();
        }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().reduced(10);
            
            // Background
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
            
            // Grid lines
            g.setColour(juce::Colours::grey.withAlpha(0.3f));
            const int gridLines = 4;
            for (int i = 1; i < gridLines; ++i)
            {
                float y = bounds.getY() + (bounds.getHeight() * i / gridLines);
                g.drawHorizontalLine(static_cast<int>(y), static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
            }
            
            // Calculate ADSR curve points
            const float maxTime = 4.0f; // Max 4 seconds total time
            const float totalTime = attack + decay + release + 0.5f; // 0.5f for sustain display
            const float timeScale = bounds.getWidth() / std::max(totalTime, 1.0f);
            
            juce::Path envelope;
            
            // Start point
            float x = static_cast<float>(bounds.getX());
            float y = static_cast<float>(bounds.getBottom());
            envelope.startNewSubPath(x, y);
            
            // Attack point (peak)
            x += attack * timeScale;
            y = static_cast<float>(bounds.getY());
            envelope.lineTo(x, y);
            
            // Decay point (to sustain level)
            x += decay * timeScale;
            y = bounds.getY() + bounds.getHeight() * (1.0f - sustain);
            envelope.lineTo(x, y);
            
            // Sustain (hold for display)
            x += 0.5f * timeScale;
            envelope.lineTo(x, y);
            
            // Release point (back to zero)
            x += release * timeScale;
            y = static_cast<float>(bounds.getBottom());
            envelope.lineTo(x, y);
            
            // Draw the envelope curve
            g.setColour(juce::Colours::cyan);
            g.strokePath(envelope, juce::PathStrokeType(2.0f));
            
            // Draw control points as small circles
            g.setColour(juce::Colours::white);
            const float circleRadius = 4.0f;
            
            // Attack point
            x = bounds.getX() + attack * timeScale;
            y = static_cast<float>(bounds.getY());
            g.fillEllipse(x - circleRadius, y - circleRadius, circleRadius * 2, circleRadius * 2);
            
            // Decay/Sustain point
            x = bounds.getX() + (attack + decay) * timeScale;
            y = bounds.getY() + bounds.getHeight() * (1.0f - sustain);
            g.fillEllipse(x - circleRadius, y - circleRadius, circleRadius * 2, circleRadius * 2);
            
            // Release start point
            x = bounds.getX() + (attack + decay + 0.5f) * timeScale;
            g.fillEllipse(x - circleRadius, y - circleRadius, circleRadius * 2, circleRadius * 2);
            
            // Labels
            g.setColour(juce::Colours::lightgrey);
            g.setFont(10.0f);
            g.drawText("A", bounds.getX(), bounds.getBottom() + 2, 20, 12, juce::Justification::centred);
            g.drawText("D", bounds.getX() + attack * timeScale, bounds.getBottom() + 2, 20, 12, juce::Justification::centred);
            g.drawText("S", bounds.getX() + (attack + decay) * timeScale, bounds.getBottom() + 2, 20, 12, juce::Justification::centred);
            g.drawText("R", bounds.getX() + (attack + decay + 0.5f) * timeScale, bounds.getBottom() + 2, 20, 12, juce::Justification::centred);
        }
        
    private:
        float attack = 0.01f;
        float decay = 0.1f;
        float sustain = 0.7f;
        float release = 0.2f;
    };
    
    ADSRVisualizer adsrVisualizer;
    
    // New Spatial Visualizer Component
    class SpatialVisualizer : public juce::Component
    {
    public:
        SpatialVisualizer() {}
        
        void setSpatialParams(float width, float height, float depth, bool movement)
        {
            spatialWidth = width;
            spatialHeight = height;
            spatialDepth = depth;
            movementEnabled = movement;
            repaint();
        }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().reduced(5);
            
            // Background
            g.setColour(juce::Colours::black.withAlpha(0.7f));
            g.fillRoundedRectangle(bounds.toFloat(), 3.0f);
            
            // Border
            g.setColour(juce::Colours::grey.withAlpha(0.5f));
            g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 1.0f);
            
            // Draw spatial field representation
            auto fieldBounds = bounds.reduced(10);
            
            // Center point
            float centerX = fieldBounds.getCentreX();
            float centerY = fieldBounds.getCentreY();
            
            // Draw width indication
            float widthSpan = fieldBounds.getWidth() * spatialWidth;
            g.setColour(juce::Colours::cyan.withAlpha(0.6f));
            g.drawHorizontalLine(static_cast<int>(centerY), 
                               centerX - widthSpan/2, centerX + widthSpan/2);
            
            // Draw height indication
            float heightSpan = fieldBounds.getHeight() * spatialHeight;
            g.setColour(juce::Colours::yellow.withAlpha(0.6f));
            g.drawVerticalLine(static_cast<int>(centerX), 
                             centerY - heightSpan/2, centerY + heightSpan/2);
            
            // Draw depth indication (as circles)
            g.setColour(juce::Colours::magenta.withAlpha(0.4f));
            float depthRadius = 20.0f * spatialDepth;
            g.drawEllipse(centerX - depthRadius, centerY - depthRadius, 
                         depthRadius * 2, depthRadius * 2, 1.0f);
            
            // Movement indicator
            if (movementEnabled)
            {
                g.setColour(juce::Colours::white.withAlpha(0.8f));
                g.fillEllipse(centerX - 3, centerY - 3, 6, 6);
                
                // Draw movement arrows
                g.drawArrow(juce::Line<float>(centerX - 15, centerY, centerX + 15, centerY), 
                           1.0f, 6.0f, 4.0f);
            }
            
            // Labels
            g.setColour(juce::Colours::lightgrey);
            g.setFont(9.0f);
            g.drawText("Spatial Field", bounds.getX() + 5, bounds.getY() + 2, 80, 12, juce::Justification::left);
        }
        
    private:
        float spatialWidth = 0.5f;
        float spatialHeight = 0.5f;
        float spatialDepth = 0.5f;
        bool movementEnabled = false;
    };
    
    SpatialVisualizer spatialVisualizer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonyScapeAudioProcessorEditor)
}; 