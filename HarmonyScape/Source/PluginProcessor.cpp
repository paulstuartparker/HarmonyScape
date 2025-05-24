#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HarmonyScapeAudioProcessor::HarmonyScapeAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    // Get parameter pointers
    chordDensityParam = parameters.getRawParameterValue("chordDensity");
    spatialWidthParam = parameters.getRawParameterValue("spatialWidth");
    waveformParam = parameters.getRawParameterValue("waveform");
    volumeParam = parameters.getRawParameterValue("volume");
    attackParam = parameters.getRawParameterValue("attack");
    decayParam = parameters.getRawParameterValue("decay");
    sustainParam = parameters.getRawParameterValue("sustain");
    releaseParam = parameters.getRawParameterValue("release");

    // Get new parameter pointers
    movementRateParam = parameters.getRawParameterValue("movementRate");
    movementDepthParam = parameters.getRawParameterValue("movementDepth");
    heightParam = parameters.getRawParameterValue("height");
    depthParam = parameters.getRawParameterValue("depth");
    enableMovementParam = parameters.getRawParameterValue("enableMovement");
    swingParam = parameters.getRawParameterValue("swing");
    grooveParam = parameters.getRawParameterValue("groove");
    shimmerParam = parameters.getRawParameterValue("shimmer");
    shimmerRateParam = parameters.getRawParameterValue("shimmerRate");
    enableRhythmParam = parameters.getRawParameterValue("enableRhythm");
}

HarmonyScapeAudioProcessor::~HarmonyScapeAudioProcessor()
{
}

//==============================================================================
const juce::String HarmonyScapeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HarmonyScapeAudioProcessor::acceptsMidi() const
{
    return true;
}

bool HarmonyScapeAudioProcessor::producesMidi() const
{
    return true;
}

bool HarmonyScapeAudioProcessor::isMidiEffect() const
{
    return false;
}

double HarmonyScapeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HarmonyScapeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HarmonyScapeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HarmonyScapeAudioProcessor::setCurrentProgram (int index)
{
    // Unused
    juce::ignoreUnused(index);
}

const juce::String HarmonyScapeAudioProcessor::getProgramName (int index)
{
    // Unused
    juce::ignoreUnused(index);
    return {};
}

void HarmonyScapeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    // Unused
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void HarmonyScapeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize engines with current sample rate
    chordEngine.prepare(sampleRate, samplesPerBlock);
    spatialEngine.prepare(sampleRate, samplesPerBlock);
}

void HarmonyScapeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool HarmonyScapeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // We only support stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void HarmonyScapeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear output buffer
    buffer.clear();
    
    // Process MIDI messages through chord engine
    auto chordOutput = chordEngine.processMidi(midiMessages, *chordDensityParam);
    
    // Store the chord output in the spatial engine for visualization
    spatialEngine.setChordOutput(chordOutput);
    
    // Combine user input MIDI with generated chord output
    juce::MidiBuffer combinedMidi;
    
    // Add user input MIDI
    for (const auto metadata : midiMessages)
    {
        combinedMidi.addEvent(metadata.getMessage(), metadata.samplePosition);
    }
    
    // Add generated chord harmony
    for (const auto metadata : chordOutput)
    {
        combinedMidi.addEvent(metadata.getMessage(), metadata.samplePosition);
    }
    
    // Convert waveform parameter to enum
    auto waveformType = static_cast<SpatialEngine::WaveformType>(
        static_cast<int>(*waveformParam));
    
    // Create ADSR parameters
    SpatialEngine::ADSRParams adsr = {
        *attackParam,
        *decayParam,
        *sustainParam,
        *releaseParam
    };
    
    // Apply spatial processing with the original method signature first
    spatialEngine.process(buffer, combinedMidi, *spatialWidthParam, waveformType, 
                         *volumeParam, adsr);
    
    // Get currently sounding notes from the spatial engine
    updateActiveVoices(spatialEngine.getActiveVoiceNotes());
}

//==============================================================================
bool HarmonyScapeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* HarmonyScapeAudioProcessor::createEditor()
{
    return new HarmonyScapeAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void HarmonyScapeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HarmonyScapeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmonyScapeAudioProcessor();
}

// Add these methods for keyboard visualization
juce::Array<int> HarmonyScapeAudioProcessor::getUserInputNotes() const
{
    // Use the spatial engine's tracking
    return spatialEngine.getUserInputNotes();
}

juce::Array<int> HarmonyScapeAudioProcessor::getGeneratedNotes() const
{
    // Use the spatial engine's tracking
    return spatialEngine.getGeneratedNotes();
}

// Get notes in release phase that are still sounding
juce::Array<int> HarmonyScapeAudioProcessor::getReleasingNotes() const
{
    return releasingNotes;
}

// Update information about which notes are currently audible for visualization
void HarmonyScapeAudioProcessor::updateActiveVoices(const juce::Array<int>& activeVoiceNotes)
{
    // Clear the releasing notes array
    releasingNotes.clearQuick();
    
    // For each active voice, check if it's a releasing note (not in userInputNotes or generatedOutputNotes)
    for (const auto& noteNumber : activeVoiceNotes)
    {
        if (!spatialEngine.getUserInputNotes().contains(noteNumber) && !spatialEngine.getGeneratedNotes().contains(noteNumber))
        {
            releasingNotes.addIfNotAlreadyThere(noteNumber);
        }
    }
} 