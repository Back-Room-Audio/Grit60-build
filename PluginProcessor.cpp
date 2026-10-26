#include "PluginProcessor.h"
#include "PluginEditor.h"

using Range = juce::NormalisableRange<float>;
using FloatParam = juce::AudioParameterFloat;

// ---- Presets -------------------------------------------------------------
static const char* kIds[12] = { "bits", "rate", "drive", "compand", "noise", "tone",
                                "mix", "out", "alias", "tdrive", "wobble", "bump" };

struct Preset { const char* name; float v[12]; };
//                              bits  rate   drive comp  noise  tone   mix  out alias tdrv wob  bump
static const Preset kPresets[] = {
    { "Default 12-bit",        { 12, 40000, 0.00f, 0.4f, -96.f, 14000, 1.f, 0.f, 0, 0.30f, 0.15f, 2.f } },
    { "Clean Glue",            { 16, 48000, 0.10f, 0.0f, -96.f, 18000, 1.f, 0.f, 0, 0.25f, 0.05f, 2.f } },
    { "MPC60 Classic",         { 12, 40000, 0.15f, 0.4f, -96.f, 14000, 1.f, 0.f, 0, 0.15f, 0.00f, 1.5f } },
    { "Dusty Sampler",         { 10, 26000, 0.30f, 0.6f, -78.f,  9000, 1.f, 0.f, 1, 0.40f, 0.25f, 3.f } },
    { "Tape Warmth",           { 16, 48000, 0.00f, 0.0f, -96.f, 16000, 1.f, 0.f, 0, 0.50f, 0.15f, 3.f } },
    { "Crushed Drums",         {  8, 22050, 0.40f, 0.5f, -84.f, 11000, 0.7f, -2.f, 1, 0.30f, 0.05f, 2.f } },
};
static const int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));

// ---- Parameters ----------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout Grit60Processor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "bits", 1 },    "Bits",        Range (4.f, 16.f, 1.f), 12.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "rate", 1 },    "Sample Rate", Range (2000.f, 48000.f, 1.f, 0.4f), 40000.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "drive", 1 },   "Drive",       Range (0.f, 1.f, 0.001f), 0.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "compand", 1 }, "Compand",     Range (0.f, 1.f, 0.001f), 0.4f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "noise", 1 },   "Noise",       Range (-96.f, -40.f, 0.1f), -96.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "tone", 1 },    "Tone",        Range (1000.f, 20000.f, 1.f, 0.4f), 14000.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "mix", 1 },     "Mix",         Range (0.f, 1.f, 0.001f), 1.f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "out", 1 },     "Output",      Range (-24.f, 6.f, 0.1f), 0.f));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "alias", 1 }, "Alias Mode", false));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "tdrive", 1 },  "Tape Drive",  Range (0.f, 1.f, 0.001f), 0.3f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "wobble", 1 },  "Tape Wobble", Range (0.f, 1.f, 0.001f), 0.15f));
    l.add (std::make_unique<FloatParam> (juce::ParameterID { "bump", 1 },    "Head Bump",   Range (0.f, 6.f, 0.1f), 2.f));
    return l;
}

Grit60Processor::Grit60Processor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    bitsP    = apvts.getRawParameterValue ("bits");
    rateP    = apvts.getRawParameterValue ("rate");
    driveP   = apvts.getRawParameterValue ("drive");
    compandP = apvts.getRawParameterValue ("compand");
    noiseP   = apvts.getRawParameterValue ("noise");
    toneP    = apvts.getRawParameterValue ("tone");
    mixP     = apvts.getRawParameterValue ("mix");
    outP     = apvts.getRawParameterValue ("out");
    aliasP   = apvts.getRawParameterValue ("alias");
    tdriveP  = apvts.getRawParameterValue ("tdrive");
    wobbleP  = apvts.getRawParameterValue ("wobble");
    bumpP    = apvts.getRawParameterValue ("bump");
}

// ---- Programs (presets) --------------------------------------------------
int Grit60Processor::getNumPrograms() { return kNumPresets; }

const juce::String Grit60Processor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, kNumPresets) ? kPresets[index].name : "";
}

void Grit60Processor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, kNumPresets))
        return;

    currentProgram = index;
    for (int i = 0; i < 12; ++i)
        if (auto* p = apvts.getParameter (kIds[i]))
            p->setValueNotifyingHost (p->convertTo0to1 (kPresets[index].v[i]));
}

// ---- Audio ---------------------------------------------------------------
void Grit60Processor::prepareToPlay (double sr, int blockSize)
{
    sampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockSize, 2 };

    for (auto* f : { &preFilter, &postFilter, &bumpFilter })
    {
        f->prepare (spec);
        f->setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        f->setResonance (0.7071f);
        f->reset();
    }
    bumpFilter.setCutoffFrequency (90.f);

    const int size = (int) (sr * 0.02) + 8;
    for (int c = 0; c < 2; ++c)
    {
        delayBuf[c].assign ((size_t) size, 0.f);
        writePos[c] = 0;
        phase[c] = 0.f;
        held[c] = 0.f;
    }

    // tape wobble needs a small fixed delay; tell the DAW so it can compensate
    setLatencySamples ((int) (0.004 * sr));
}

bool Grit60Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void Grit60Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const float sr = (float) sampleRate;

    const float bits     = bitsP->load();
    const float rate     = rateP->load();
    const float drive    = driveP->load();
    const float compand  = compandP->load();
    const float noiseDb  = noiseP->load();
    const float tone     = toneP->load();
    const float mix      = mixP->load();
    const float outGain  = juce::Decibels::decibelsToGain (outP->load());
    const bool  alias    = aliasP->load() > 0.5f;
    const float tdrive   = tdriveP->load();
    const float wobble   = wobbleP->load();
    const float bumpDb   = bumpP->load();

    const float levels    = std::pow (2.f, bits - 1.f);
    const float step      = juce::jmin (rate / sr, 1.f);
    const float driveG    = 1.f + drive * 6.f;
    const float driveNorm = 1.f / std::tanh (driveG);
    const float mu        = compand > 0.001f ? 1.f + compand * 254.f : 0.f;
    const float logMu     = mu > 0.f ? std::log1p (mu) : 1.f;
    const float noiseAmp  = noiseDb <= -95.f ? 0.f : juce::Decibels::decibelsToGain (noiseDb);

    // tape constants
    const float bumpK     = juce::Decibels::decibelsToGain (bumpDb) - 1.f;
    const float tapeG     = 1.f + tdrive * 4.f;
    const float tapeBias  = 0.08f * tdrive;
    const float tapeOff   = std::tanh (tapeG * tapeBias);
    const float baseDelay = 0.004f * sr;
    const float wowDepth  = 0.0015f * sr * wobble;
    const float flutDepth = 0.0002f * sr * wobble;
    const float wowInc    = 0.6f / sr;
    const float flutInc   = 6.3f / sr;
    const float twoPi     = juce::MathConstants<float>::twoPi;

    preFilter.setCutoffFrequency  (juce::jmin (rate * 0.45f, sr * 0.45f));
    postFilter.setCutoffFrequency (juce::jmin (tone, sr * 0.45f));

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        auto& buf = delayBuf[ch];
        const int size = (int) buf.size();

        for (int i = 0; i < numSamples; ++i)
        {
            const float dry = data[i];

            // ---- sampler section ----
            float x = drive > 0.001f ? std::tanh (dry * driveG) * driveNorm : dry;

            if (! alias)
                x = preFilter.processSample (ch, x);

            phase[ch] += step;
            if (phase[ch] >= 1.f)
            {
                phase[ch] -= 1.f;

                float s = juce::jlimit (-1.f, 1.f, x);
                if (noiseAmp > 0.f)
                    s += (rng.nextFloat() * 2.f - 1.f) * noiseAmp;

                const float sign = s < 0.f ? -1.f : 1.f;
                float a = juce::jmin (std::abs (s), 1.f);
                if (mu > 0.f)
                    a = std::log1p (mu * a) / logMu;

                a = std::round (a * levels) / levels;

                if (mu > 0.f)
                    a = (std::pow (1.f + mu, a) - 1.f) / mu;

                held[ch] = sign * a;
            }

            const float wet = postFilter.processSample (ch, held[ch]);
            float y = dry * (1.f - mix) + wet * mix;

            // ---- tape section ----
            // head bump: low-frequency lift
            if (bumpK > 0.f)
                y += bumpK * bumpFilter.processSample (ch, y);

            // tape saturation: gentle peak compression with a touch of even harmonics
            if (tdrive > 0.001f)
                y = (std::tanh (tapeG * (y + tapeBias)) - tapeOff) / tapeG;

            // wow & flutter: slowly modulated delay
            buf[(size_t) writePos[ch]] = y;
            const float d = baseDelay
                          + wowDepth  * std::sin (twoPi * wowPh[ch])
                          + flutDepth * std::sin (twoPi * flutPh[ch]);

            float rp = (float) writePos[ch] - d;
            while (rp < 0.f) rp += (float) size;
            const int i0 = (int) rp;
            const float fr = rp - (float) i0;
            const int i1 = (i0 + 1) % size;
            y = buf[(size_t) i0] * (1.f - fr) + buf[(size_t) i1] * fr;

            writePos[ch] = (writePos[ch] + 1) % size;
            wowPh[ch]  += wowInc;  if (wowPh[ch]  >= 1.f) wowPh[ch]  -= 1.f;
            flutPh[ch] += flutInc; if (flutPh[ch] >= 1.f) flutPh[ch] -= 1.f;

            data[i] = y * outGain;
        }
    }
}

juce::AudioProcessorEditor* Grit60Processor::createEditor() { return new Grit60Editor (*this); }

void Grit60Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void Grit60Processor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Grit60Processor(); }
