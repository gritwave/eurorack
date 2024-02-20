project(gritwave-gritrack VERSION ${CMAKE_PROJECT_VERSION})

FetchContent_Declare(JUCE GIT_REPOSITORY "https://github.com/juce-framework/JUCE" GIT_TAG "develop" GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

set_directory_properties(PROPERTIES JUCE_COMPANY_COPYRIGHT "Copyright 2024 gritwave. All rights reserved.")
set_directory_properties(PROPERTIES JUCE_COMPANY_NAME "gritwave")
set_directory_properties(PROPERTIES JUCE_VST3_COPY_DIR "${CMAKE_BINARY_DIR}/VST3")
set_directory_properties(PROPERTIES JUCE_AU_COPY_DIR "${CMAKE_BINARY_DIR}/AU")

juce_add_plugin(Gritrack
    PRODUCT_NAME "Gritrack"                 # The name of the final executable, which can differ from the target name
    BUNDLE_ID "com.gritwave.gritrack"       # Should uniquely identify this target
    PLUGIN_CODE Grtr                        # A unique four-character plugin id with exactly one upper-case character
    PLUGIN_MANUFACTURER_CODE Grtw           # A four-character manufacturer id with at least one upper-case character
    FORMATS AU VST3 Standalone              # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    HARDENED_RUNTIME_ENABLED TRUE           # Enables macOS' hardened runtime for this target. Required for notarisation.
    COPY_PLUGIN_AFTER_BUILD TRUE            # Should the plugin be installed to a default location after building?
    IS_SYNTH FALSE                          # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE                  # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE                 # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                    # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE       # Does the editor need keyboard focus?
)

set(Gritrack_Sources
    "src/PluginEditor.hpp"
    "src/PluginEditor.cpp"
    "src/PluginParameter.hpp"
    "src/PluginParameter.cpp"
    "src/PluginProcessor.hpp"
    "src/PluginProcessor.cpp"
)

target_sources(Gritrack PRIVATE ${Gritrack_Sources})
set_target_properties(Gritrack PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)
set_source_files_properties(Gritrack ${Gritrack_Sources} PROPERTIES UNITY_GROUP "src")

target_compile_definitions(Gritrack
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_MP3AUDIOFORMAT=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_REPORT_APP_USAGE=0
        JUCE_DISABLE_JUCE_VERSION_PRINTING=1
)

target_include_directories(Gritrack
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(Gritrack
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp

    PUBLIC
        gritwave::eurorack
        juce::juce_recommended_config_flags
        # juce::juce_recommended_lto_flags
        # juce::juce_recommended_warning_flags
)
