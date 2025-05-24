#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HarmonyScapeAudioProcessor::HarmonyScapeAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    // Get core parameter pointers
    chordDensityParam = parameters.getRawParameterValue("chordDensity");
    spatialWidthParam = parameters.getRawParameterValue("spatialWidth");
    waveformParam = parameters.getRawParameterValue("waveform");
    volumeParam = parameters.getRawParameterValue("volume");
    attackParam = parameters.getRawParameterValue("attack");
    decayParam = parameters.getRawParameterValue("decay");
    sustainParam = parameters.getRawParameterValue("sustain");
    releaseParam = parameters.getRawParameterValue("release");

    // Get enhanced spatial parameter pointers
    movementRateParam = parameters.getRawParameterValue("movementRate");
    movementDepthParam = parameters.getRawParameterValue("movementDepth");
    heightParam = parameters.getRawParameterValue("height");
    depthParam = parameters.getRawParameterValue("depth");
    enableMovementParam = parameters.getRawParameterValue("enableMovement");
    
    // Get ribbon parameter pointers
    enableRibbonsParam = parameters.getRawParameterValue("enableRibbons");
    ribbonCountParam = parameters.getRawParameterValue("ribbonCount");
    pulseParam = parameters.getRawParameterValue("pulse");
    variationParam = parameters.getRawParameterValue("variation");
    wobbleParam = parameters.getRawParameterValue("wobble");
    swingParam = parameters.getRawParameterValue("swing");
    shimmerParam = parameters.getRawParameterValue("shimmer");
    
    // Get legacy rhythmic parameter pointers
    grooveParam = parameters.getRawParameterValue("groove");
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
    // Initialize all engines with current sample rate
    chordEngine.prepare(sampleRate, samplesPerBlock);
    spatialEngine.prepare(sampleRate, samplesPerBlock);
    ribbonEngine.prepare(sampleRate, samplesPerBlock);
}

void HarmonyScapeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    ribbonEngine.reset();
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
    
    // Process MIDI messages through chord engine with contextual awareness
    auto chordOutput = chordEngine.processMidi(midiMessages, *chordDensityParam);
    
    // Store the chord output in the spatial engine for visualization
    spatialEngine.setChordOutput(chordOutput);
    
    // Get current chord notes for ribbon processing (from both user input and generated harmony)
    juce::Array<int> currentChordNotes;
    
    // Add user input notes
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            currentChordNotes.addIfNotAlreadyThere(message.getNoteNumber());
        }
    }
    
    // Add generated chord harmony notes
    for (const auto metadata : chordOutput)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            currentChordNotes.addIfNotAlreadyThere(message.getNoteNumber());
        }
    }
    
    // Create ribbon parameters structure with NEW MASTER CONTROLS
    RibbonEngine::RibbonParams ribbonParams;
    ribbonParams.enableRibbons = *enableRibbonsParam > 0.5f;
    ribbonParams.activeRibbons = static_cast<int>(*ribbonCountParam);
    ribbonParams.pulse = *pulseParam;
    ribbonParams.variation = *variationParam;
    ribbonParams.wobble = *wobbleParam;
    ribbonParams.swing = *swingParam;
    ribbonParams.shimmer = *shimmerParam;
    
    // Process ribbons if enabled and we have notes to arpeggiate
    juce::MidiBuffer ribbonMidi;
    juce::Array<int> activeRibbonNotes; // Track notes for keyboard display
    
    if (ribbonParams.enableRibbons && !currentChordNotes.isEmpty())
    {
        auto ribbonNotes = ribbonEngine.processChord(currentChordNotes, ribbonParams, 
                                                    buffer.getNumSamples(), 120.0);
        
        // Convert ribbon notes to MIDI events - NOW WITH CORRECT TIMING!
        for (const auto& ribbonNote : ribbonNotes)
        {
            if (ribbonNote.active)
            {
                // Use the buffer-relative sample position directly
                int samplePosition = ribbonNote.bufferSamplePosition;
                
                // Ensure sample position is within buffer bounds
                samplePosition = juce::jlimit(0, buffer.getNumSamples() - 1, samplePosition);
                
                if (ribbonNote.isNoteOn)
                {
                    // Note ON event - BOOST VELOCITY for prominence!
                    float ribbonVelocity = juce::jlimit(0.7f, 1.0f, ribbonNote.velocity * 1.2f);
                    auto noteOnMsg = juce::MidiMessage::noteOn(1, ribbonNote.midiNote, ribbonVelocity);
                    ribbonMidi.addEvent(noteOnMsg, samplePosition);
                    
                    // Track this note for keyboard visualization
                    activeRibbonNotes.addIfNotAlreadyThere(ribbonNote.midiNote);
                }
                else
                {
                    // Note OFF event
                    auto noteOffMsg = juce::MidiMessage::noteOff(1, ribbonNote.midiNote);
                    ribbonMidi.addEvent(noteOffMsg, samplePosition);
                }
            }
        }
    }
    
    // Advance ribbon engine time
    ribbonEngine.advanceTime(buffer.getNumSamples());
    
    // Store ribbon notes for keyboard visualization
    spatialEngine.setRibbonNotes(activeRibbonNotes);
    
    // Combine MIDI sources for audio processing - RIBBONS REPLACE HARMONY!
    juce::MidiBuffer combinedMidi;
    
    if (ribbonParams.enableRibbons && !ribbonMidi.isEmpty())
    {
        // RIBBONS ARE ENABLED - they replace the sustained harmony!
        
        // Add user input MIDI (original notes, but reduce velocity for base)
        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                // Reduce user input to 30% when ribbons are active (subtle base)
                auto reducedMsg = juce::MidiMessage::noteOn(message.getChannel(), 
                    message.getNoteNumber(), message.getVelocity() * 0.3f);
                combinedMidi.addEvent(reducedMsg, metadata.samplePosition);
            }
            else
            {
                combinedMidi.addEvent(message, metadata.samplePosition);
            }
        }
        
        // SKIP the generated chord harmony completely - ribbons replace it!
        
        // Add ribbon MIDI at FULL VOLUME - these are the stars!
        for (const auto metadata : ribbonMidi)
        {
            combinedMidi.addEvent(metadata.getMessage(), metadata.samplePosition);
        }
    }
    else
    {
        // RIBBONS DISABLED - use normal harmony processing
        
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
        
        // No ribbons to add
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
    
    // Create enhanced spatial parameters
    SpatialEngine::SpatialParams spatialParams;
    spatialParams.movementRate = *movementRateParam;
    spatialParams.movementDepth = *movementDepthParam;
    spatialParams.height = *heightParam;
    spatialParams.depth = *depthParam;
    spatialParams.enableMovement = *enableMovementParam > 0.5f;
    
    // Create rhythm parameters
    SpatialEngine::RhythmParams rhythmParams;
    rhythmParams.swing = *swingParam;
    rhythmParams.groove = *grooveParam;
    rhythmParams.shimmer = *shimmerParam;
    rhythmParams.shimmerRate = *shimmerRateParam;
    rhythmParams.enableRhythm = *enableRhythmParam > 0.5f;
    
    // Apply spatial processing with enhanced parameters
    spatialEngine.process(buffer, combinedMidi, *spatialWidthParam, waveformType, 
                         *volumeParam, adsr, spatialParams, rhythmParams);
    
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

// Get ribbon notes for keyboard visualization
juce::Array<int> HarmonyScapeAudioProcessor::getRibbonNotes() const
{
    return spatialEngine.getRibbonNotes();
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