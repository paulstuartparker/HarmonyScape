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
    
    // Calculate new voicing BEFORE sending any note-offs
    juce::Array<int> newVoicing;
    if (!currentChord.isEmpty())
    {
        newVoicing = generateVoicing(currentChord, densityParam);
    }
    
    // Now handle note-offs intelligently
    if (notesOff.size() > 0)
    {
        // Only turn off notes that are NOT in the new voicing
        for (int voiceNote : currentVoicing)
        {
            if (!newVoicing.contains(voiceNote))
            {
                // This note is no longer needed, turn it off
                outputBuffer.addEvent(juce::MidiMessage::noteOff(1, voiceNote, 0.0f), 0);
            }
        }
    }
    
    // Now send note-ons for truly new notes
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
    
    // For single notes, generate more colorful and contextual harmony
    if (chord.notes.size() == 1)
    {
        int root = chord.notes[0];
        int rootPitchClass = root % 12;
        
        // Create different harmonic colors based on the root note and density
        // Lower density = simpler harmonies, higher density = more complex
        
        if (density < 0.33f)
        {
            // Simple: Just add a fifth for a power chord feel
            voicing.add(root + 7);  // Perfect fifth
            
            // Only add low octave if the root is not already too low
            if (root > 48 && root < 72) // Avoid mud in low register
                voicing.add(root - 12);
        }
        else if (density < 0.66f)
        {
            // Medium: Create sus2/sus4 ambiguity for color
            // Avoid too many notes in the low register
            if (root < 48) // Low notes
            {
                voicing.add(root + 7);   // Perfect fifth
                voicing.add(root + 12);  // Octave
                voicing.add(root + 14);  // Major ninth
            }
            else // Mid to high notes
            {
                voicing.add(root + 2);   // Major second (sus2)
                voicing.add(root + 7);   // Perfect fifth
                voicing.add(root + 12);  // Octave
                
                // Add some shimmer in the upper register
                if (root < 72)
                {
                    voicing.add(root + 14); // Major ninth
                    voicing.add(root + 19); // Perfect fifth + octave
                }
            }
        }
        else
        {
            // Complex: Create a rich, jazz-influenced stack
            // Use different voicings based on the pitch class for variety
            // But avoid clustering in low frequencies
            
            if (root < 48) // Low register - use wider spacing
            {
                switch (rootPitchClass)
                {
                    case 0:  // C
                    case 5:  // F
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 16);  // Major third + octave
                        voicing.add(root + 23);  // Major seventh + octave
                        break;
                        
                    case 2:  // D
                    case 7:  // G
                    case 9:  // A
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 15);  // Minor third + octave
                        voicing.add(root + 22);  // Minor seventh + octave
                        break;
                        
                    default:
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 12);  // Octave
                        voicing.add(root + 19);  // Fifth + octave
                        break;
                }
            }
            else // Mid to high register - normal voicings
            {
                switch (rootPitchClass)
                {
                    case 0:  // C - Cmaj9#11
                    case 5:  // F - Fmaj9#11
                        voicing.add(root + 4);   // Major third
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 11);  // Major seventh
                        voicing.add(root + 14);  // Major ninth
                        if (root < 72)
                            voicing.add(root + 18);  // #11
                        break;
                        
                    case 2:  // D - Dm11
                    case 7:  // G - Gm11
                    case 9:  // A - Am11
                        voicing.add(root + 3);   // Minor third
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 10);  // Minor seventh
                        voicing.add(root + 14);  // Major ninth
                        if (root < 72)
                            voicing.add(root + 17);  // Perfect 11th
                        break;
                        
                    case 4:  // E - E7#9
                    case 11: // B - B7#9
                        voicing.add(root + 4);   // Major third
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 10);  // Minor seventh
                        voicing.add(root + 15);  // #9
                        break;
                        
                    default: // Others - sus4add9
                        voicing.add(root + 5);   // Perfect fourth
                        voicing.add(root + 7);   // Perfect fifth
                        voicing.add(root + 14);  // Major ninth
                        voicing.add(root + 12);  // Octave
                        break;
                }
                
                // Only add bass notes if we're in a good range
                if (root > 60 && root < 84 && voicing.size() < 6)
                {
                    voicing.add(root - 12); // Octave below
                }
            }
        }
    }
    else
    {
        // For chord input, be more selective about what we add
        // Focus on color tones and upper extensions rather than just octaves
        
        int rootNote = chord.rootNote;
        bool hasThird = false;
        bool hasSeventh = false;
        
        // Analyze what's in the chord already
        for (auto note : chord.notes)
        {
            int interval = (note - rootNote) % 12;
            if (interval == 3 || interval == 4) hasThird = true;
            if (interval == 10 || interval == 11) hasSeventh = true;
        }
        
        if (density < 0.33f)
        {
            // Light: Just add some sparkle on top
            if (rootNote + 24 < 108 && rootNote > 36)
                voicing.add(rootNote + 24); // Two octaves up
                
            // Only add bass if root is in mid range
            if (rootNote > 48 && rootNote < 72)
                voicing.add(rootNote - 12);
        }
        else if (density < 0.66f)
        {
            // Medium: Add color tones
            if (!hasSeventh && rootNote + 11 < 108)
                voicing.add(rootNote + 11); // Add major 7th for color
                
            // Add 9th for shimmer
            if (rootNote + 14 < 108)
                voicing.add(rootNote + 14);
                
            // Add some upper structure, but avoid too many low notes
            for (auto note : chord.notes)
            {
                if (note > 48 && note + 12 < 96 && voicing.size() < 4)
                    voicing.add(note + 12);
            }
        }
        else
        {
            // Complex: Build rich upper structure triads
            // Avoid adding too many notes below middle C (60)
            if (chord.name.contains("maj") && rootNote > 36)
            {
                if (rootNote + 14 < 108) voicing.add(rootNote + 14); // 9th
                if (rootNote + 18 < 108) voicing.add(rootNote + 18); // #11
                if (rootNote + 21 < 108) voicing.add(rootNote + 21); // 13th
            }
            else if (chord.name.contains("m") && rootNote > 36)
            {
                if (rootNote + 14 < 108) voicing.add(rootNote + 14); // 9th
                if (rootNote + 17 < 108) voicing.add(rootNote + 17); // 11th
                if (rootNote + 20 < 108) voicing.add(rootNote + 20); // b13
            }
            else if (chord.name.contains("7") && rootNote > 36)
            {
                if (rootNote + 14 < 108) voicing.add(rootNote + 14); // 9th
                if (rootNote + 16 < 108) voicing.add(rootNote + 16); // #9
                if (rootNote + 21 < 108) voicing.add(rootNote + 21); // 13th
            }
            
            // Only add bass notes if we're in an appropriate range
            if (rootNote > 48 && rootNote < 72)
            {
                voicing.add(rootNote - 12); // Root an octave down
            }
        }
        
        // Remove any notes that might clash or are out of reasonable range
        for (int i = voicing.size() - 1; i >= 0; --i)
        {
            if (voicing[i] < 36 || voicing[i] > 108) // Tighter range to avoid mud
                voicing.remove(i);
        }
    }
    
    return voicing;
} 