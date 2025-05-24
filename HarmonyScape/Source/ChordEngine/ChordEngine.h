#pragma once

#include "../JuceHeader.h"

/**
 * ChordEngine handles chord recognition and voicing.
 * It analyzes MIDI input to detect chords and generates appropriate voicings.
 */
class ChordEngine
{
public:
    /**
     * Represents a recognized chord
     */
    struct Chord
    {
        juce::String name;           // e.g., "Cmaj7"
        int rootNote = 60;           // MIDI note number (C4 = 60)
        juce::Array<int> notes;      // MIDI note numbers of chord tones
        
        bool isEmpty() const { return notes.isEmpty(); }
    };

    ChordEngine();
    ~ChordEngine();
    
    /**
     * Prepares the engine with audio specs
     */
    void prepare(double sampleRate, int samplesPerBlock);
    
    /**
     * Process incoming MIDI data, recognize chords and generate voicings
     * @param midiMessages Incoming MIDI buffer
     * @param densityParam Chord density parameter (0.0-1.0)
     * @return Generated chord voices as a MIDI buffer
     */
    juce::MidiBuffer processMidi(const juce::MidiBuffer& midiMessages, float densityParam);
    
    /**
     * Update harmonic context and sustaining notes
     */
    void updateHarmonicContext(double currentTimeSeconds, double adsrReleaseTime);
    
    /**
     * Get currently sustaining notes for contextual awareness
     */
    juce::Array<int> getSustainingNotes() const;
    
    /**
     * Generate contextually-aware chord voicing considering tonal gravity
     */
    juce::Array<int> generateContextualVoicing(const Chord& newChord, float density);
    
    /**
     * Find harmonic bridge between current context and new chord
     */
    juce::Array<int> findHarmonicBridge(const juce::Array<int>& currentHarmony, const juce::Array<int>& newNotes);
    
private:
    /**
     * Analyzes active notes to detect the chord
     */
    Chord detectChord(const juce::Array<int>& activeNotes);
    
    /**
     * Generates appropriate voicings for the detected chord
     */
    juce::Array<int> generateVoicing(const Chord& chord, float density);
    
    /**
     * Maps intervals to chord types for recognition
     */
    bool matchChordType(const juce::Array<int>& intervals, juce::String& chordName);
    
    // Engine state
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    
    // Active notes currently being played by user
    juce::Array<int> activeNotes;
    
    // Sustaining notes (notes that are still ringing with ADSR)
    struct SustainingNote
    {
        int noteNumber;
        double startTime;
        double releaseTime = -1.0; // -1 means still held, >=0 means released at this time
        float velocity;
    };
    juce::Array<SustainingNote> sustainingNotes;
    
    // Harmonic context for tonal gravity
    struct HarmonicContext
    {
        juce::Array<int> recentRootNotes;    // Recently used root notes for tonal gravity
        juce::Array<int> harmonicField;      // Current harmonic field (all recently active notes)
        double lastUpdateTime = 0.0;
    };
    HarmonicContext harmonicContext;
    
    // Current chord and voicing state
    Chord currentChord;
    juce::Array<int> currentVoicing;
    double currentTime = 0.0;
}; 