# =============================================================================
#  RR-1 Lernsynth – Bauanleitung für CMake
#  Diese Datei sagt dem Computer, aus welchen Dateien das Plugin besteht und
#  welche Formate (AU, VST3, eigenständige App) gebaut werden sollen.
#  Das Framework JUCE wird beim ersten Bauen automatisch von GitHub geladen.
# =============================================================================
cmake_minimum_required(VERSION 3.22)
project(RR1 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Auf dem Mac: Plugin läuft auf Apple-Silicon und Intel, ab macOS 11
if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
    endif()
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# AU (Audio Unit) gibt es nur auf dem Mac
if(APPLE)
    set(RR1_FORMATE AU VST3 Standalone)
else()
    set(RR1_FORMATE VST3 Standalone)
endif()

juce_add_plugin(RR1
    COMPANY_NAME                "Ralf"
    BUNDLE_ID                   "de.ralf.rr1lernsynth"
    PLUGIN_MANUFACTURER_CODE    Rrlf
    PLUGIN_CODE                 Rr1s
    FORMATS                     ${RR1_FORMATE}
    PRODUCT_NAME                "RR-1 Lernsynth"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Synth
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice)

set(RR1_QUELLEN
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_sources(RR1 PRIVATE ${RR1_QUELLEN})

set(RR1_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_definitions(RR1 PUBLIC ${RR1_DEFINES})

target_link_libraries(RR1
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# -----------------------------------------------------------------------------
#  Optional: Testprogramm (nur zum Prüfen, wird für das Plugin nicht gebraucht)
#  Einschalten mit  -DRR1_TESTS=ON
# -----------------------------------------------------------------------------
option(RR1_TESTS "Testprogramm mitbauen" OFF)
if(RR1_TESTS)
    juce_add_console_app(RR1Test PRODUCT_NAME "RR1Test")
    target_sources(RR1Test PRIVATE ${RR1_QUELLEN} Tests/TestMain.cpp)
    target_compile_definitions(RR1Test PRIVATE ${RR1_DEFINES} JUCE_STANDALONE_APPLICATION=1)
    target_link_libraries(RR1Test PRIVATE juce::juce_audio_utils juce::juce_audio_formats
        juce::juce_recommended_config_flags)
endif()
