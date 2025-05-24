# HarmonyScape JUCE Project Setup Guide
**Getting Started with Development Environment**

## Overview

This document outlines the setup process for developing the HarmonyScape VST/AU plugin using the JUCE framework. It covers environment setup, project configuration, and folder structure to ensure a consistent development process.

## Development Environment Requirements

### Minimum Requirements

- **C++ Compiler**:
  - Windows: Visual Studio 2019 or newer
  - macOS: Xcode 12.4 or newer (with Command Line Tools)
  - Linux: GCC 9.0 or newer

- **JUCE Framework**: Version 7.0.0 or newer

- **VST3 SDK**: Included with JUCE (no separate download needed)

- **Git**: For version control

- **CMake**: Version 3.22 or newer (for cross-platform builds)

### Recommended Tools

- **IDE**:
  - Windows: Visual Studio 2022
  - macOS: Xcode 14+
  - Linux: CLion or Visual Studio Code with C++ extensions

- **Audio Tools**:
  - REAPER (efficient for plugin testing)
  - JUCE AudioPluginHost (included with JUCE)

- **Graphics Tools**:
  - Figma/Adobe XD for UI prototyping
  - SVG editor for interface assets

## JUCE Installation

### Method 1: Using Git

```bash
# Clone the JUCE repository
git clone https://github.com/juce-framework/JUCE.git

# Enter the JUCE directory
cd JUCE

# Checkout stable version
git checkout 7.0.5  # Use the desired version
```

### Method 2: Downloading Release

1. Visit the [JUCE Releases Page](https://github.com/juce-framework/JUCE/releases)
2. Download the latest stable release
3. Extract to your preferred development location

## Project Setup

### Option 1: Using Projucer (JUCE's project management tool)

1. Open the Projucer application from your JUCE installation
2. Select "Create New Project"
3. Choose "Audio Plugin" as the project type
4. Configure project settings:
   - Project Name: "HarmonyScape"
   - Project Type: "Audio Plugin"
   - Plugin Formats: VST3, AU
   - Plugin Characteristics:
     - Plugin is a Synth: ✓
     - Plugin MIDI Input: ✓
     - Plugin MIDI Output: ✓
     - MIDI Effect Plugin: ✓
5. Choose a location to save the project
6. Select modules (all JUCE modules are needed for this project)
7. Configure exporters for target platforms (Windows, macOS, Linux)
8. Save and open in your preferred IDE

### Option 2: Using CMake (Recommended for Cross-Platform)

1. Create a new project folder
2. Create a `CMakeLists.txt` file with the following content:

```cmake
cmake_minimum_required(VERSION 3.22)

project(HarmonyScape VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Initialize JUCE
juce_add_plugin(HarmonyScape
    # Plugin Version Info
    VERSION 1.0.0
    # VST3/AU compatibility
    FORMATS VST3 AU 
    # Plugin characteristics
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    PLUGIN_MANUFACTURER_CODE Hrmy
    PLUGIN_CODE HScp
    PRODUCT_NAME "HarmonyScape"
    COMPANY_NAME "YourCompanyName"
    
    # Sources will be added here
    SOURCES
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Harmony/ChordEngine.cpp
        Source/Harmony/ChordEngine.h
        # Add more files as project grows
)

# Required JUCE modules
target_compile_definitions(HarmonyScape
    PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link with JUCE modules
target_link_libraries(HarmonyScape
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_graphics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

3. Create folders and initial files:

```bash
# Create directory structure
mkdir -p Source/Harmony Source/Spatial Source/UI Source/Utils

# Create basic files by copying templates or creating new ones
touch Source/PluginProcessor.cpp Source/PluginProcessor.h
touch Source/PluginEditor.cpp Source/PluginEditor.h
touch Source/Harmony/ChordEngine.cpp Source/Harmony/ChordEngine.h
```

4. Configure and build:

```bash
# Create build directory
mkdir build && cd build

# Generate build files
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
cmake --build . --config Debug
```

## Project Folder Structure

The recommended folder structure for the HarmonyScape project:

```
HarmonyScape/
├── CMakeLists.txt                  # Main CMake file
├── JUCE/                           # JUCE as git submodule or symlink
├── Source/                         # Main source code folder
│   ├── PluginProcessor.cpp/h       # Main audio processor
│   ├── PluginEditor.cpp/h          # Main UI component
│   ├── Harmony/                    # Harmony engine components
│   │   ├── ChordEngine.cpp/h       # Chord voicing engine
│   │   ├── VoiceLeading.cpp/h      # Voice leading algorithms
│   │   └── HarmonyTypes.h          # Shared harmony type definitions
│   ├── Spatial/                    # Spatial audio components
│   │   ├── SpatialEngine.cpp/h     # Spatial positioning engine
│   │   ├── Patterns.cpp/h          # Movement patterns
│   │   └── SpatialTypes.h          # Spatial type definitions
│   ├── UI/                         # UI components
│   │   ├── ChordSequencer.cpp/h    # Sequencer component
│   │   ├── VoicingDisplay.cpp/h    # Voicing visualization
│   │   ├── SpatialView.cpp/h       # Spatial visualization
│   │   ├── ParameterControls.cpp/h # Parameter panel
│   │   └── LookAndFeel.cpp/h       # Custom styling
│   └── Utils/                      # Utility functions and helpers
│       ├── MusicTheory.cpp/h       # Music theory helpers
│       ├── PresetManager.cpp/h     # Preset handling
│       └── AudioHelpers.cpp/h      # Audio utility functions
├── Resources/                      # Project resources
│   ├── icons/                      # UI icons (SVG preferred)
│   ├── fonts/                      # Custom fonts
│   └── presets/                    # Factory presets
├── Tests/                          # Unit and integration tests
│   ├── HarmonyTests.cpp            # Tests for harmony engine
│   ├── SpatialTests.cpp            # Tests for spatial engine
│   └── TestMain.cpp                # Test runner
└── docs/                           # Documentation
    ├── HarmonyScape_Project_Document.md       # Main project spec
    ├── HarmonyScape_ChordEngine_Algorithm.md  # Chord engine details
    ├── HarmonyScape_UI_Design.md              # UI specifications
    └── HarmonyScape_JUCE_Setup.md             # This setup guide
```

## Base Classes Implementation

### Core Plugin Classes

The starting point for HarmonyScape will include these essential classes:

1. **HarmonyScapeProcessor** (extends `juce::AudioProcessor`)
   - Main DSP processing
   - Parameter management
   - State handling

2. **HarmonyScapeEditor** (extends `juce::AudioProcessorEditor`)
   - Main UI component
   - Layout management
   - Component coordination

### Core Domain Classes

1. **ChordEngine**
   - Chord detection and representation
   - Voicing generation
   - Voice leading optimization

2. **SpatialEngine**
   - Voice positioning in spatial field
   - Movement pattern application
   - Stereo/3D format support

3. **PresetManager**
   - Preset loading/saving
   - Metadata handling
   - Categorization

## Build and Run

### Debug Build and Execution

```bash
# In your build directory
cmake --build . --config Debug

# The plugin will be in a location like:
# Windows: build/HarmonyScape_artefacts/Debug/VST3/HarmonyScape.vst3
# macOS: build/HarmonyScape_artefacts/Debug/VST3/HarmonyScape.vst3
# Linux: build/HarmonyScape_artefacts/Debug/VST3/HarmonyScape.vst3
```

To test the plugin:
1. Open JUCE's AudioPluginHost or a DAW like REAPER
2. Scan for new plugins, pointing to your build directory
3. Load the HarmonyScape plugin

### Release Build

```bash
# Clean build directory for release
rm -rf build && mkdir build && cd build

# Configure for release
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build release version
cmake --build . --config Release
```

## Development Workflow

1. Implement new features in focused branches
2. Write unit tests for new functionality
3. Run tests before committing changes
4. Keep the main branch stable
5. Follow semantic versioning for releases

## Plugin Structure

The plugin will use a modular architecture:

- **PluginProcessor**: Core audio processing and parameter handling
- **PluginEditor**: Main UI container
- **ChordEngine**: Handles all harmony generation
- **SpatialEngine**: Manages spatial positioning and automation
- **PresetManager**: Handles preset storage and retrieval

## Debugging Tips

1. Use JUCE's debugging tools:
   ```cpp
   DBG("Debug message: " + String(value));
   ```

2. Enable the JUCE Assertion macros for development:
   ```cpp
   jassert(condition); // Will break in debugger if false
   ```

3. Test regularly with the AudioPluginHost to isolate DAW-specific issues

4. Add logging for complex algorithms to trace execution flow

## Next Steps

1. Implement the core classes outlined in this document
2. Create basic UI layout following the UI design document
3. Implement MVP features for the chord voicing engine
4. Develop basic spatial positioning functionality
5. Implement preset system for saving/loading
6. Add essential UI controls and visualizations

## Resources

- [JUCE Documentation](https://juce.com/learn/documentation)
- [JUCE Tutorials](https://juce.com/learn/tutorials)
- [JUCE Forum](https://forum.juce.com/)
- [JUCE API Reference](https://docs.juce.com/master/index.html)
- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)

By following this setup guide, developers can quickly establish a consistent development environment for the HarmonyScape project and begin implementing the features outlined in the project documentation. 