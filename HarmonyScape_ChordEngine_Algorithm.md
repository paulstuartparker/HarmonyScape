# HarmonyScape Chord Voicing Engine
**Technical Architecture and Algorithm Design**

## Overview

The chord voicing engine is a core component of HarmonyScape that transforms basic chord input into rich, musically coherent voicings. This document outlines the technical architecture and algorithm designs that will power this functionality.

## System Architecture

The chord engine consists of four primary subsystems:

1. **Chord Recognition Module** - Identifies incoming chords from MIDI or internal sequencer
2. **Voicing Generator** - Creates appropriate voicings based on musical context and user settings
3. **Voice Leading Optimizer** - Ensures smooth transitions between chord voicings
4. **Distribution Engine** - Allocates voices across the spatial field

```
┌───────────────────┐     ┌───────────────────┐     ┌───────────────────┐     ┌───────────────────┐
│  Chord            │     │  Voicing          │     │  Voice Leading    │     │  Distribution     │
│  Recognition      │──-->│  Generator        │──-->│  Optimizer        │──-->│  Engine           │
│  Module           │     │                   │     │                   │     │                   │
└───────────────────┘     └───────────────────┘     └───────────────────┘     └───────────────────┘
         │                         ▲                         ▲                         │
         │                         │                         │                         │
         │                         │                         │                         ▼
         │                 ┌───────────────────┐     ┌───────────────────┐     ┌───────────────────┐
         └────────────────>│  Music Theory     │     │  Progression      │<────│  Spatial          │
                           │  Rules Engine     │────>│  Context          │     │  Configuration    │
                           └───────────────────┘     └───────────────────┘     └───────────────────┘
```

## 1. Chord Recognition Module

### Input Types
- MIDI note data (live or from DAW)
- Step sequencer with chord buttons
- Text-based chord input (e.g., "Cmaj7")

### Chord Detection Algorithm
The chord detection algorithm uses the following approach:

1. **Note Collection**:
   - Gather all active MIDI notes within the detection window (10-30ms)
   - Filter out notes outside the detection range (optional)

2. **Interval Analysis**:
   - Calculate all intervals between notes (relative to lowest note)
   - Create an interval fingerprint (array of semitone distances)

3. **Chord Type Matching**:
   - Compare fingerprint against standard chord types
   - Calculate confidence score for each potential match
   - Select highest confidence match above threshold

4. **Root Detection**:
   - Identify the most likely root note based on:
     - Lowest note (bass-priority mode)
     - Note with strongest support from intervals (theory-priority mode)
     - User preference setting

### Chord Representation Data Structure
```typescript
interface Chord {
  root: Note;             // Root note (e.g., C)
  quality: ChordQuality;  // Major, minor, diminished, etc.
  extensions: number[];   // Array of extensions (e.g., [7, 9] for 7th and 9th)
  alterations: {          // Altered tones
    note: number;         // Scale degree
    alteration: number;   // Semitones (+/-1 for sharp/flat)
  }[];
  inversion: number;      // 0=root position, 1=first inversion, etc.
  bass: Note;             // Bass note if different from root
}
```

## 2. Voicing Generator

### Principles of Operation
The voicing generator transforms the abstract chord representation into specific notes distributed across registers. It applies music theory rules and stylistic patterns to create musically appropriate voicings.

### Core Algorithm

#### Step 1: Essential Tones Selection
1. Include the root (except in specific styles or high inversions)
2. Include the defining chord tone (3rd for major/minor, 5th for sus4, etc.)
3. Add character tones based on chord type (7th, 9th, etc.)
4. Apply user-defined "density" parameter to include/exclude optional tones

#### Step 2: Register Distribution
1. Set the primary register based on user settings
2. Distribute essential tones across optimal ranges:
   - Root/5th: Lower registers
   - 3rds/7ths: Middle registers
   - Extensions (9ths, 11ths, 13ths): Upper registers
3. Avoid excessive intervals between adjacent voices (typically >12 semitones)
4. Apply voice count constraints based on style and user settings

#### Step 3: Style Application
1. Apply style-specific voicing templates:
   - Jazz: Extended voicings with alterations, often rootless
   - Pop: Simpler voicings with clear harmonic function
   - Classical: Traditional voice leading with functional harmony
   - Electronic: Open voicings with strategic tone omissions for clarity
2. Adjust voicing width (distance between lowest and highest notes)
3. Apply style-specific tone preferences (e.g., avoid certain intervals in specific styles)

#### Style Templates Data Structure
```typescript
interface VoicingStyle {
  id: string;                     // Style identifier
  name: string;                   // Display name
  defaultDensity: number;         // 0.0-1.0 density setting
  preferredExtensions: number[];  // Extensions to prioritize
  avoidedExtensions: number[];    // Extensions to avoid
  rootBassFrequency: number;      // How often root is in bass (0.0-1.0)
  voiceCountRange: [number, number]; // Min/max voice count
  intervalPreferences: {           // Interval preferences
    preferred: number[];          // Preferred intervals
    avoided: number[];            // Avoided intervals
  };
  patternRules: PatternRule[];    // Style-specific pattern rules
}
```

## 3. Voice Leading Optimizer

The voice leading optimizer ensures smooth transitions between chord voicings by minimizing the movement of individual voices and following traditional voice leading principles.

### Algorithm Stages

#### Stage 1: Voice Mapping
1. Identify corresponding voices between consecutive chords
2. Create a voice mapping that minimizes total distance moved
3. Detect voices that need to be added or removed

#### Stage 2: Movement Optimization
1. Minimize overall movement using optimization algorithm:
   - Keep common tones static when possible
   - Move each voice to nearest available destination note
   - Ensure no voice crossing unless stylistically appropriate
2. Handle special cases:
   - Leading tones resolve upward
   - 7ths resolve downward
   - Avoid parallel 5ths and octaves in classical style

#### Stage 3: Transition Smoothing
1. Apply transition rules based on current style
2. Handle voice count changes (adding/removing voices)
3. Apply minimum/maximum constraints on voice movement

### Voice Leading Rules Matrix
Each style has a matrix of voice leading preferences:

```typescript
interface VoiceLeadingRules {
  maxMovement: number;             // Maximum semitones for any voice
  preferredResolutions: {          // Preferred resolution targets
    scale_degree: number;          // Scale degree
    preferred_movement: number[];  // Preferred movement options
  }[];
  commonTonePreference: number;    // 0.0-1.0 preference for common tones
  parallelMotionAvoidance: {       // Parallel motion constraints
    fifths: boolean;
    octaves: boolean;
    preference: number;            // 0.0-1.0 strength of constraint
  };
}
```

## 4. Distribution Engine

The Distribution Engine allocates voices across the stereo/3D field based on spatial settings and musical context.

### Core Functionality

#### Spatial Allocation Algorithm
1. Create spatial groups based on voice function:
   - Bass (lowest notes)
   - Core (essential harmonic tones)
   - Color (extensions and alterations)
2. Apply primary spatial configuration:
   - Width: How spread out across stereo field
   - Depth: Distribution in front/back (for 3D formats)
   - Height: Distribution of vertical placement (for 3D formats)
3. Calculate specific position for each note based on:
   - Pitch (higher notes typically wider/higher in field)
   - Function (bass centered, extensions wider)
   - User spatial presets

#### Movement Patterns
The Distribution Engine also handles dynamic movement of voices in the spatial field:

1. Static positioning - Fixed positions for each voice
2. Orbital movement - Voices move in configurable patterns
3. Expansion/contraction - Spatial width changes with harmonic tension
4. Frequency-based - Higher notes positioned differently than lower notes

### Data Structures

```typescript
interface SpatialVoice {
  note: number;              // MIDI note
  velocity: number;          // MIDI velocity
  position: {                // Spatial position
    x: number;               // -1.0 to 1.0 (left to right)
    y: number;               // -1.0 to 1.0 (down to up)
    z: number;               // -1.0 to 1.0 (back to front)
  };
  movement: {                // Optional movement pattern
    type: MovementType;      // Static, orbital, etc.
    parameters: any;         // Pattern-specific parameters
  };
}
```

## Integration with Other Systems

### UI Interaction
- Visualization of chord voicings on staff notation and/or piano roll
- Real-time updates as user modifies parameters
- Visualization of spatial positioning

### DAW Integration
- Time-synchronized generation based on transport position
- Parameter automation for all major controls
- MIDI export of generated voicings

### Preset System
- Style-based presets with complete voicing and spatial settings
- User-defined presets with metadata
- A/B comparison feature for exploring variations

## Implementation Notes

### MVP Implementation (Phase 1)
For the MVP, we'll implement a simplified version with:
- Basic chord detection for common chord types
- 3-4 core voicing styles (Pop, Jazz, Cinematic)
- Simple voice leading with common tone preservation
- Basic stereo field distribution
- Limited parameter set for user control

### Optimization Considerations
- Use lookup tables for common chord voicings to reduce CPU usage
- Implement multi-threaded processing for voice generation
- Cache recent calculations to avoid redundant processing
- Optimize voice leading calculations with motion prediction

### Future Extensions (Phase 2+)
- Machine learning model for style-specific voicing generation
- Advanced voice leading with contextual awareness of phrase
- Integration with specific orchestral libraries
- User-trainable voicing preferences

This technical architecture provides the foundation for HarmonyScape's chord voicing engine, balancing musical sophistication with computational efficiency to deliver a powerful yet intuitive tool for harmonic exploration. 