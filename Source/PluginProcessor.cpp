#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

static inline float clampf (float v, float lo, float hi){ return std::max(lo, std::min(hi, v)); }

VoiceChangerAudioProcessor::VoiceChangerAudioProcessor()
: AudioProcessor (BusesProperties()
    .withInput  ("Input",  AudioChannelSet::stereo(), true)
    .withOutput ("Output", AudioChannelSet::stereo(), true))
{
    apvts = std::make_unique<AudioProcessorValueTreeState> (*this, nullptr, "PARAMS", createLayout());
}

AudioProcessorValueTreeState::ParameterLayout VoiceChangerAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat>("pitch",   "Pitch (semitones)", NormalisableRange<float> (-12.f, 12.f, 0.01f), 0.0f));
    p.push_back (std::make_unique<AudioParameterFloat>("length",  "Formant Length",    NormalisableRange<float> (0.70f, 1.40f, 0.001f), 1.00f));
    p.push_back (std::make_unique<AudioParameterFloat>("thick",   "Thickness (Tilt)",  NormalisableRange<float> (-12.f, 12.f, 0.1f), 0.0f));
    p.push_back (std::make_unique<AudioParameterFloat>("pivot",   "Tilt Pivot (Hz)",   NormalisableRange<float> (300.f, 3000.f, 1.f, 0.3f), 1000.f));
    p.push_back (std::make_unique<AudioParameterFloat>("mix",     "Mix",               NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f));

    return { p.begin(), p.end() };
}

bool VoiceChangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet()  == AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

void VoiceChangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    channels = getTotalNumOutputChannels();

    lengthResampler.prepare (sr, channels);
    pitchShifter.prepare (sr, channels, samplesPerBlock);
    tilt.prepare (sr, channels);

    tmp1.setSize (channels, samplesPerBlock * 4);
    tmp2.setSize (channels, samplesPerBlock * 4);
}

void VoiceChangerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    auto pitchSemi = apvts->getRawParameterValue("pitch")->load();
    auto length    = apvts->getRawParameterValue("length")->load();
    auto tiltDb    = apvts->getRawParameterValue("thick")->load();
    auto pivotHz   = apvts->getRawParameterValue("pivot")->load();
    auto mixPct    = apvts->getRawParameterValue("mix")->load();

    // 1) Length（声道長）: a倍でリサンプル
    lengthResampler.setRatio (length);
    lengthResampler.process (buffer, tmp1);         // tmp1 = length-scaled

    // 2) ピッチ補正（length分を差し引く）: pitch' = pitch - 12*log2(a)
    const float correctedSemi = pitchSemi - 12.0f * std::log2 (std::max (0.001f, length));
    pitchShifter.setSemitone (correctedSemi);
    pitchShifter.process (tmp1, tmp2);              // tmp2 = formant length applied + pitch corrected

    // 3) Thickness（チルトEQ）
    tilt.update (tiltDb, pivotHz);
    tilt.process (tmp2);

    // 4) Dry/Wet
    const float wet = clampf (mixPct / 100.f, 0.f, 1.f);
    const float dry = 1.0f - wet;

    const int n = buffer.getNumSamples();
    for (int c=0; c<juce::jmin (buffer.getNumChannels(), tmp2.getNumChannels()); ++c)
    {
        auto* d = buffer.getWritePointer (c);
        const auto* w = tmp2.getReadPointer (c);
        for (int i=0; i<n; ++i)
            d[i] = dry * d[i] + wet * w[i];
    }
}

AudioProcessorEditor* VoiceChangerAudioProcessor::createEditor()
{
    return new VoiceChangerAudioProcessorEditor(*this);
}
