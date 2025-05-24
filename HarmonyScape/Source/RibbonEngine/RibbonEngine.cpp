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
                                                                const RibbonParams& ribbonParams,
                                                                int numSamples,
                                                                double hostTempo)
{
    if (!ribbonParams.enableRibbons || chordNotes.isEmpty())
    {
        return {};
    }
    
    // Clear old scheduled notes that have passed
    scheduledNotes.removeIf([this](const RibbonNote& note) {
        return note.startTime + note.duration < currentSamplePosition;
    });
    
    // Update current chord if it has changed
    if (currentChordNotes != chordNotes)
    {
        setCurrentChord(chordNotes);
    }
    
    // Process each active ribbon
    int activeCount = juce::jlimit(1, MAX_RIBBONS, ribbonParams.activeRibbons);
    
    for (int i = 0; i < activeCount; ++i)
    {
        if (ribbonParams.ribbons[i].enabled)
        {
            updateRibbonPhase(i, ribbonParams.ribbons[i], chordNotes, numSamples);
        }
    }
    
    // Return notes that should be active in this block
    juce::Array<RibbonNote> activeNotes;
    for (const auto& note : scheduledNotes)
    {
        if (note.startTime <= currentSamplePosition + numSamples &&
            note.startTime + note.duration >= currentSamplePosition)
        {
            activeNotes.add(note);
        }
    }
    
    return activeNotes;
}

void RibbonEngine::advanceTime(int numSamples)
{
    currentSamplePosition += numSamples;
}

juce::Array<RibbonEngine::RibbonNote> RibbonEngine::getActiveNotes(int samplePosition) const
{
    juce::Array<RibbonNote> activeNotes;
    
    for (const auto& note : scheduledNotes)
    {
        if (note.startTime <= currentSamplePosition + samplePosition &&
            note.startTime + note.duration >= currentSamplePosition + samplePosition)
        {
            activeNotes.add(note);
        }
    }
    
    return activeNotes;
}

void RibbonEngine::reset()
{
    currentSamplePosition = 0.0;
    scheduledNotes.clear();
    
    for (auto& state : ribbonStates)
    {
        state.phase = 0.0;
        state.currentStep = 0;
        state.lastEventTime = 0.0;
        state.active = false;
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
    
    switch (pattern)
    {
        case RibbonPattern::Up:
            sortedNotes.sort();
            sequence = sortedNotes;
            break;
            
        case RibbonPattern::Down:
            sortedNotes.sort();
            for (int i = sortedNotes.size() - 1; i >= 0; --i)
                sequence.add(sortedNotes[i]);
            break;
            
        case RibbonPattern::Outside:
            {
                sortedNotes.sort();
                int mid = sortedNotes.size() / 2;
                
                // Start from middle and work outward
                if (sortedNotes.size() % 2 == 1)
                {
                    sequence.add(sortedNotes[mid]);
                    for (int i = 1; i <= mid; ++i)
                    {
                        if (mid + i < sortedNotes.size())
                            sequence.add(sortedNotes[mid + i]);
                        if (mid - i >= 0)
                            sequence.add(sortedNotes[mid - i]);
                    }
                }
                else
                {
                    for (int i = 0; i < mid; ++i)
                    {
                        sequence.add(sortedNotes[mid + i]);
                        sequence.add(sortedNotes[mid - 1 - i]);
                    }
                }
            }
            break;
            
        case RibbonPattern::Inside:
            {
                sortedNotes.sort();
                int left = 0, right = sortedNotes.size() - 1;
                bool addFromLeft = true;
                
                while (left <= right)
                {
                    if (addFromLeft)
                        sequence.add(sortedNotes[left++]);
                    else
                        sequence.add(sortedNotes[right--]);
                    addFromLeft = !addFromLeft;
                }
            }
            break;
            
        case RibbonPattern::Random:
            {
                sequence = sortedNotes;
                std::random_device rd;
                std::mt19937 g(rd() + ribbonIndex); // Add ribbon index for consistency
                std::shuffle(sequence.begin(), sequence.end(), g);
            }
            break;
            
        case RibbonPattern::Cascade:
            {
                sortedNotes.sort();
                // Create overlapping waves
                for (int wave = 0; wave < 2; ++wave)
                {
                    for (int i = 0; i < sortedNotes.size(); ++i)
                    {
                        if ((i + wave) % 2 == 0) // Offset for each wave
                            sequence.add(sortedNotes[i]);
                    }
                }
            }
            break;
            
        case RibbonPattern::Spiral:
            {
                sortedNotes.sort();
                // Alternate between low and high, spiraling inward
                juce::Array<int> temp = sortedNotes;
                bool fromLow = (ribbonIndex % 2 == 0);
                
                while (!temp.isEmpty())
                {
                    if (fromLow)
                    {
                        sequence.add(temp.removeAndReturn(0));
                    }
                    else
                    {
                        sequence.add(temp.removeAndReturn(temp.size() - 1));
                    }
                    fromLow = !fromLow;
                }
            }
            break;
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

void RibbonEngine::updateRibbonPhase(int ribbonIndex, const RibbonConfig& config,
                                    const juce::Array<int>& chordNotes, int numSamples)
{
    auto& state = ribbonStates[ribbonIndex];
    
    if (state.sequence.isEmpty())
    {
        state.sequence = generateArpeggiationSequence(chordNotes, config.pattern, ribbonIndex);
        if (state.sequence.isEmpty())
            return;
    }
    
    // Calculate how fast this ribbon should progress
    double rateMultiplier = config.rate * 2.0; // 0-2x speed
    double phaseIncrement = (static_cast<double>(numSamples) / sampleRate) * rateMultiplier;
    
    state.phase += phaseIncrement;
    
    // Check if we should trigger new notes
    int stepsPerCycle = state.sequence.size();
    if (stepsPerCycle > 0)
    {
        double stepPhase = state.phase * stepsPerCycle;
        int currentStepInPhase = static_cast<int>(stepPhase) % stepsPerCycle;
        
        // If we've moved to a new step, schedule a note
        if (currentStepInPhase != state.currentStep || 
            (state.phase > state.lastEventTime + 1.0 / stepsPerCycle))
        {
            state.currentStep = currentStepInPhase;
            state.lastEventTime = state.phase;
            
            // Create a new ribbon note
            RibbonNote newNote;
            newNote.midiNote = state.sequence[currentStepInPhase];
            newNote.ribbon = ribbonIndex;
            newNote.startTime = currentSamplePosition;
            newNote.duration = sampleRate / (4.0 + config.rate * 8.0); // Variable duration
            newNote.velocity = config.intensity;
            newNote.spatialPosition = calculateRibbonSpatialPosition(
                currentStepInPhase, stepsPerCycle, config, 0.5f);
            newNote.active = true;
            newNote.stepIndex = currentStepInPhase;
            
            // Apply decay based on step position
            float decayFactor = std::pow(config.decay, currentStepInPhase);
            newNote.velocity *= decayFactor;
            
            scheduledNotes.add(newNote);
        }
    }
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