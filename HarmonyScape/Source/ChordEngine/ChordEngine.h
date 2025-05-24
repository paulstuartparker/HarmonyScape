#pragma once

#include "../JuceHeader.h"

/**
 * ChordEngine handles chord recognition and voicing.
 * It analyzes MIDI input to detect chords and generates appropriate voicings.
 */
class ChordEngine
{
public:
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
     * Represents a recognized chord
     */
    struct Chord
    {
        juce::String name;           // e.g., "Cmaj7"
        int rootNote = 60;           // MIDI note number (C4 = 60)
        juce::Array<int> notes;      // MIDI note numbers of chord tones
        
        bool isEmpty() const { return notes.isEmpty(); }
    };
    
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
    juce::Array<int> activeNotes;
    Chord currentChord;
    juce::Array<int> currentVoicing;
    
    // Cached parameters
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
}; 