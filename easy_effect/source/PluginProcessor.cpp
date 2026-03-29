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

  diffusion.configure(sampleRate);
  feedback.configure(sampleRate);
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
  const auto totalNumInputChannels = getTotalNumInputChannels();
  const auto totalNumOutputChannels = getTotalNumOutputChannels();
  const auto numSamples = buffer.getNumSamples();

    // 清除输入以外的输出通道，防止产生垃圾噪音
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, numSamples);

  if (totalNumInputChannels == 0)
    return;

  // 读取 mix 参数（0.0 ~ 1.0）
  const float mix = apvts.getRawParameterValue("mix")->load();

  auto* leftChannelData = buffer.getWritePointer(0);
  // 防止单声道输入导致越界崩溃
  auto* rightChannelData =
      totalNumInputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
    // 读取原始干信号 (Dry)
    float dryLeft = leftChannelData[sampleIdx];
    float dryRight =
        rightChannelData != nullptr ? rightChannelData[sampleIdx] : dryLeft;

    // 1. Upmix: 将双声道干信号映射到 8 声道网格中
    std::array<double, 8> channels{};
    channels[0] = static_cast<double>(dryLeft);
    channels[1] = static_cast<double>(dryRight);
    // 为了使初始混响更饱满，可以将输入稍微分配给其他通道
    channels[2] = channels[0] * 0.5;
    channels[3] = channels[1] * 0.5;

    // 2. 处理: 通过你编写的 DSP 组件
    channels = diffusion.process(channels);
    channels = diffusion.process(channels);
    channels = feedback.process(channels);

    // 3. Downmix: 将 8 声道湿信号 (Wet) 混缩回双声道
    // (乘以 0.5 是为了稍微衰减防爆音，你可以根据情况调整)
    double wetLeft =
        (channels[0] + channels[2] + channels[4] + channels[6]) * 0.5;
    double wetRight =
        (channels[1] + channels[3] + channels[5] + channels[7]) * 0.5;

    // 4. 干湿混合 (Dry / Wet Mix): 线性插值
    leftChannelData[sampleIdx] =
        static_cast<float>(dryLeft * (1.0f - mix) + wetLeft * mix);

    if (rightChannelData != nullptr) {
      rightChannelData[sampleIdx] =
          static_cast<float>(dryRight * (1.0f - mix) + wetRight * mix);
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
