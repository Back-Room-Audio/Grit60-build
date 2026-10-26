cmake_minimum_required(VERSION 3.22)
project(Grit60 VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(GRIT_FORMATS AU VST3 Standalone)
else()
    set(GRIT_FORMATS VST3 Standalone)
endif()

option(GRIT_COPY_AFTER_BUILD "Copy plugin to system folder after build" ON)

juce_add_plugin(Grit60
    COMPANY_NAME "Tapework"
    PLUGIN_MANUFACTURER_CODE Tpwk
    PLUGIN_CODE G60a
    FORMATS ${GRIT_FORMATS}
    PRODUCT_NAME "Grit60"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${GRIT_COPY_AFTER_BUILD})

target_sources(Grit60 PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(Grit60 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Grit60
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
