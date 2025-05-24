#include "SpatialEngine.h"

SpatialEngine::SpatialEngine()
{
    // Initialize all voices as inactive
    for (auto& voice : voices)
    {
        voice.active = false;
        voice.envelopeState = Voice::EnvelopeState::Idle;
        voice.envelopeLevel = 0.0f;
    }
}

SpatialEngine::~SpatialEngine()
{
}

void SpatialEngine::prepare(double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;
}

void SpatialEngine::process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiBuffer, 
                           float spatialWidth, WaveformType waveformType, float volume,
                           const ADSRParams& adsr)
{
    int numSamples = buffer.getNumSamples();
    
    // Clear the buffer first
    buffer.clear();
    
    // Get current time for note timeout checks
    int64_t currentTime = juce::Time::currentTimeMillis();
    
    // Process all MIDI messages at the start of the buffer
    juce::Array<int> activeNotes;
    
    // Track which notes have been stopped in this block
    juce::Array<int> stoppedNotes;
    
    // First pass - collect all note-on and note-off messages
    for (const auto metadata : midiBuffer)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn() && message.getVelocity() > 0) // Ensure non-zero velocity
        {
            int noteNumber = message.getNoteNumber();
            activeNotes.addIfNotAlreadyThere(noteNumber);
        }
        else if (message.isNoteOff() || (message.isNoteOn() && message.getVelocity() == 0)) // Handle both note-off and note-on with 0 velocity
        {
            int noteNumber = message.getNoteNumber();
            stoppedNotes.add(noteNumber);
            // Also remove from active notes if it's there
            activeNotes.removeFirstMatchingValue(noteNumber);
        }
    }
    
    // Explicitly mark user input notes for keyboard display
    userInputNotes.clearQuick();
    userInputNotes.addArray(activeNotes);
    
    // Sort active notes to determine chord structure
    activeNotes.sort();
    
    // Second pass - process note-on messages with chord position context
    for (const auto metadata : midiBuffer)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn() && message.getVelocity() > 0)
        {
            // Find a free voice for this note or reuse one already releasing
            bool noteAssigned = false;
            
            // First try to find a completely free voice
            for (auto& voice : voices)
            {
                if (voice.envelopeState == Voice::EnvelopeState::Idle)
                {
                    int noteNumber = message.getNoteNumber();
                    int chordPosition = activeNotes.indexOf(noteNumber);
                    voice.trigger(noteNumber, 
                                 calculatePosition(noteNumber, chordPosition, spatialWidth),
                                 chordPosition);
                    voice.envelopeLevel = 0.0f; // FIX: Start at 0 instead of 0.01f for proper attack
                    noteAssigned = true;
                    break;
                }
            }
            
            // If no free voice found, try to steal one that's releasing
            if (!noteAssigned)
            {
                for (auto& voice : voices)
                {
                    if (!voice.active && voice.envelopeState == Voice::EnvelopeState::Release && voice.envelopeLevel < 0.01f)
                    {
                        int noteNumber = message.getNoteNumber();
                        int chordPosition = activeNotes.indexOf(noteNumber);
                        voice.trigger(noteNumber, 
                                     calculatePosition(noteNumber, chordPosition, spatialWidth),
                                     chordPosition);
                        voice.envelopeLevel = 0.0f; // FIX: Start at 0 instead of 0.01f
                        noteAssigned = true;
                        break;
                    }
                }
            }
            
            // Last resort - steal the oldest voice if we still couldn't assign
            if (!noteAssigned)
            {
                // Find the voice with the earliest start time
                int64_t oldestTime = currentTime;
                int oldestIndex = -1;
                
                for (int i = 0; i < static_cast<int>(voices.size()); ++i)
                {
                    if (voices[static_cast<size_t>(i)].noteStartTime < oldestTime)
                    {
                        oldestTime = voices[static_cast<size_t>(i)].noteStartTime;
                        oldestIndex = i;
                    }
                }
                
                if (oldestIndex >= 0)
                {
                    int noteNumber = message.getNoteNumber();
                    int chordPosition = activeNotes.indexOf(noteNumber);
                    voices[static_cast<size_t>(oldestIndex)].trigger(noteNumber, 
                                              calculatePosition(noteNumber, chordPosition, spatialWidth),
                                              chordPosition);
                    voices[static_cast<size_t>(oldestIndex)].envelopeLevel = 0.0f; // FIX: Start at 0 instead of 0.01f
                }
            }
        }
    }
    
    // Third pass - CRITICAL: Process note-off messages - force transition to release
    for (int i = 0; i < stoppedNotes.size(); ++i)
    {
        int noteNumber = stoppedNotes[i];
        
        // Release any matching voices - ensure release is triggered
        for (auto& voice : voices)
        {
            if (voice.active && voice.midiNote == noteNumber)
            {
                // CRITICAL: Force transition to release phase
                voice.active = false;
                voice.envelopeState = Voice::EnvelopeState::Release;
                
                // This is critical for sustain->release transition
                // Don't wait for the next processing cycle - begin release immediately
                float releaseRate = voice.envelopeLevel / (adsr.release * static_cast<float>(sampleRate));
                voice.envelopeLevel -= releaseRate * 0.1f; // Start the release phase with an initial decrease
            }
        }
    }
    
    // Update output notes array for keyboard display
    if (!chordOutput.isEmpty())
    {
        generatedNotes.clearQuick();
        for (const auto metadata : chordOutput)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                generatedNotes.addIfNotAlreadyThere(message.getNoteNumber());
            }
        }
    }
    
    // Safety check - stop notes that have been playing too long
    for (auto& voice : voices)
    {
        if (voice.active && voice.hasTimedOut(currentTime))
        {
            voice.forceStop();
        }
    }
    
    // Calculate active voice count for volume scaling
    int activeVoiceCount = 0;
    for (const auto& voice : voices)
    {
        if (voice.active || (voice.envelopeState != Voice::EnvelopeState::Idle && voice.envelopeLevel > 0.01f))
        {
            activeVoiceCount++;
        }
    }
    
    // Render all active voices
    float masterVolume = volume * 0.5f; // Overall volume scaling to prevent clipping
    
    for (auto& voice : voices)
    {
        if (voice.active || (voice.envelopeState != Voice::EnvelopeState::Idle && voice.envelopeLevel > 0.0f))
        {
            // Apply volume scaling based on number of active voices to prevent clipping
            float voiceVolume = masterVolume;
            if (activeVoiceCount > 0 && activeVoiceCount > 1)
            {
                // Scale by square root of voice count for a more natural sound
                voiceVolume = masterVolume / std::sqrt(static_cast<float>(activeVoiceCount));
            }
            
            renderVoice(voice, buffer, 0, numSamples, waveformType, voiceVolume, adsr);
        }
        else
        {
            // Make sure silent voices are properly reset
            voice.envelopeState = Voice::EnvelopeState::Idle;
            voice.envelopeLevel = 0.0f;
        }
    }
}

void SpatialEngine::processEnvelope(Voice& voice, const ADSRParams& adsr, float& envelopeIncrement)
{
    // Calculate envelope increments based on current state
    switch (voice.envelopeState)
    {
        case Voice::EnvelopeState::Attack:
            // Attack phase - ramp up to 1.0
            if (adsr.attack > 0.0f)
                envelopeIncrement = 1.0f / (adsr.attack * static_cast<float>(sampleRate)); // FIX: Removed 0.5f multiplier for proper attack time
            else
                envelopeIncrement = 1.0f / (0.001f * static_cast<float>(sampleRate)); // FIX: Use minimum attack time to avoid clicks
            
            // Transition to decay once we reach peak
            if (voice.envelopeLevel >= 1.0f)
            {
                voice.envelopeLevel = 1.0f;
                voice.envelopeState = Voice::EnvelopeState::Decay;
            }
            break;
            
        case Voice::EnvelopeState::Decay:
            // Decay phase - ramp down to sustain level
            if (adsr.decay > 0.0f)
                envelopeIncrement = (adsr.sustain - 1.0f) / (adsr.decay * static_cast<float>(sampleRate));
            else
                envelopeIncrement = adsr.sustain - 1.0f; // Immediate decay
            
            // Transition to sustain once we reach sustain level
            if (voice.envelopeLevel <= adsr.sustain)
            {
                voice.envelopeLevel = adsr.sustain;
                voice.envelopeState = Voice::EnvelopeState::Sustain;
                envelopeIncrement = 0.0f;
            }
            break;
            
        case Voice::EnvelopeState::Sustain:
            // Sustain phase - hold at sustain level
            envelopeIncrement = 0.0f;
            voice.envelopeLevel = adsr.sustain;
            
            // If the note is no longer active, FORCE transition to release
            // This is CRITICAL for proper ADSR behavior
            if (!voice.active)
            {
                voice.envelopeState = Voice::EnvelopeState::Release;
                // Force immediate decrement to start release
                envelopeIncrement = -0.01f;
            }
            break;
            
        case Voice::EnvelopeState::Release:
            // Release phase - ramp down to zero
            if (adsr.release > 0.001f)
            {
                // Calculate release rate based on current level, but ensure it's never too small
                float releaseRate = std::max(0.001f, voice.envelopeLevel) / (adsr.release * static_cast<float>(sampleRate));
                envelopeIncrement = -releaseRate;
                
                // Ensure we're always decreasing at a minimum rate
                if (std::abs(envelopeIncrement) < 0.0001f)
                    envelopeIncrement = -0.0001f;
            }
            else
            {
                // Fast release if release time is very small
                envelopeIncrement = -0.1f;
            }
            
            // Transition to idle once we reach a very low level
            if (voice.envelopeLevel <= 0.001f)
            {
                voice.envelopeLevel = 0.0f;
                voice.envelopeState = Voice::EnvelopeState::Idle;
                voice.active = false;
                envelopeIncrement = 0.0f;
            }
            break;
            
        case Voice::EnvelopeState::Idle:
            // Idle phase - no sound
            envelopeIncrement = 0.0f;
            voice.envelopeLevel = 0.0f;
            voice.active = false;
            break;
            
        default:
            // Failsafe - if we somehow get an invalid state, force to idle
            voice.envelopeState = Voice::EnvelopeState::Idle;
            voice.envelopeLevel = 0.0f;
            voice.active = false;
            envelopeIncrement = 0.0f;
            break;
    }
}

void SpatialEngine::renderVoice(Voice& voice, juce::AudioBuffer<float>& buffer, 
                               int startSample, int numSamples, 
                               WaveformType waveformType, float masterVolume,
                               const ADSRParams& adsr)
{
    // Check we have stereo output
    if (buffer.getNumChannels() < 2)
        return;
    
    // Get buffer pointers
    float* leftBuffer = buffer.getWritePointer(0, startSample);
    float* rightBuffer = buffer.getWritePointer(1, startSample);
    
    // Convert MIDI note to frequency
    float frequency = 440.0f * std::pow(2.0f, (voice.midiNote - 69) / 12.0f);
    
    // Calculate phase increment
    float phaseIncrement = frequency / static_cast<float>(sampleRate);
    
    // Calculate stereo position (pan)
    float leftGain = std::sqrt(0.5f - voice.position * 0.5f);
    float rightGain = std::sqrt(0.5f + voice.position * 0.5f);
    
    // Render samples
    for (int i = 0; i < numSamples; ++i)
    {
        // Process envelope for this sample
        float envelopeIncrement = 0.0f;
        processEnvelope(voice, adsr, envelopeIncrement);
        
        // Apply envelope increment
        voice.envelopeLevel += envelopeIncrement;
        
        // Clamp envelope level to valid range
        if (voice.envelopeLevel > 1.0f)
            voice.envelopeLevel = 1.0f;
        else if (voice.envelopeLevel < 0.0f)
            voice.envelopeLevel = 0.0f;
        
        // Generate sample based on waveform type and apply envelope
        float sample = generateSample(voice.phase, waveformType) * voice.envelopeLevel * masterVolume;
        
        // Apply stereo positioning
        leftBuffer[i] += sample * leftGain;
        rightBuffer[i] += sample * rightGain;
        
        // Increment phase
        voice.phase += phaseIncrement;
        if (voice.phase >= 1.0f)
            voice.phase -= 1.0f;
    }
    
    // After rendering, check if we should transition to idle
    if (voice.envelopeLevel <= 0.001f && voice.envelopeState == Voice::EnvelopeState::Release)
    {
        voice.envelopeLevel = 0.0f;
        voice.envelopeState = Voice::EnvelopeState::Idle;
        voice.active = false;
    }
}

float SpatialEngine::calculatePosition(int midiNote, int chordPosition, float width)
{
    // More nuanced stereo positioning algorithm:
    // 1. Base position on a combination of note height and chord function
    // 2. Apply slight randomization to avoid perfect alignment
    // 3. Keep important chord tones (root, fifth) more centered
    
    // Note height influence (reduced from original algorithm)
    float noteHeight = (static_cast<float>(midiNote) - 60.0f) / 48.0f; // -0.5 to 0.5 for most notes
    
    // Chord position influence
    float positionFactor;
    
    // Root notes and fifths more centered, extensions wider
    if (chordPosition == 0) // Root
        positionFactor = 0.2f;
    else if (chordPosition % 2 == 0) // Even positions including fifth
        positionFactor = 0.5f;
    else // Odd positions including thirds, sevenths
        positionFactor = 0.8f;
    
    // Slight randomization based on MIDI note number
    float jitter = (midiNote % 5) / 20.0f - 0.1f; // -0.1 to 0.1
    
    // Combine factors with balanced weights
    float position = (noteHeight * 0.3f) + (positionFactor * 0.6f) + jitter;
    
    // Ensure we're in range and apply width
    position = juce::jlimit(-1.0f, 1.0f, position) * width;
    
    return position;
}

float SpatialEngine::generateSample(float phase, WaveformType waveformType)
{
    const float pi = juce::MathConstants<float>::pi;
    
    switch (waveformType)
    {
        case WaveformType::Sine:
            return std::sin(phase * 2.0f * pi) * 0.9f;
            
        case WaveformType::Saw:
            return (2.0f * phase - 1.0f) * 0.7f;
            
        case WaveformType::Square:
            return (phase < 0.5f ? 1.0f : -1.0f) * 0.6f;
            
        case WaveformType::Triangle:
            return (phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase) * 0.9f;
            
        default:
            return 0.0f;
    }
}


juce::Array<int> SpatialEngine::getActiveVoiceNotes() const
{
    // Return a combination of user input notes and generated notes
    juce::Array<int> allNotes;
    allNotes.addArray(userInputNotes);
    allNotes.addArray(generatedNotes);
    return allNotes;
} 
