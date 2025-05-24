# HarmonyScape: Spatial Harmony Generator and Arranger
**VST Project Specification Document**

## Table of Contents
1. [Project Overview](#project-overview)
2. [Core Concept](#core-concept)
3. [Technical Implementation](#technical-implementation)
4. [Phased Product Plan](#phased-product-plan)
5. [Development Plan](#development-plan)
6. [Monetization Strategy](#monetization-strategy)
7. [Appendix: Technology Resources](#appendix-technology-resources)

## Project Overview

HarmonyScape is a VST plugin that transforms simple chord progressions into rich, spatially-aware harmonic environments. It addresses a significant gap in the current market by combining intelligent chord voicing with spatial audio capabilities and dynamic arrangement features. 

The plugin is designed to appeal to:
- Film and media composers seeking rich harmonic beds
- Electronic producers wanting to add harmonic depth without music theory expertise
- Songwriters looking to explore new chord progressions and voicings
- Sound designers creating evolving harmonic textures

## Core Concept

HarmonyScape combines three key innovations:

1. **Intelligent chord voicing and extension** - Takes basic chords and generates sophisticated voicings and extensions based on genre/style
2. **Spatial audio integration** - Places different harmonic elements across the stereo/3D sound field
3. **Dynamic arrangement** - Creates movement and evolution in harmonic content over time

## Technical Implementation

- Built on JUCE framework for cross-platform compatibility (Windows, macOS)
- VST3 format with AU support for maximum DAW compatibility
- Optimized CPU usage with intelligent voice allocation
- Modern, scalable UI with clear visualization of harmonic and spatial elements

## Phased Product Plan

### Phase 1: MVP (Minimum Viable Product)
**Core Functionality (Highest Priority)**
- [P0] Basic chord input via MIDI or internal step sequencer
- [P0] Fundamental chord voicing engine (basic extensions and inversions)
- [P0] Simple stereo placement controls for harmonic elements
- [P0] Stable VST3 implementation with standard DAW integration
- [P0] Essential preset system with 20+ factory presets
- [P0] Clean, functional UI with basic visualization
- [P0] Basic MIDI export of generated chord voicings

### Phase 2: Enhanced Features
**Secondary Features (Medium Priority)**
- [P1] Advanced chord voicing options (jazz, neo-soul, cinematic)
- [P1] Enhanced spatial positioning with 3D visualization
- [P1] Time-based parameter automation for evolving harmonies
- [P1] User preset saving and loading functionality
- [P1] Voice-leading optimization engine
- [P1] Expanded preset library (50+ total)
- [P1] Basic tension and release controls

### Phase 3: Advanced Capabilities
**Extended Features (Lower Priority)**
- [P2] Genre-specific harmonic templates and algorithms
- [P2] Full integration with binaural and Dolby Atmos standards
- [P2] Advanced pattern generator for rhythmic harmonic movements
- [P2] Community preset sharing platform
- [P2] MIDI file import/export with advanced options
- [P2] Custom scale and tuning support
- [P2] Comprehensive help system and interactive tutorials

### Phase 4: Professional Extensions
**Future Enhancements (Lowest Priority)**
- [P3] Integration with orchestral libraries for direct instrument assignment
- [P3] AI-powered progression suggestion engine
- [P3] Real-time performance controls for live use
- [P3] DAW-specific optimizations for major platforms
- [P3] Advanced modulation matrix for all parameters
- [P3] SDK for third-party extensions
- [P3] Mobile companion app for remote control and ideas

## Development Plan

### Phase 1 Implementation (MVP)

#### Stage 1: Core Architecture (Weeks 1-3)
1. Set up JUCE development environment
2. Implement basic plugin architecture
   - Audio processor component
   - Editor/UI component
   - Parameter management system
3. Create fundamental data structures for chord representation
4. Establish VST3/AU wrapper functionality

#### Stage 2: Chord Engine Development (Weeks 4-6)
1. Implement MIDI input handling
2. Develop basic chord detection algorithm
3. Create fundamental voicing engine
   - Basic extensions (7ths, 9ths)
   - Inversions
   - Simple voice distribution
4. Build step sequencer for internal chord progression creation

#### Stage 3: Spatial System Implementation (Weeks 7-8)
1. Develop stereo positioning algorithm
2. Create voice allocation system for spatial distribution
3. Implement basic stereo field visualization
4. Add simple panning automation

#### Stage 4: UI Development (Weeks 9-11)
1. Design and implement core UI layout
2. Create chord input and visualization components
3. Build spatial positioning controls
4. Implement preset browser interface
5. Develop parameter controls and automation handling

#### Stage 5: Testing and Refinement (Weeks 12-14)
1. Conduct comprehensive DAW compatibility testing
2. Perform CPU usage optimization
3. Create factory presets (minimum 20)
4. Fix bugs and refine user experience
5. Prepare for release

### Phase 2 Implementation

#### Stage 1: Advanced Chord Engine (Weeks 15-18)
1. Expand voicing algorithms to include genre-specific patterns
2. Implement voice-leading optimization engine
3. Add extended chord types and alterations
4. Develop intelligent bass note selection

#### Stage 2: Enhanced Spatial Features (Weeks 19-22)
1. Develop advanced 3D positioning algorithms
2. Create enhanced visualization of spatial field
3. Implement movement patterns for harmonic elements
4. Add depth and height parameters for supported formats

#### Stage 3: Dynamic Evolution System (Weeks 23-26)
1. Implement time-based parameter automation system
2. Develop tension and release algorithms
3. Create evolution patterns for harmonic complexity
4. Build envelope system for spatial movement

#### Stage 4: Expanded UI and Presets (Weeks 27-30)
1. Enhance UI with additional visualization options
2. Implement user preset system with metadata
3. Create 30+ additional factory presets
4. Refine parameter controls for intuitive use

### Technical Components Implementation Details

#### Chord Processing System
- Implement MIDI parsing for chord detection
- Create data structures for chord representation
- Develop voicing algorithms based on music theory principles
- Build extension and inversion logic with voice-leading constraints

#### Spatial Audio Engine
- Implement stereo field positioning algorithms
- Build 3D audio processing for advanced spatial features
- Create movement patterns with parameter automation
- Develop visualization system for spatial representation

#### User Interface
- Implement modular UI components using JUCE
- Create responsive layout for different window sizes
- Develop visualization components for harmony and spatial elements
- Build preset browser and management system

#### Plugin Infrastructure
- Implement parameter automation with DAW integration
- Create preset serialization and management
- Build MIDI export functionality
- Ensure cross-platform compatibility

## Monetization Strategy

- **Base Version**: $89 with all Phase 1 and Phase 2 features
- **Lite Version**: $39 with limited features as an entry point
- **Future Expansion Packs**: $29-39 for genre-specific content
  - Jazz Extensions Pack
  - Cinematic Harmonies Pack
  - Electronic Production Pack
- **Educational Discount**: 30% off for verified students/educators
- **Promotional Strategy**:
  - Freemium introduction version with limited features
  - Educational content showing practical applications
  - Strategic partnerships with music education platforms
  - YouTube tutorial series to demonstrate capabilities

## Appendix: Technology Resources

### JUCE Framework
- [JUCE Homepage](https://juce.com/) - C++ framework for audio applications
- [JUCE Documentation](https://docs.juce.com/) - Official documentation
- [JUCE Tutorial: Create a basic Audio/MIDI plugin](https://docs.juce.com/master/tutorial_create_projucer_basic_plugin.html) - Getting started with VST development
- [GitHub Repository](https://github.com/juce-framework/JUCE) - Source code and examples

### VST Development
- [VST3 SDK](https://steinbergmedia.github.io/vst3_doc/) - Official Steinberg VST3 documentation
- [The Audio Programmer](https://theaudioprogrammer.com/) - Educational resources for audio programming
- [KVR Audio Development Forum](https://www.kvraudio.com/forum/viewforum.php?f=33) - Community help for audio plugin development

### Spatial Audio Resources
- [Immersive Audio Resources](https://imeav.github.io/resource-list/) - Comprehensive list of spatial audio technologies
- [Dolby Atmos Music Tools](https://professional.dolby.com/music/) - Professional toolset for Atmos
- [Spatial Audio API Guide](https://developer.apple.com/documentation/musickit/musickit_spatial_audio) - Apple's spatial audio documentation
- [Binaural Processing](https://www.aes.org/e-lib/browse.cfm?elib=20133) - Academic resources for binaural algorithms

### Music Theory and Harmony
- [The Chord Genome Project](https://www.chordgenome.com/) - Research on chord progressions and relationships
- [Music Algorithm Resources](https://github.com/topics/music-generation) - GitHub repositories on algorithmic composition
- [Music21 Library](http://web.mit.edu/music21/) - A toolkit for computer-aided musicology

### GUI Development
- [JUCE Widget Gallery](https://docs.juce.com/master/tutorial_look_and_feel_customisation.html) - UI components and customization
- [Modern UI Design for Audio Plugins](https://www.amazingux.com/articles/) - Best practices for plugin UI
- [Plugin GUI Gallery](https://www.kvraudio.com/plugins/newest) - Reference for current plugin GUI design

### Audio Programming Fundamentals
- [The Audio Programmer YouTube Channel](https://www.youtube.com/c/TheAudioProgrammer) - Tutorials for audio plugin development
- [DSP Resources](https://www.dsprelated.com/) - Digital signal processing fundamentals
- [Will Pirkle's Plugin Books](https://www.willpirkle.com/) - Comprehensive guides to VST development

This document serves as the foundation for the HarmonyScape project, outlining both product and technical requirements. Development should proceed according to the phased approach, focusing first on MVP features before progressing to more advanced capabilities. 