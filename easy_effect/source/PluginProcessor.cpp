namespace easy_effect {
PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JUCE_IS_MIDI_EFFECT
#if !JUCE_IS_SYNTH
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", createParameterLayout()) 
{
}

// 对于插件参数的定义，使用了AudioProcessorValueTreeState来管理参数状态，并且在构造函数中初始化了参数布局。createParameterLayout函数创建了一个名为"mix"的参数，范围从0.0到1.0，默认值为0.25，并且提供了一个lambda函数来格式化参数值为百分比字符串，以及一个lambda函数来解析百分比字符串回到参数值。
juce::AudioProcessorValueTreeState::ParameterLayout
PluginProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
  parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"mix", 1}, "Mix",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f, juce::String(),
      juce::AudioProcessorParameter::genericParameter,
      [](float value, int) {
        return juce::String(juce::roundToInt(value * 100.0f));
      },
      [](const juce::String& text) {
        return juce::jlimit(
            0.0f, 1.0f,
            text.upToFirstOccurrenceOf("%", false, false).getFloatValue() /
                100.0f);
      }));
  return {parameters.begin(), parameters.end()};
}

const juce::String PluginProcessor::getName() const {
  return JUCE_PLUGIN_NAME;
}

bool PluginProcessor::acceptsMidi() const {
#if JUCE_NEEDS_MIDI_INPUT
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JUCE_NEEDS_MIDI_OUTPUT
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JUCE_IS_MIDI_EFFECT
  return true;
#else
  return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int PluginProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
  return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void PluginProcessor::changeProgramName(int index,
                                        const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JUCE_IS_MIDI_EFFECT
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JUCE_IS_SYNTH
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  const auto totalNumOutputChannels = getTotalNumOutputChannels();
  const auto numSamples = buffer.getNumSamples();

  // 读取 mix 参数（0.0 ~ 1.0）
  const float mix = apvts.getRawParameterValue("mix")->load();

  // 测试正弦参数
  constexpr double testFrequencyHz = 440.0;
  constexpr float baseAmplitude = 0.2f;  // 防止太响
  const double sr = getSampleRate();

  // 简化测试：用 static 相位（后续可改成成员变量）
  static double phase = 0.0;
  const double phaseIncrement =
      (sr > 0.0 ? (juce::MathConstants<double>::twoPi * testFrequencyHz / sr)
                : 0.0);

  // 直接生成输出，不使用输入音频
  for (int sample = 0; sample < numSamples; ++sample) {
    const float sineSample =
        std::sin(phase) * baseAmplitude * mix;  // mix 当输出增益

    phase += phaseIncrement;
    if (phase >= juce::MathConstants<double>::twoPi) {
      phase -= juce::MathConstants<double>::twoPi;
    }

    for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
      buffer.setSample(channel, sample, sineSample);
    }
  }
}

bool PluginProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
  return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  if (auto state = apvts.copyState().createXml()) {
        copyXmlToBinary(*state, destData);
  } 
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  if (auto xmlState = getXmlFromBinary(data, sizeInBytes)) {
    if (xmlState->hasTagName(apvts.state.getType())) {
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
  }
}
}  // namespace easy_effect

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new easy_effect::PluginProcessor();
}
