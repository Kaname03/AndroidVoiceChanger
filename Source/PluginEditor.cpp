#include "PluginEditor.h"

VoiceChangerAudioProcessorEditor::VoiceChangerAudioProcessorEditor(VoiceChangerAudioProcessor& p)
  : AudioProcessorEditor(&p), processorRef(p)
{
  setSize(400, 300);
}
