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

  setSize(420, 260);
}

void PluginEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  g.drawFittedText("EasyEffect - Reverb (GUI/Parameter Stage)",
                   getLocalBounds().removeFromTop(30),
                   juce::Justification::centred, 1);
}

void PluginEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  auto area = getLocalBounds().reduced(20);
  area.removeFromTop(30);

  auto knobArea = area.withSizeKeepingCentre(160, 180);
  mixLabel.setBounds(knobArea.removeFromTop(24));
  mixSlider.setBounds(knobArea);
}
}  // namespace audio_plugin
