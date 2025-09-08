#pragma once
#include <JuceHeader.h>
#include "Dsp/GranularPitchShifter.h"
#include "Dsp/Formant.h"
#include "Dsp/Tilt.h"

class VoiceChangerAudioProcessor : public juce::AudioProcessor
{
public:
    VoiceChangerAudioProcessor();
    ~VoiceChangerAudioProcessor() override = default;

    // AudioProcessor
    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // GUI
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Info
    const juce::String getName() const override { return "VoiceChanger"; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    // Programs (unused)
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // State
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    // Params
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    // DSP blocks
    LengthResampler lengthResampler;
    GranularPitchShifter pitchShifter;
    TiltEQ tilt;

    // Utility
    juce::AudioBuffer<float> tmp1, tmp2;
    int channels = 2;
    double sr = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChangerAudioProcessor)
};
