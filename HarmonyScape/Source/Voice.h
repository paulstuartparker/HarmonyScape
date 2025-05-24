#pragma once

#include "JuceHeader.h"

// ADSR parameters structure
struct ADSRParams
{
    float attack = 0.01f;   // In seconds - Quick attack for responsiveness
    float decay = 0.1f;     // In seconds - Quick decay
    float sustain = 0.5f;   // Level (0.0-1.0) - Medium sustain
    float release = 0.2f;   // In seconds - Quick release to avoid muddiness
};

// Voice structure for polyphonic synth
struct Voice
{
    enum class EnvelopeState
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };
    
    bool active = false;                  // Is this voice playing a note?
    int midiNote = 60;                    // MIDI note number
    float phase = 0.0f;                   // Oscillator phase (0.0-1.0)
    float position = 0.0f;                // Stereo position (-1.0 to 1.0)
    int chordPosition = 0;                // Position in chord (0 = root)
    EnvelopeState envelopeState = EnvelopeState::Idle;  // Current ADSR state
    float envelopeLevel = 0.0f;           // Current envelope level (0.0-1.0)
    int64_t noteStartTime = 0;            // When the note was triggered
    float filterState = 0.0f;             // State for the resonant filter
    float highpassState = 0.0f;           // State for the high-pass filter
    
    // Trigger a new note
    void trigger(int newNote, float newPosition, int newChordPosition)
    {
        midiNote = newNote;
        position = newPosition;
        chordPosition = newChordPosition;
        active = true;
        envelopeState = EnvelopeState::Attack;
        envelopeLevel = 0.0f;  // Always start at 0 for proper attack
        phase = 0.0f;          // Reset oscillator phase
        filterState = 0.0f;    // Reset filter state
        highpassState = 0.0f;  // Reset high-pass filter state
        noteStartTime = juce::Time::currentTimeMillis();
    }
    
    // Release a note (transition to release phase)
    void release()
    {
        active = false;
        envelopeState = EnvelopeState::Release;
    }
    
    // Force stop a voice
    void forceStop()
    {
        active = false;
        envelopeState = EnvelopeState::Idle;
        envelopeLevel = 0.0f;
    }
    
    // Check if a note has been playing too long (safety timeout)
    bool hasTimedOut(int64_t currentTime) const
    {
        const int64_t maxNoteTimeMs = 30000; // 30 seconds max
        return (currentTime - noteStartTime) > maxNoteTimeMs;
    }
}; 