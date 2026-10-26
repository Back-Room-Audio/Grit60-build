#pragma once
#include "PluginProcessor.h"

class Grit60Editor : public juce::AudioProcessorEditor
{
public:
    explicit Grit60Editor (Grit60Processor&);
    ~Grit60Editor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void addKnob (const juce::String& id, const juce::String& name, const juce::String& suffix = {});

    Grit60Processor& proc;
    juce::OwnedArray<Knob> knobs;
    juce::ComboBox presetBox;
    juce::ToggleButton aliasButton { "ALIAS MODE (no anti-alias filter)" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aliasAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grit60Editor)
};
