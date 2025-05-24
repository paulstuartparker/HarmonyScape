#pragma once

#include "../JuceHeader.h"
#include "../Voice.h"
#include <array>

// Forward declarations
struct Voice;
struct ADSRParams;

/**
 * SpatialEngine handles stereo positioning of chord voicings.
 * It generates audio from MIDI and positions voices across the stereo field.
 */
class SpatialEngine
{
public:
    // Waveform types supported by the engine
    enum class WaveformType
    {
        Sine,
        Saw,
        Square,
        Triangle
    };

    // ADSR envelope parameter struct
    struct ADSRParams
    {
        float attack;   // in seconds
        float decay;    // in seconds
        float sustain;  // 0.0-1.0 level
        float release;  // in seconds
    };

    SpatialEngine();
    ~SpatialEngine();
    
    /**
     * Prepares the engine with audio specs
     */
    void prepare(double sampleRate, int samplesPerBlock);
    
    /**
     * Process audio buffer using generated chord voicings
     * @param buffer Audio buffer to fill with generated sounds
     * @param midiBuffer MIDI data representing chord voicings
     * @param spatialWidth Width parameter (0.0-1.0) controlling stereo spread
     * @param waveformType The type of waveform to generate
     * @param volume Master volume level (0.0-1.0)
     * @param adsr ADSR envelope parameters
     */
    void process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiBuffer, 
                float spatialWidth, WaveformType waveformType, float volume,
                const ADSRParams& adsr);
                
    /**
     * Get all currently active voice notes (including those in release phase)
     * Used for visual display of active notes
     * @return Array of MIDI note numbers that are currently sounding
     */
    juce::Array<int> getActiveVoiceNotes() const;
    
    // Store generated chord output for visualization
    void setChordOutput(const juce::MidiBuffer& output) { chordOutput = output; }
    
private:
    /**
     * Represents a voice with position information
     */
    struct Voice
    {
        int midiNote = 0;
        bool active = false;
        float position = 0.0f;  // -1.0 to 1.0 (stereo position)
        float phase = 0.0f;
        int chordPosition = 0;  // Position within the chord (0 = root, etc.)
        
        // Time tracking for safety release
        int64_t noteStartTime = 0;
        int64_t noteMaxDuration = 10000; // 10 seconds in milliseconds - safety limit
        
        // Envelope state
        enum class EnvelopeState
        {
            Idle,
            Attack,
            Decay,
            Sustain,
            Release
        };
        
        EnvelopeState envelopeState = EnvelopeState::Idle;
        float envelopeLevel = 0.0f;  // Current envelope level
        float filterState = 0.0f;    // Simple one-pole low-pass filter state
        float highpassState = 0.0f;  // High-pass filter state for removing muddiness
        
        void trigger(int note, float pos, int chordPos = 0) 
        {
            midiNote = note;
            active = true;
            position = pos;
            chordPosition = chordPos;
            envelopeState = EnvelopeState::Attack;
            noteStartTime = juce::Time::currentTimeMillis();
            
            // Start at 0 for proper attack envelope
            envelopeLevel = 0.0f;
            // Reset phase for clean start
            phase = 0.0f;
            // Reset filter state
            filterState = 0.0f;
            // Reset high-pass filter state
            highpassState = 0.0f;
        }
        
        void release() 
        {
            active = false;
            
            // Always transition to release phase if currently active
            if (envelopeState != EnvelopeState::Idle) {
                envelopeState = EnvelopeState::Release;
            }
        }
        
        // Check if voice is currently audible (making sound)
        bool isAudible() const
        {
            return (active || envelopeState != EnvelopeState::Idle) && envelopeLevel > 0.0f;
        }
        
        // Force a voice to stop immediately
        void forceStop()
        {
            active = false;
            envelopeState = EnvelopeState::Idle;
            envelopeLevel = 0.0f;
            filterState = 0.0f;
            highpassState = 0.0f;
        }
        
        // Check if this voice has been playing too long (safety feature)
        bool hasTimedOut(int64_t currentTime) const
        {
            return (currentTime - noteStartTime) > noteMaxDuration;
        }
    };
    
    /**
     * Renders a single voice to the buffer
     */
    void renderVoice(Voice& voice, juce::AudioBuffer<float>& buffer, int startSample, int numSamples, 
                    WaveformType waveformType, float masterVolume, const ADSRParams& adsr);
    
    /**
     * Processes the envelope for a voice
     */
    void processEnvelope(Voice& voice, const ADSRParams& adsr, float& envelopeIncrement);
    
    /**
     * Calculates stereo position for a given MIDI note and chord position
     * @param midiNote The MIDI note number
     * @param chordPosition Position in the chord (0 = root, etc.)
     * @param width The width parameter (0.0-1.0)
     * @return Position value between -1.0 and 1.0
     */
    float calculatePosition(int midiNote, int chordPosition, float width);
    
    /**
     * Generates a sample based on the selected waveform type
     */
    float generateSample(float phase, WaveformType waveformType);
    
    // Audio generator state
    std::array<Voice, 16> voices;  // Polyphony of 16 voices
    
    // Cached parameters
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    
    // Track active notes for keyboard visualization
    juce::Array<int> userInputNotes;
    juce::Array<int> generatedNotes;
    juce::MidiBuffer chordOutput;  // Store the last chord output for visualization
}; 