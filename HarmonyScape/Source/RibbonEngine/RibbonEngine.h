#pragma once

#include "../JuceHeader.h"
#include <array>

/**
 * RibbonEngine handles rhythmic arpeggiations that ripple forward in space and time.
 * It creates up to 5 "ribbons" (rhythmic patterns) that can be applied to chord clusters.
 */
class RibbonEngine
{
public:
    // Maximum number of ribbons supported
    static constexpr int MAX_RIBBONS = 5;
    
    // Ribbon pattern types
    enum class RibbonPattern
    {
        Up,           // Low to high notes
        Down,         // High to low notes
        Outside,      // From middle outward
        Inside,       // From edges inward
        Random,       // Random order
        Cascade,      // Overlapping waves
        Spiral        // Circular pattern in space
    };
    
    // Individual ribbon configuration
    struct RibbonConfig
    {
        bool enabled = false;
        RibbonPattern pattern = RibbonPattern::Up;
        float rate = 0.5f;          // Speed of arpeggiation (0.0-1.0)
        float offset = 0.0f;        // Phase offset (0.0-1.0)
        float spatialSpread = 0.5f; // How much this ribbon affects spatial position
        float intensity = 1.0f;     // Volume/presence of this ribbon
        float decay = 0.8f;         // How quickly notes fade in the ribbon
    };
    
    // Global ribbon parameters
    struct RibbonParams
    {
        int activeRibbons = 1;      // Number of active ribbons (1-5)
        float globalRate = 0.5f;    // Master rate control
        float spatialMovement = 0.3f; // How much ribbons affect spatial positioning
        float rhythmSync = 0.0f;    // Sync to host tempo (0=free, 1=sync)
        bool enableRibbons = true;  // Master enable/disable
        
        std::array<RibbonConfig, MAX_RIBBONS> ribbons;
    };
    
    // Ribbon note event for scheduling
    struct RibbonNote
    {
        int midiNote = 60;
        int ribbon = 0;             // Which ribbon this belongs to
        double startTime = 0.0;     // When to start (in samples)
        double duration = 0.0;      // How long to play (in samples)
        float velocity = 1.0f;      // Note velocity
        float spatialPosition = 0.0f; // Spatial position for this note
        bool active = false;
        int stepIndex = 0;          // Position in arpeggiation sequence
    };

    RibbonEngine();
    ~RibbonEngine();
    
    /**
     * Prepares the engine with audio specs
     */
    void prepare(double sampleRate, int samplesPerBlock);
    
    /**
     * Process a chord cluster and generate rhythmic ribbon patterns
     * @param chordNotes Array of MIDI notes representing the chord
     * @param ribbonParams Configuration for ribbon behavior
     * @param numSamples Number of samples in this processing block
     * @param hostTempo Current host tempo (BPM) for sync
     * @return Array of RibbonNote events to be triggered
     */
    juce::Array<RibbonNote> processChord(const juce::Array<int>& chordNotes,
                                        const RibbonParams& ribbonParams,
                                        int numSamples,
                                        double hostTempo = 120.0);
    
    /**
     * Update internal timing and generate note events for this block
     */
    void advanceTime(int numSamples);
    
    /**
     * Get currently active ribbon notes for this sample position
     */
    juce::Array<RibbonNote> getActiveNotes(int samplePosition) const;
    
    /**
     * Reset all ribbon state (e.g., when transport stops)
     */
    void reset();
    
    /**
     * Set the current chord for ribbon processing
     */
    void setCurrentChord(const juce::Array<int>& chordNotes);

private:
    /**
     * Generate arpeggiation sequence for a ribbon pattern
     */
    juce::Array<int> generateArpeggiationSequence(const juce::Array<int>& chordNotes,
                                                  RibbonPattern pattern,
                                                  int ribbonIndex);
    
    /**
     * Calculate spatial position for a note in a ribbon
     */
    float calculateRibbonSpatialPosition(int noteIndex, int totalNotes, 
                                       const RibbonConfig& config,
                                       float globalSpatialMovement);
    
    /**
     * Calculate timing for ribbon note events
     */
    double calculateNoteStartTime(int stepIndex, const RibbonConfig& config,
                                 float globalRate, double hostTempo, bool sync);
    
    /**
     * Update ribbon phase and generate new events
     */
    void updateRibbonPhase(int ribbonIndex, const RibbonConfig& config,
                          const juce::Array<int>& chordNotes, int numSamples);

    // Engine state
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    double currentSamplePosition = 0.0;
    
    // Current chord being processed
    juce::Array<int> currentChordNotes;
    
    // Ribbon state
    struct RibbonState
    {
        double phase = 0.0;              // Current phase in pattern
        int currentStep = 0;             // Current step in sequence
        juce::Array<int> sequence;       // Current arpeggiation sequence
        double lastEventTime = 0.0;      // When last note was triggered
        bool active = false;
    };
    
    std::array<RibbonState, MAX_RIBBONS> ribbonStates;
    
    // Scheduled note events
    juce::Array<RibbonNote> scheduledNotes;
    
    // Timing utilities
    double beatsToSamples(double beats, double bpm) const;
    double samplesToBeats(double samples, double bpm) const;
}; 