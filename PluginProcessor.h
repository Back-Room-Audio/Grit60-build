#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

class Grit60Processor : public juce::AudioProcessor
{
public:
    Grit60Processor();
    ~Grit60Processor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Grit60"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::atomic<float> *bitsP, *rateP, *driveP, *compandP, *noiseP, *toneP,
                       *mixP, *outP, *aliasP, *tdriveP, *wobbleP, *bumpP;

    double sampleRate = 44100.0;
    int currentProgram = 0;

    // sampler section state
    float phase[2] { 0.f, 0.f };
    float held[2]  { 0.f, 0.f };
    juce::dsp::StateVariableTPTFilter<float> preFilter, postFilter;
    juce::Random rng;

    // tape section state
    juce::dsp::StateVariableTPTFilter<float> bumpFilter;
    std::vector<float> delayBuf[2];
    int writePos[2] { 0, 0 };
    float wowPh[2]  { 0.f, 0.3f };
    float flutPh[2] { 0.f, 0.45f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grit60Processor)
};
