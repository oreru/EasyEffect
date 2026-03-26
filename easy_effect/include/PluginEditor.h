#pragma once

namespace easy_effect {
class PluginEditor : public juce::AudioProcessorEditor {
public:
  explicit PluginEditor(PluginProcessor&);

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PluginProcessor& processorRef;

  juce::Slider mixSlider;
  juce::Label mixLabel;
  std::unique_ptr<SliderAttachment> mixAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
}  // namespace audio_plugin
