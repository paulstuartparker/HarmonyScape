#pragma once

#include "JuceHeader.h"
#include "ChordEngine/ChordEngine.h"
#include "SpatialEngine/SpatialEngine.h"
#include "RibbonEngine/RibbonEngine.h"

//==============================================================================
/**
 * HarmonyScape audio processor implementing basic chord input and spatial distribution
 */
class HarmonyScapeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    HarmonyScapeAudioProcessor();
    ~HarmonyScapeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Get the active user input notes (for visual display)
    juce::Array<int> getUserInputNotes() const;
    
    // Get the generated notes (for visual display)
    juce::Array<int> getGeneratedNotes() const;
    
    // Get notes that are in release phase but still audible (for keyboard display)
    juce::Array<int> getReleasingNotes() const;
    
    // Update the active voice information from the SpatialEngine
    void updateActiveVoices(const juce::Array<int>& activeVoiceNotes);

    // Parameter layout creation
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        // Existing parameters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "chordDensity", "Chord Density", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "spatialWidth", "Spatial Width", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "waveform", "Waveform", juce::StringArray("Sine", "Saw", "Square", "Triangle"), 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "volume", "Volume", 0.0f, 1.0f, 0.7f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "attack", "Attack", 0.001f, 2.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "decay", "Decay", 0.001f, 2.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "sustain", "Sustain", 0.0f, 1.0f, 0.7f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "release", "Release", 0.001f, 2.0f, 0.2f));

        // Enhanced spatial movement parameters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "movementRate", "Movement Rate", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "movementDepth", "Movement Depth", 0.0f, 1.0f, 0.3f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "height", "Height", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "depth", "Depth", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "enableMovement", "Enable Movement", true));

        // Ribbon parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "enableRibbons", "Enable Ribbons", true));
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            "ribbonCount", "Ribbon Count", 1, 5, 2));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "ribbonRate", "Ribbon Rate", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "ribbonSpread", "Ribbon Spread", 0.0f, 1.0f, 0.6f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "ribbonIntensity", "Ribbon Intensity", 0.0f, 1.0f, 0.8f));
        
        // Individual ribbon controls (for first 3 ribbons in basic UI)
        for (int i = 0; i < 3; ++i)
        {
            juce::String prefix = "ribbon" + juce::String(i + 1);
            params.push_back(std::make_unique<juce::AudioParameterBool>(
                prefix + "Enable", "Ribbon " + juce::String(i + 1) + " Enable", i < 2));
            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                prefix + "Pattern", "Ribbon " + juce::String(i + 1) + " Pattern",
                juce::StringArray("Up", "Down", "Outside", "Inside", "Random", "Cascade", "Spiral"), i % 7));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                prefix + "Rate", "Ribbon " + juce::String(i + 1) + " Rate", 0.0f, 1.0f, 0.5f + i * 0.1f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                prefix + "Offset", "Ribbon " + juce::String(i + 1) + " Offset", 0.0f, 1.0f, i * 0.33f));
        }

        // Legacy rhythmic parameters (keep for backward compatibility)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "swing", "Swing", 0.0f, 1.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "groove", "Groove", 0.0f, 1.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "shimmer", "Shimmer", 0.0f, 1.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "shimmerRate", "Shimmer Rate", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "enableRhythm", "Enable Rhythm", true));

        return { params.begin(), params.end() };
    }

private:
    // Core engines
    ChordEngine chordEngine;
    SpatialEngine spatialEngine;
    RibbonEngine ribbonEngine;
    
    // Value tree for plugin state
    juce::AudioProcessorValueTreeState parameters;
    
    // Core parameters
    std::atomic<float>* chordDensityParam = nullptr;
    std::atomic<float>* spatialWidthParam = nullptr;
    std::atomic<float>* waveformParam = nullptr;
    std::atomic<float>* volumeParam = nullptr;
    
    // ADSR parameters
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    
    // Enhanced spatial parameters
    std::atomic<float>* movementRateParam = nullptr;
    std::atomic<float>* movementDepthParam = nullptr;
    std::atomic<float>* heightParam = nullptr;
    std::atomic<float>* depthParam = nullptr;
    std::atomic<float>* enableMovementParam = nullptr;
    
    // Ribbon parameters
    std::atomic<float>* enableRibbonsParam = nullptr;
    std::atomic<float>* ribbonCountParam = nullptr;
    std::atomic<float>* ribbonRateParam = nullptr;
    std::atomic<float>* ribbonSpreadParam = nullptr;
    std::atomic<float>* ribbonIntensityParam = nullptr;
    
    // Individual ribbon parameters (first 3 ribbons)
    struct RibbonParamSet
    {
        std::atomic<float>* enable = nullptr;
        std::atomic<float>* pattern = nullptr;
        std::atomic<float>* rate = nullptr;
        std::atomic<float>* offset = nullptr;
    };
    std::array<RibbonParamSet, 3> ribbonParams;
    
    // Legacy rhythmic parameters
    std::atomic<float>* swingParam = nullptr;
    std::atomic<float>* grooveParam = nullptr;
    std::atomic<float>* shimmerParam = nullptr;
    std::atomic<float>* shimmerRateParam = nullptr;
    std::atomic<float>* enableRhythmParam = nullptr;
    
    // For keyboard visualization
    juce::Array<int> userInputNotes;          // Notes actively pressed by user
    juce::Array<int> generatedOutputNotes;    // Notes generated by chord engine
    juce::Array<int> releasingNotes;          // Notes in release phase but still sounding
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonyScapeAudioProcessor)
}; 