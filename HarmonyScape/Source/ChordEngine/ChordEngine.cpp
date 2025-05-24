#include "ChordEngine.h"

ChordEngine::ChordEngine()
{
}

ChordEngine::~ChordEngine()
{
}

void ChordEngine::prepare(double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;
}

juce::MidiBuffer ChordEngine::processMidi(const juce::MidiBuffer& midiMessages, float densityParam)
{
    // Track which notes were turned off in this block
    juce::Array<int> notesOff;
    
    // Process incoming MIDI messages to update active notes
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            int noteNumber = message.getNoteNumber();
            if (!activeNotes.contains(noteNumber))
                activeNotes.add(noteNumber);
        }
        else if (message.isNoteOff())
        {
            int noteNumber = message.getNoteNumber();
            notesOff.add(noteNumber);
            activeNotes.removeFirstMatchingValue(noteNumber);
        }
    }
    
    // Detect chord from active notes (now handles single notes too)
    if (activeNotes.size() >= 1)  // Changed from >= 2 to >= 1
    {
        currentChord = detectChord(activeNotes);
    }
    else
    {
        // Reset if no notes active
        currentChord = Chord();
        currentVoicing.clear();
    }
    
    // Generate output MIDI buffer with chord voicing
    juce::MidiBuffer outputBuffer;
    
    // First, handle any note-offs that need to be sent
    for (int i = 0; i < notesOff.size(); ++i)
    {
        // Send note-off for any generated notes based on this input note
        for (int voiceNote : currentVoicing)
        {
            // Check if this voicing note is related to the note that was turned off
            // For now, turn off all voicing notes when any input note is released
            // This ensures proper envelope release behavior
            outputBuffer.addEvent(juce::MidiMessage::noteOff(1, voiceNote, 0.0f), 0);
        }
    }
    
    if (!currentChord.isEmpty())
    {
        // Generate voicing based on current chord and density parameter
        auto newVoicing = generateVoicing(currentChord, densityParam);
        
        // Only send note-ons for truly new notes
        for (int i = 0; i < newVoicing.size(); ++i)
        {
            int newNote = newVoicing[i];
            if (!currentVoicing.contains(newNote))
            {
                // Turn on new notes in the voicing
                outputBuffer.addEvent(juce::MidiMessage::noteOn(1, newNote, 0.8f), 0);
            }
        }
        
        // Update current voicing
        currentVoicing = newVoicing;
    }
    else
    {
        // All notes released - clear voicing
        currentVoicing.clear();
    }
    
    return outputBuffer;
}

ChordEngine::Chord ChordEngine::detectChord(const juce::Array<int>& notes)
{
    if (notes.size() < 1)  // Changed from < 2 to < 1
        return Chord();
    
    // Sort notes in ascending order
    juce::Array<int> sortedNotes = notes;
    sortedNotes.sort();
    
    // Find root note (lowest note for now in MVP)
    int rootNote = sortedNotes[0];
    
    // Handle single notes
    if (sortedNotes.size() == 1)
    {
        Chord chord;
        chord.rootNote = rootNote;
        chord.notes = sortedNotes;
        
        // Single note - just name it
        static const juce::String noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        chord.name = noteNames[rootNote % 12];
        
        return chord;
    }
    
    // Calculate intervals relative to root for multiple notes
    juce::Array<int> intervals;
    for (auto note : sortedNotes)
    {
        int interval = (note - rootNote) % 12;
        if (!intervals.contains(interval) && interval > 0)
            intervals.add(interval);
    }
    
    // Create chord based on intervals
    Chord chord;
    chord.rootNote = rootNote;
    chord.notes = sortedNotes;
    
    // Map intervals to chord name
    if (matchChordType(intervals, chord.name))
    {
        // Root note name
        static const juce::String noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        juce::String rootName = noteNames[rootNote % 12];
        
        // Combine root name with chord type
        chord.name = rootName + chord.name;
    }
    else
    {
        // Default to just a collection of notes if we can't identify the chord
        chord.name = "Unknown";
    }
    
    return chord;
}

bool ChordEngine::matchChordType(const juce::Array<int>& intervals, juce::String& chordName)
{
    // Basic chord type recognition based on interval pattern
    // MVP implementation with limited chord types
    
    if (intervals.contains(4) && intervals.contains(7))
    {
        // Major chord
        if (intervals.contains(11))
            chordName = "maj7";
        else if (intervals.contains(10))
            chordName = "7";
        else
            chordName = "maj";
        return true;
    }
    else if (intervals.contains(3) && intervals.contains(7))
    {
        // Minor chord
        if (intervals.contains(10))
            chordName = "m7";
        else
            chordName = "m";
        return true;
    }
    else if (intervals.contains(3) && intervals.contains(6))
    {
        // Diminished chord
        if (intervals.contains(9))
            chordName = "dim7";
        else
            chordName = "dim";
        return true;
    }
    else if (intervals.contains(4) && intervals.contains(8))
    {
        // Augmented chord
        chordName = "aug";
        return true;
    }
    else if (intervals.contains(5) && intervals.contains(7))
    {
        // Sus4 chord
        chordName = "sus4";
        return true;
    }
    
    // Unknown chord type
    chordName = "";
    return false;
}

juce::Array<int> ChordEngine::generateVoicing(const Chord& chord, float density)
{
    juce::Array<int> voicing;
    
    if (chord.isEmpty())
        return voicing;
    
    // DON'T include the user's original notes - they're already playing!
    // We want to generate ADDITIONAL harmonic notes only
    
    // For single notes, generate simple harmony
    if (chord.notes.size() == 1)
    {
        int root = chord.notes[0];
        
        // Generate major triad based on density
        if (density > 0.3f)
        {
            voicing.add(root + 4);  // Major third
            voicing.add(root + 7);  // Perfect fifth
        }
        
        if (density > 0.6f)
        {
            voicing.add(root + 12); // Octave
            voicing.add(root + 16); // Major third + octave
        }
        
        if (density > 0.8f)
        {
            voicing.add(root + 19); // Perfect fifth + octave
            voicing.add(root - 12); // Octave below
        }
    }
    else
    {
        // For chord input, add octave transpositions and extensions
        int octaveSpread = 1 + static_cast<int>(density * 2); // 1-3 octaves based on density
        int noteCount = 2 + static_cast<int>(density * 4);    // 2-6 additional notes based on density
        
        // Add octave transpositions of the chord tones (but not the originals)
        for (int octave = 1; octave <= octaveSpread && voicing.size() < noteCount; ++octave)
        {
            for (auto note : chord.notes)
            {
                int transposedNote = note + (octave * 12);
                if (transposedNote < 108) // Stay within MIDI range
                {
                    voicing.addIfNotAlreadyThere(transposedNote);
                    if (voicing.size() >= noteCount)
                        break;
                }
            }
        }
        
        // Add some notes below the root for richness
        if (density > 0.5f && voicing.size() < noteCount)
        {
            int root = chord.rootNote;
            int bassNote = root - 12; // Octave below
            if (bassNote >= 24) // Keep it reasonable
            {
                voicing.addIfNotAlreadyThere(bassNote);
            }
        }
    }
    
    return voicing;
} 