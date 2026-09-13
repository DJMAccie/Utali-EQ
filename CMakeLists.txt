cmake_minimum_required(VERSION 3.22)

project(UTALITEQ VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE path - check if defined, in build cache, or fallback to FetchContent
if (NOT DEFINED JUCE_PATH)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
        set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    elseif (EXISTS "/Users/user/Documents/Utali-Reverbus/build/_deps/juce-src")
        set(JUCE_PATH "/Users/user/Documents/Utali-Reverbus/build/_deps/juce-src")
    elseif (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/_deps/juce-src")
        set(JUCE_PATH "${CMAKE_CURRENT_BINARY_DIR}/_deps/juce-src")
    elseif (EXISTS "${CMAKE_BINARY_DIR}/_deps/juce-src")
        set(JUCE_PATH "${CMAKE_BINARY_DIR}/_deps/juce-src")
    endif()
endif()

if (DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}")
    add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Determine whether to copy plugin binaries to system directories after build
# (Disabled in CI environments to prevent permission issues)
if (DEFINED ENV{CI})
    set(COPY_AFTER_BUILD FALSE)
else()
    set(COPY_AFTER_BUILD TRUE)
endif()

# Binary resources (images)
juce_add_binary_data(Assets
    HEADER_NAME "BinaryData.h"
    NAMESPACE "BinaryData"
    SOURCES
        "Assets/teq_panel.png"
)

# Plugin settings - VST3 only per user request
juce_add_plugin(UTALITEQ
    COMPANY_NAME "UTALI"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Utal
    PLUGIN_CODE Uteq
    FORMATS VST3
    PRODUCT_NAME "UTALITEQ"
    DESCRIPTION "Vintage Analog Character Equalizer"
)

# Source files
target_sources(UTALITEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.cpp
        Source/Parameters.h
        Source/DSP/EqualizerEngine.h
        Source/DSP/SaturationProcessor.h
        Source/UI/UtaliLookAndFeel.h
)

# Preprocessor definitions
target_compile_definitions(UTALITEQ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# Link JUCE modules
target_link_libraries(UTALITEQ
    PRIVATE
        Assets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(UTALITEQ)

# Automatically copy VST3 to ~/Documents/plugins for DAW testing when built locally
if (TARGET UTALITEQ_VST3 AND NOT DEFINED ENV{CI})
    add_custom_command(TARGET UTALITEQ_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Documents/plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:UTALITEQ_VST3>"
            "$ENV{HOME}/Documents/plugins/UTALITEQ.vst3"
        COMMENT "Copying VST3 to ~/Documents/plugins/"
        VERBATIM
    )
endif()
