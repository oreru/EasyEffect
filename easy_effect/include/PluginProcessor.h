#pragma once

#include "EasyReverb.h"

namespace easy_effect {
class PluginProcessor : public juce::AudioProcessor {
public:
  PluginProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
  static juce::AudioProcessorValueTreeState::ParameterLayout
  createParameterLayout();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
  juce::AudioProcessorValueTreeState apvts;

  DiffusionStep<8> diffusion;
  MultiChannelMixedFeedback<8> feedback;
  double currentSampleRate = 44100.0;
  float lastDiffusionMsRange = -1.0f;
  float lastDecayGain = -1.0f;
};
}  // namespace audio_plugin
