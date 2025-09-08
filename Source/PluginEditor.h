#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VoiceChangerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
  VoiceChangerAudioProcessorEditor(VoiceChangerAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
  {
    setSize(400, 300);
  }

  void paint(juce::Graphics& g) override
  {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawFittedText("VoiceChanger", getLocalBounds(), juce::Justification::centred, 1);
  }

  void resized() override {}

private:
  VoiceChangerAudioProcessor& processorRef;
};
