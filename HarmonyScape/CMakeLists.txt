cmake_minimum_required(VERSION 3.15)

project(HarmonyScape VERSION 0.1.0)

# Add the JUCE subdirectory
add_subdirectory(JUCE JUCE_BUILD)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Initialize JUCE
juce_add_plugin(HarmonyScape
    PRODUCT_NAME "HarmonyScape"
    COMPANY_NAME "HarmonyScape"
    PLUGIN_MANUFACTURER_CODE "Hrsc"
    PLUGIN_CODE "Hrsc"
    FORMATS AU VST3 Standalone
    VST3_CATEGORIES "Instrument"
    AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
    MICROPHONE_PERMISSION_ENABLED TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_SYNTH TRUE
)

# Add source files
target_sources(HarmonyScape PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ChordEngine/ChordEngine.cpp
    Source/SpatialEngine/SpatialEngine.cpp
    Source/RibbonEngine/RibbonEngine.cpp
)

# Add binary data
# juce_add_binary_data(HarmonyScapeData SOURCES
#     Resources/logo.png
# )

# Link to the appropriate JUCE modules
target_compile_definitions(HarmonyScape
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HarmonyScape
    PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    # HarmonyScapeData
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
) 