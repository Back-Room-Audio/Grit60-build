#include "PluginEditor.h"

static const juce::Colour kBg     { 0xff1b1b1f };
static const juce::Colour kAccent { 0xffff8c1a };
static const juce::Colour kText   { 0xffd8d8d8 };

Grit60Editor::Grit60Editor (Grit60Processor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    // sampler row(s)
    addKnob ("bits",    "BITS");
    addKnob ("rate",    "SAMPLE RATE", " Hz");
    addKnob ("compand", "COMPAND");
    addKnob ("noise",   "NOISE", " dB");
    addKnob ("tone",    "TONE", " Hz");
    addKnob ("drive",   "DRIVE");
    addKnob ("mix",     "MIX");
    addKnob ("out",     "OUTPUT", " dB");
    // tape row
    addKnob ("tdrive",  "TAPE DRIVE");
    addKnob ("wobble",  "WOBBLE");
    addKnob ("bump",    "HEAD BUMP", " dB");

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.setTextWhenNothingSelected ("Presets");
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    aliasButton.setColour (juce::ToggleButton::textColourId, kText);
    aliasButton.setColour (juce::ToggleButton::tickColourId, kAccent);
    addAndMakeVisible (aliasButton);
    aliasAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "alias", aliasButton);

    setSize (640, 520);
}

void Grit60Editor::addKnob (const juce::String& id, const juce::String& name, const juce::String& suffix)
{
    auto* k = knobs.add (new Knob());
    k->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    k->slider.setTextValueSuffix (suffix);
    k->slider.setColour (juce::Slider::rotarySliderFillColourId, kAccent);
    k->slider.setColour (juce::Slider::thumbColourId, kAccent);
    k->slider.setColour (juce::Slider::textBoxTextColourId, kText);
    k->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (k->slider);

    k->label.setText (name, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setColour (juce::Label::textColourId, kText);
    k->label.setFont (juce::FontOptions (13.f, juce::Font::bold));
    addAndMakeVisible (k->label);

    k->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->slider);
}

void Grit60Editor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);
    g.setColour (kAccent);
    g.setFont (juce::FontOptions (30.f, juce::Font::bold));
    g.drawText ("GRIT 60", 20, 12, 300, 36, juce::Justification::centredLeft);
    g.setColour (kText.withAlpha (0.6f));
    g.setFont (juce::FontOptions (13.f));
    g.drawText ("12-bit sampler + tape  |  Tapework", 20, 46, 400, 18, juce::Justification::centredLeft);
}

void Grit60Editor::resized()
{
    auto area = getLocalBounds().reduced (20);
    presetBox.setBounds (area.getRight() - 200, 18, 200, 28);
    area.removeFromTop (70);
    auto bottom = area.removeFromBottom (30);
    aliasButton.setBounds (bottom.removeFromLeft (320));

    const int cols = 4, rows = 3;
    const int w = area.getWidth() / cols, h = area.getHeight() / rows;

    for (int i = 0; i < knobs.size(); ++i)
    {
        auto cell = juce::Rectangle<int> (area.getX() + (i % cols) * w,
                                          area.getY() + (i / cols) * h, w, h).reduced (6);
        knobs[i]->label.setBounds (cell.removeFromTop (18));
        knobs[i]->slider.setBounds (cell);
    }
}
