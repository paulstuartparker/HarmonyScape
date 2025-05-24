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
    ribbonRateParam = parameters.getRawParameterValue("ribbonRate");
    ribbonSpreadParam = parameters.getRawParameterValue("ribbonSpread");
    ribbonIntensityParam = parameters.getRawParameterValue("ribbonIntensity");
    
    // Get individual ribbon parameter pointers
    for (int i = 0; i < 3; ++i)
    {
        juce::String prefix = "ribbon" + juce::String(i + 1);
        ribbonParams[i].enable = parameters.getRawParameterValue(prefix + "Enable");
        ribbonParams[i].pattern = parameters.getRawParameterValue(prefix + "Pattern");
        ribbonParams[i].rate = parameters.getRawParameterValue(prefix + "Rate");
        ribbonParams[i].offset = parameters.getRawParameterValue(prefix + "Offset");
    }
    
    // Get legacy rhythmic parameter pointers
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
    
    // Create ribbon parameters structure
    RibbonEngine::RibbonParams ribbonParams;
    ribbonParams.enableRibbons = *enableRibbonsParam > 0.5f;
    ribbonParams.activeRibbons = static_cast<int>(*ribbonCountParam);
    ribbonParams.globalRate = *ribbonRateParam;
    ribbonParams.spatialMovement = *ribbonSpreadParam;
    
    // Configure individual ribbons
    for (int i = 0; i < 3; ++i)
    {
        ribbonParams.ribbons[i].enabled = *this->ribbonParams[i].enable > 0.5f;
        ribbonParams.ribbons[i].pattern = static_cast<RibbonEngine::RibbonPattern>(
            static_cast<int>(*this->ribbonParams[i].pattern));
        ribbonParams.ribbons[i].rate = *this->ribbonParams[i].rate;
        ribbonParams.ribbons[i].offset = *this->ribbonParams[i].offset;
        ribbonParams.ribbons[i].intensity = *ribbonIntensityParam;
        ribbonParams.ribbons[i].spatialSpread = *ribbonSpreadParam;
        ribbonParams.ribbons[i].decay = 0.8f; // Good default for musical decay
    }
    
    // Process ribbons if enabled and we have notes to arpeggiate
    juce::MidiBuffer ribbonMidi;
    juce::Array<int> activeRibbonNotes; // Track notes for keyboard display
    
    if (ribbonParams.enableRibbons && !currentChordNotes.isEmpty())
    {
        auto ribbonNotes = ribbonEngine.processChord(currentChordNotes, ribbonParams, 
                                                    buffer.getNumSamples(), 120.0);
        
        // Convert ribbon notes to MIDI events with sample-accurate timing
        for (const auto& ribbonNote : ribbonNotes)
        {
            if (ribbonNote.active)
            {
                double samplesPerSecond = getSampleRate();
                double currentTimeInSamples = ribbonEngine.getCurrentTime();
                double noteStartSample = ribbonNote.startTime - currentTimeInSamples;
                
                // Only add notes that start within this buffer
                if (noteStartSample >= 0 && noteStartSample < buffer.getNumSamples())
                {
                    int samplePosition = static_cast<int>(noteStartSample);
                    
                    // Use appropriate velocity for audible arpeggiation
                    float ribbonVelocity = juce::jlimit(0.4f, 0.9f, ribbonNote.velocity);
                    
                    auto noteOnMsg = juce::MidiMessage::noteOn(1, ribbonNote.midiNote, ribbonVelocity);
                    ribbonMidi.addEvent(noteOnMsg, samplePosition);
                    
                    // Track this note for keyboard visualization
                    activeRibbonNotes.addIfNotAlreadyThere(ribbonNote.midiNote);
                    
                    // Calculate note duration for clean arpeggiation
                    double noteDurationSeconds = ribbonNote.duration / samplesPerSecond;
                    int noteDurationSamples = static_cast<int>(ribbonNote.duration);
                    
                    // Schedule note off within buffer if duration allows
                    int noteOffSample = samplePosition + noteDurationSamples;
                    if (noteOffSample < buffer.getNumSamples())
                    {
                        auto noteOffMsg = juce::MidiMessage::noteOff(1, ribbonNote.midiNote);
                        ribbonMidi.addEvent(noteOffMsg, noteOffSample);
                    }
                }
            }
        }
    }
    
    // Advance ribbon engine time
    ribbonEngine.advanceTime(buffer.getNumSamples());
    
    // Store ribbon notes for keyboard visualization
    spatialEngine.setRibbonNotes(activeRibbonNotes);
    
    // Combine all MIDI sources for audio processing
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
    
    // Add ribbon MIDI
    for (const auto metadata : ribbonMidi)
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