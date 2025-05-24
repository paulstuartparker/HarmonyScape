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
        
        // Release any matching voices - ensure release is triggered for ALL matching voices
        for (auto& voice : voices)
        {
            if (voice.midiNote == noteNumber && voice.envelopeState != Voice::EnvelopeState::Idle)
            {
                // CRITICAL: Force transition to release phase immediately
                voice.active = false;
                
                // Only transition if not already in release
                if (voice.envelopeState != Voice::EnvelopeState::Release)
                {
                    voice.envelopeState = Voice::EnvelopeState::Release;
                }
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
            if (adsr.attack > 0.0f)
                envelopeIncrement = 1.0f / (adsr.attack * static_cast<float>(sampleRate));
            else
                envelopeIncrement = 1.0f / (0.001f * static_cast<float>(sampleRate));
            
            if (voice.envelopeLevel >= 1.0f)
            {
                voice.envelopeLevel = 1.0f;
                voice.envelopeState = Voice::EnvelopeState::Decay;
            }
            break;
            
        case Voice::EnvelopeState::Decay:
            if (adsr.decay > 0.0f)
                envelopeIncrement = (adsr.sustain - 1.0f) / (adsr.decay * static_cast<float>(sampleRate));
            else
                envelopeIncrement = adsr.sustain - 1.0f;
            
            if (voice.envelopeLevel <= adsr.sustain)
            {
                voice.envelopeLevel = adsr.sustain;
                voice.envelopeState = Voice::EnvelopeState::Sustain;
                envelopeIncrement = 0.0f;
            }
            break;
            
        case Voice::EnvelopeState::Sustain:
            envelopeIncrement = 0.0f;
            voice.envelopeLevel = adsr.sustain;
            
            // CRITICAL: Immediately transition to release when note is released
            if (!voice.active)
            {
                voice.envelopeState = Voice::EnvelopeState::Release;
            }
            break;
            
        case Voice::EnvelopeState::Release:
            if (adsr.release > 0.001f)
            {
                // Smooth release calculation
                float releaseRate = voice.envelopeLevel / (adsr.release * static_cast<float>(sampleRate));
                envelopeIncrement = -std::max(0.0001f, releaseRate); // Ensure minimum release rate
            }
            else
            {
                envelopeIncrement = -0.01f; // Fast release for very short release times
            }
            
            // Transition to idle when envelope reaches zero
            if (voice.envelopeLevel <= 0.001f)
            {
                voice.envelopeLevel = 0.0f;
                voice.envelopeState = Voice::EnvelopeState::Idle;
                voice.active = false;
                envelopeIncrement = 0.0f;
            }
            break;
            
        case Voice::EnvelopeState::Idle:
            envelopeIncrement = 0.0f;
            voice.envelopeLevel = 0.0f;
            voice.active = false;
            break;
            
        default:
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
    float baseFrequency = 440.0f * std::pow(2.0f, (voice.midiNote - 69) / 12.0f);
    
    // Add subtle pitch modulation for liveliness
    float lfoPhase = std::fmod(voice.noteStartTime * 0.001f + voice.chordPosition * 0.3f, 1.0f);
    float pitchModAmount = 0.002f + (voice.chordPosition * 0.001f); // More modulation for higher chord tones
    
    // Calculate stereo position (pan)
    float leftGain = std::sqrt(0.5f - voice.position * 0.5f);
    float rightGain = std::sqrt(0.5f + voice.position * 0.5f);
    
    // Dynamic filter cutoff based on envelope and note pitch
    // Higher cutoff for brighter sound, especially in low notes
    float baseCutoff = 0.4f + (voice.midiNote / 127.0f) * 0.4f; // Brighter overall
    
    // High-pass filter frequency to reduce muddiness
    float highpassFreq = 80.0f; // 80Hz high-pass
    if (voice.midiNote < 48) // For low notes
        highpassFreq = 120.0f; // More aggressive high-pass
    
    // Calculate high-pass filter coefficient
    float highpassCutoff = highpassFreq / static_cast<float>(sampleRate);
    float highpassCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * highpassCutoff);
    
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
        
        // Apply subtle pitch modulation (vibrato)
        lfoPhase += 0.0001f; // ~4.4Hz at 44.1kHz
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float pitchMod = 1.0f + std::sin(lfoPhase * 2.0f * juce::MathConstants<float>::pi) * pitchModAmount;
        float frequency = baseFrequency * pitchMod;
        
        // Calculate phase increment with modulation
        float phaseIncrement = frequency / static_cast<float>(sampleRate);
        
        // Generate sample based on waveform type and apply envelope
        float sample = generateSample(voice.phase, waveformType) * voice.envelopeLevel * masterVolume;
        
        // Dynamic filter that opens with envelope (low-pass)
        float dynamicCutoff = baseCutoff + (voice.envelopeLevel * 0.2f); // Less envelope modulation
        
        // Resonant low-pass filter for character
        float resonance = 0.3f + (voice.chordPosition * 0.05f); // Less resonance for cleaner sound
        
        // Simple resonant filter implementation
        voice.filterState = voice.filterState * dynamicCutoff + sample * (1.0f - dynamicCutoff);
        float filteredSample = voice.filterState + (sample - voice.filterState) * resonance;
        
        // Apply high-pass filter to reduce muddiness
        voice.highpassState += (filteredSample - voice.highpassState) * highpassCoeff;
        filteredSample = filteredSample - voice.highpassState;
        
        // Soft saturation for warmth (less aggressive)
        filteredSample = std::tanh(filteredSample * 0.8f) * 0.95f;
        
        // Apply stereo positioning with subtle movement
        float panLfo = std::sin((lfoPhase * 0.3f + voice.chordPosition * 0.2f) * 2.0f * juce::MathConstants<float>::pi);
        float dynamicPan = voice.position + (panLfo * 0.05f); // Subtle stereo movement
        dynamicPan = juce::jlimit(-1.0f, 1.0f, dynamicPan);
        
        float currentLeftGain = std::sqrt(0.5f - dynamicPan * 0.5f);
        float currentRightGain = std::sqrt(0.5f + dynamicPan * 0.5f);
        
        leftBuffer[i] += filteredSample * currentLeftGain;
        rightBuffer[i] += filteredSample * currentRightGain;
        
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
            // Add some subtle harmonics for warmth
            return (std::sin(phase * 2.0f * pi) * 0.8f +
                    std::sin(phase * 4.0f * pi) * 0.1f +  // 2nd harmonic
                    std::sin(phase * 6.0f * pi) * 0.05f) * 0.9f; // 3rd harmonic
            
        case WaveformType::Saw:
        {
            // Band-limited saw wave to reduce aliasing
            float saw = 0.0f;
            for (int harmonic = 1; harmonic <= 8; ++harmonic)
            {
                float amplitude = 1.0f / static_cast<float>(harmonic);
                saw += std::sin(phase * 2.0f * pi * harmonic) * amplitude;
            }
            return saw * 0.5f;
        }
            
        case WaveformType::Square:
        {
            // Band-limited square wave with only odd harmonics
            float square = 0.0f;
            for (int harmonic = 1; harmonic <= 7; harmonic += 2)
            {
                float amplitude = 1.0f / static_cast<float>(harmonic);
                square += std::sin(phase * 2.0f * pi * harmonic) * amplitude;
            }
            return square * 0.6f;
        }
            
        case WaveformType::Triangle:
        {
            // Enhanced triangle with slight detuning for chorus effect
            float triangle1 = (phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase);
            float phase2 = std::fmod(phase + 0.002f, 1.0f); // Slight detune
            float triangle2 = (phase2 < 0.5f ? 4.0f * phase2 - 1.0f : 3.0f - 4.0f * phase2);
            return (triangle1 * 0.7f + triangle2 * 0.3f) * 0.85f;
        }
            
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
