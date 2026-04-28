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

  juce::Slider diffusionSlider;
  juce::Label diffusionLabel;
  std::unique_ptr<SliderAttachment> diffusionAttachment;

  juce::Slider decaySlider;
  juce::Label decayLabel;
  std::unique_ptr<SliderAttachment> decayAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
}  // namespace audio_plugin
