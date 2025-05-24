#include "RibbonEngine.h"
#include <random>

RibbonEngine::RibbonEngine()
{
    // Initialize default ribbon configurations
    for (int i = 0; i < MAX_RIBBONS; ++i)
    {
        auto& ribbon = ribbonStates[i].sequence;
        ribbon.clear();
        ribbonStates[i].active = false;
        ribbonStates[i].phase = i * 0.2; // Offset each ribbon slightly
    }
}

RibbonEngine::~RibbonEngine()
{
}

void RibbonEngine::prepare(double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;
    currentSamplePosition = 0.0;
}

juce::Array<RibbonEngine::RibbonNote> RibbonEngine::processChord(const juce::Array<int>& chordNotes,
                                                                RibbonParams& ribbonParams,
                                                                int numSamples,
                                                                double hostTempo)
{
    if (!ribbonParams.enableRibbons || chordNotes.isEmpty())
    {
        return {};
    }
    
    // Calculate individual ribbon configurations from master controls
    calculateRibbonConfigurations(ribbonParams);
    
    // Update current chord if it has changed
    if (currentChordNotes != chordNotes)
    {
        setCurrentChord(chordNotes);
    }
    
    // Generate note events for this buffer
    juce::Array<RibbonNote> bufferNotes;
    
    // Process each active ribbon
    int activeCount = juce::jlimit(1, MAX_RIBBONS, ribbonParams.activeRibbons);
    
    for (int i = 0; i < activeCount; ++i)
    {
        if (ribbonParams.ribbons[i].enabled)
        {
            auto ribbonEvents = updateRibbonPhase(i, ribbonParams.ribbons[i], chordNotes, numSamples);
            bufferNotes.addArray(ribbonEvents);
        }
    }
    
    return bufferNotes;
}

void RibbonEngine::advanceTime(int numSamples)
{
    currentSamplePosition += numSamples;
}

juce::Array<RibbonEngine::RibbonNote> RibbonEngine::getActiveNotes(int samplePosition) const
{
    // This method is no longer needed with the new buffer-based approach
    return {};
}

void RibbonEngine::reset()
{
    currentSamplePosition = 0.0;
    
    for (auto& state : ribbonStates)
    {
        state.phase = 0.0;
        state.currentStep = 0;
        state.lastEventTime = 0.0;
        state.active = false;
        state.sequence.clear();
        state.currentlyPlayingNote = -1;
        state.noteStartTime = 0.0;
        state.noteEndTime = 0.0;
    }
}

void RibbonEngine::setCurrentChord(const juce::Array<int>& chordNotes)
{
    currentChordNotes = chordNotes;
    
    // Regenerate sequences for all ribbons when chord changes
    for (int i = 0; i < MAX_RIBBONS; ++i)
    {
        // Use default pattern for now - this could be configurable
        RibbonPattern pattern = static_cast<RibbonPattern>(i % 7);
        ribbonStates[i].sequence = generateArpeggiationSequence(chordNotes, pattern, i);
        ribbonStates[i].currentStep = 0;
    }
}

juce::Array<int> RibbonEngine::generateArpeggiationSequence(const juce::Array<int>& chordNotes,
                                                           RibbonPattern pattern,
                                                           int ribbonIndex)
{
    if (chordNotes.isEmpty())
        return {};
    
    juce::Array<int> sequence;
    auto sortedNotes = chordNotes;
    sortedNotes.sort();
    
    switch (pattern)
    {
        case RibbonPattern::Up:
            // Simple ascending
            sequence = sortedNotes;
            break;
            
        case RibbonPattern::Down:
            // Simple descending
            for (int i = sortedNotes.size() - 1; i >= 0; --i) {
                sequence.add(sortedNotes[i]);
            }
            break;
            
        case RibbonPattern::Outside:
            {
                // From middle outward
                int mid = sortedNotes.size() / 2;
                
                if (sortedNotes.size() % 2 == 1) {
                    sequence.add(sortedNotes[mid]);
                    for (int i = 1; i <= mid; ++i) {
                        if (mid + i < sortedNotes.size()) {
                            sequence.add(sortedNotes[mid + i]);
                        }
                        if (mid - i >= 0) {
                            sequence.add(sortedNotes[mid - i]);
                        }
                    }
                } else {
                    for (int i = 0; i < mid; ++i) {
                        sequence.add(sortedNotes[mid + i]);
                        sequence.add(sortedNotes[mid - 1 - i]);
                    }
                }
            }
            break;
            
        case RibbonPattern::Inside:
            {
                // Edges to center
                int left = 0, right = sortedNotes.size() - 1;
                
                while (left <= right) {
                    sequence.add(sortedNotes[left++]);
                    if (left <= right) {
                        sequence.add(sortedNotes[right--]);
                    }
                }
            }
            break;
            
        case RibbonPattern::Random:
            {
                // Random order but no excessive repeats
                sequence = sortedNotes;
                std::random_device rd;
                std::mt19937 g(rd() + ribbonIndex * 1000);
                std::shuffle(sequence.begin(), sequence.end(), g);
            }
            break;
            
        case RibbonPattern::Cascade:
            {
                // Step through notes with jumps
                for (int i = 0; i < sortedNotes.size(); ++i) {
                    sequence.add(sortedNotes[i]);
                    // Add octave jump occasionally
                    if (i % 2 == 1 && i + 1 < sortedNotes.size()) {
                        sequence.add(sortedNotes[(i + 2) % sortedNotes.size()]);
                    }
                }
            }
            break;
            
        case RibbonPattern::Spiral:
            {
                // Alternating low/high
                juce::Array<int> temp = sortedNotes;
                bool fromLow = true;
                
                while (!temp.isEmpty()) {
                    if (fromLow) {
                        sequence.add(temp.removeAndReturn(0));
                    } else {
                        sequence.add(temp.removeAndReturn(temp.size() - 1));
                    }
                    fromLow = !fromLow;
                }
            }
            break;
    }
    
    // Ensure minimum length but don't over-repeat
    if (sequence.size() < 3 && !sortedNotes.isEmpty()) {
        // Just cycle through the available notes
        while (sequence.size() < 3) {
            sequence.add(sortedNotes[sequence.size() % sortedNotes.size()]);
        }
    }
    
    return sequence;
}

float RibbonEngine::calculateRibbonSpatialPosition(int noteIndex, int totalNotes, 
                                                  const RibbonConfig& config,
                                                  float globalSpatialMovement)
{
    if (totalNotes <= 1)
        return 0.0f;
    
    // Base position spreads notes across stereo field
    float basePosition = (static_cast<float>(noteIndex) / (totalNotes - 1)) * 2.0f - 1.0f; // -1 to 1
    
    // Apply ribbon-specific spatial spread
    basePosition *= config.spatialSpread;
    
    // Add movement based on ribbon phase and global spatial movement
    float movement = std::sin(ribbonStates[0].phase * 2.0f * juce::MathConstants<float>::pi) 
                    * globalSpatialMovement * 0.3f;
    
    // Ensure we stay within bounds
    return juce::jlimit(-1.0f, 1.0f, basePosition + movement);
}

double RibbonEngine::calculateNoteStartTime(int stepIndex, const RibbonConfig& config,
                                           float globalRate, double hostTempo, bool sync)
{
    // Base timing calculation
    double stepDuration;
    
    if (sync && hostTempo > 0.0)
    {
        // Sync to host tempo - use 16th notes as base
        double beatsPerStep = 0.25 * (1.0f - globalRate * 0.8f); // Faster rate = shorter notes
        stepDuration = beatsToSamples(beatsPerStep, hostTempo);
    }
    else
    {
        // Free-running mode
        double rateHz = 1.0 + globalRate * 10.0; // 1-11 Hz range
        stepDuration = sampleRate / rateHz;
    }
    
    // Apply individual ribbon rate scaling
    stepDuration *= (2.0f - config.rate); // rate 0=slow, 1=fast
    
    // Add offset for this ribbon
    double offset = config.offset * stepDuration;
    
    return currentSamplePosition + stepIndex * stepDuration + offset;
}

juce::Array<RibbonEngine::RibbonNote> RibbonEngine::updateRibbonPhase(int ribbonIndex, const RibbonConfig& config,
                                                                   const juce::Array<int>& chordNotes, int numSamples)
{
    juce::Array<RibbonNote> bufferEvents;
    auto& state = ribbonStates[ribbonIndex];
    
    if (state.sequence.isEmpty())
    {
        state.sequence = generateArpeggiationSequence(chordNotes, config.pattern, ribbonIndex);
        if (state.sequence.isEmpty())
            return bufferEvents;
            
        // Initialize timing with per-ribbon offset for cascading
        double ribbonOffsetSamples = config.offset * sampleRate; // Convert seconds to samples
        state.lastEventTime = currentSamplePosition - 100000.0 + ribbonOffsetSamples;
        state.currentlyPlayingNote = -1;
    }
    
    // Calculate step timing - reasonable rates for rhythmic patterns
    double baseRateHz = 0.5 + config.rate * 4.0;  // 0.5-4.5 Hz range (2 seconds to 0.22 seconds per note)
    double stepInterval = sampleRate / baseRateHz; // Samples between each step
    
    // Check if we need to send note-off for currently playing note
    if (state.currentlyPlayingNote >= 0)
    {
        double timeSinceNoteStart = currentSamplePosition - state.noteStartTime;
        if (timeSinceNoteStart >= (state.noteEndTime - state.noteStartTime))
        {
            // Time to turn off the current note
            for (int sampleInBuffer = 0; sampleInBuffer < numSamples; ++sampleInBuffer)
            {
                double currentSample = currentSamplePosition + sampleInBuffer;
                if (currentSample >= state.noteEndTime && state.currentlyPlayingNote >= 0)
                {
                    // Create note-off event
                    RibbonNote noteOff;
                    noteOff.midiNote = state.currentlyPlayingNote;
                    noteOff.ribbon = ribbonIndex;
                    noteOff.bufferSamplePosition = sampleInBuffer;
                    noteOff.isNoteOn = false;
                    noteOff.active = true;
                    bufferEvents.add(noteOff);
                    
                    state.currentlyPlayingNote = -1; // Mark as no note playing
                    break;
                }
            }
        }
    }
    
    // Check each sample in this buffer to see if we should trigger a new note
    for (int sampleInBuffer = 0; sampleInBuffer < numSamples; ++sampleInBuffer)
    {
        double currentSample = currentSamplePosition + sampleInBuffer;
        double timeSinceLastEvent = currentSample - state.lastEventTime;
        
        // Check if it's time for the next step AND no note is currently playing
        if (timeSinceLastEvent >= stepInterval && state.currentlyPlayingNote < 0)
        {
            int stepsPerCycle = state.sequence.size();
            if (stepsPerCycle > 0)
            {
                // Move to next step in sequence
                state.currentStep = (state.currentStep + 1) % stepsPerCycle;
                state.lastEventTime = currentSample;
                
                // Apply swing timing
                int swingDelay = 0;
                if (config.swing > 0.0f && (state.currentStep % 2 == 1))
                {
                    swingDelay = static_cast<int>(stepInterval * config.swing * 0.2); // Max 20% swing
                }
                
                // Calculate when this note should actually trigger
                int triggerSample = sampleInBuffer + swingDelay;
                if (triggerSample < numSamples)
                {
                    // Create note-on event
                    RibbonNote noteOn;
                    noteOn.midiNote = state.sequence[state.currentStep];
                    noteOn.ribbon = ribbonIndex;
                    noteOn.bufferSamplePosition = triggerSample;
                    noteOn.isNoteOn = true;
                    noteOn.stepIndex = state.currentStep;
                    
                    // Calculate velocity with dynamics
                    float baseVelocity = config.intensity;
                    float accentFactor = (state.currentStep == 0) ? 1.3f : 1.0f; // Accent on downbeat
                    float ribbonCharacter = 0.7f + (ribbonIndex * 0.06f); // Each ribbon slightly different
                    
                    noteOn.velocity = juce::jlimit(0.5f, 0.95f, 
                        baseVelocity * accentFactor * ribbonCharacter);
                    
                    // Calculate note duration - USING PROPER GATE!
                    double noteDurationSamples = stepInterval * config.gate; // Gate is 10-50% of interval
                    noteOn.duration = noteDurationSamples / sampleRate; // Convert to seconds for display
                    
                    // Track this note as currently playing
                    state.currentlyPlayingNote = noteOn.midiNote;
                    state.noteStartTime = currentSample;
                    state.noteEndTime = currentSample + noteDurationSamples;
                    
                    // Calculate spatial position for this ribbon
                    float ribbonSpatialBase = (static_cast<float>(ribbonIndex) / (MAX_RIBBONS - 1)) * 2.0f - 1.0f;
                    noteOn.spatialPosition = ribbonSpatialBase * 0.8f; // Each ribbon owns 80% of its space
                    
                    noteOn.active = true;
                    bufferEvents.add(noteOn);
                }
            }
        }
    }
    
    return bufferEvents;
}

double RibbonEngine::beatsToSamples(double beats, double bpm) const
{
    if (bpm <= 0.0) return 0.0;
    double beatsPerSecond = bpm / 60.0;
    double secondsPerBeat = 1.0 / beatsPerSecond;
    return beats * secondsPerBeat * sampleRate;
}

double RibbonEngine::samplesToBeats(double samples, double bpm) const
{
    if (bpm <= 0.0) return 0.0;
    double beatsPerSecond = bpm / 60.0;
    double secondsPerBeat = 1.0 / beatsPerSecond;
    double seconds = samples / sampleRate;
    return seconds / secondsPerBeat;
}

void RibbonEngine::calculateRibbonConfigurations(RibbonParams& params)
{
    // Use variation parameter as seed for consistent but different ribbon behaviors
    int seed = static_cast<int>(params.variation * 1000.0f);
    std::srand(seed);
    
    for (int i = 0; i < MAX_RIBBONS; ++i)
    {
        auto& ribbon = params.ribbons[i];
        
        // Enable based on activeRibbons count
        ribbon.enabled = (i < params.activeRibbons);
        
        if (!ribbon.enabled) continue;
        
        // PULSE affects rate - slower for clarity but still rhythmic
        float baseRate = 0.15f + params.pulse * 0.5f;  // 0.15 to 0.65 range
        
        // VARIATION creates differences between ribbons
        float variationFactor = 0.8f + (params.variation * 0.8f) + (i * 0.2f);
        ribbon.rate = juce::jlimit(0.1f, 1.0f, baseRate * variationFactor);
        
        // WOBBLE creates staggering between ribbons for cascading
        float baseOffset = i * 0.5f; // Base 0.5 second separation between ribbons
        float wobbleOffset = params.wobble * (i * 1.0f); // Up to 1 second extra per ribbon
        ribbon.offset = baseOffset + wobbleOffset;  // Ribbons start 0.5-3.5 seconds apart
        
        // Spatial spread - each ribbon has its own position
        ribbon.spatialSpread = 0.3f + (i * 0.15f) + (params.wobble * 0.3f);
        
        // SWING is applied globally to all ribbons
        ribbon.swing = params.swing;
        
        // SHIMMER affects intensity
        float shimmerVariation = 0.8f + (static_cast<float>(std::rand()) / RAND_MAX * 0.4f) * params.shimmer;
        ribbon.intensity = juce::jlimit(0.6f, 1.0f, shimmerVariation);
        ribbon.decay = 0.9f; // High decay to maintain note clarity
        
        // GATE - MUCH SHORTER for distinct notes! This is KEY!
        // Gate should be 10-50% of the interval for clear rhythmic patterns
        ribbon.gate = 0.1f + params.pulse * 0.4f;  // 0.1 to 0.5 range (10-50% of interval)
        
        // PATTERN selection - ensure each ribbon gets different patterns
        int patternIndex = (i * 2 + static_cast<int>(params.variation * 7.0f)) % 7;
        ribbon.pattern = static_cast<RibbonPattern>(patternIndex);
    }
} 