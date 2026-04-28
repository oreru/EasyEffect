namespace easy_effect {
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
  mixSlider.setTextValueSuffix(" %");
  addAndMakeVisible(mixSlider);

  mixLabel.setText("Mix", juce::dontSendNotification);
  mixLabel.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(mixLabel);

  mixAttachment = std::make_unique<SliderAttachment>(processorRef.getAPVTS(),
                                                     "mix", mixSlider);

  diffusionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  diffusionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
  diffusionSlider.setTextValueSuffix(" ms");
  addAndMakeVisible(diffusionSlider);

  diffusionLabel.setText("Diffusion", juce::dontSendNotification);
  diffusionLabel.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(diffusionLabel);

  diffusionAttachment = std::make_unique<SliderAttachment>(
      processorRef.getAPVTS(), "diffusion", diffusionSlider);

  decaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
  decaySlider.setTextValueSuffix(" %");
  addAndMakeVisible(decaySlider);

  decayLabel.setText("Decay", juce::dontSendNotification);
  decayLabel.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(decayLabel);

  decayAttachment =
      std::make_unique<SliderAttachment>(processorRef.getAPVTS(), "decay",
                                         decaySlider);

  setSize(560, 260);
}

void PluginEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  g.drawFittedText("Reverb that sounds like shit",
                   getLocalBounds().removeFromTop(30),
                   juce::Justification::centred, 1);
}

void PluginEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  auto area = getLocalBounds().reduced(20);
  area.removeFromTop(30);

  auto knobArea = area.withSizeKeepingCentre(420, 180);
  auto columnWidth = knobArea.getWidth() / 3;

  auto mixArea = knobArea.removeFromLeft(columnWidth);
  mixLabel.setBounds(mixArea.removeFromTop(24));
  mixSlider.setBounds(mixArea);

  auto diffusionArea = knobArea.removeFromLeft(columnWidth);
  diffusionLabel.setBounds(diffusionArea.removeFromTop(24));
  diffusionSlider.setBounds(diffusionArea);

  auto decayArea = knobArea;
  decayLabel.setBounds(decayArea.removeFromTop(24));
  decaySlider.setBounds(decayArea);
}
}  // namespace audio_plugin
