#include "PluginProcessor.h"

#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              // discreteChannels doesn't work well with VST3
              .withInput("Input", juce::AudioChannelSet::ambisonic(7), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  startTimer(timer_rate);
}

const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const { return 0.0; }

int PluginProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int PluginProcessor::getCurrentProgram() { return 0; }

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

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  iamfbr_ = std::make_unique<iamfbr::IamfbrImpl>(samplesPerBlock, sampleRate);
  startTimer(timer_rate);
}

void PluginProcessor::releaseResources() {
  stopTimer();
  iamfbr_.reset();
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;  // Verify if this is necessary

  if (!iamfbr_) return;

  // Get the number of channels and samples per channel.
  auto numChannels = static_cast<size_t>(buffer.getNumChannels());
  auto numSamples = static_cast<size_t>(buffer.getNumSamples());

  // Declare input and output buffers.
  std::vector<std::vector<float>> input_buffer(
      numChannels, std::vector<float>(numSamples, 0.0f));
  std::vector<std::vector<float>> output_buffer(
      numChannels, std::vector<float>(numSamples, 0.0f));

  // Copy data from juce::AudioBuffer to input_buffer.
  for (size_t channel = 0; channel < numChannels; ++channel) {
    const float* source = buffer.getReadPointer(static_cast<int>(channel));
    std::copy(source, source + numSamples, input_buffer[channel].begin());
  }

//  output_buffer = input_buffer;
    iamfbr_->process(input_buffer, output_buffer);

  // Copy data from output_buffer to juce::AudioBuffer.
  for (size_t channel = 0; channel < numChannels; ++channel) {
    float* dest = buffer.getWritePointer(static_cast<int>(channel));
    std::copy(output_buffer[channel].begin(), output_buffer[channel].end(),
              dest);
  }
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
  return new PluginEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new PluginProcessor();
}

void PluginProcessor::timerCallback() {
  if (!iamfbr_) return;

  // Update azimuth with a rate of 30 degrees per second.
  static float azimuth = 0.0f;
  azimuth += 30.0f * (float)timer_rate / 1000.0f;

  if (azimuth >= 180.0f) {
    azimuth -= 360.0f;
  }

  for (auto index : iamfbr_->get_ids_of_audio_elements_containing_objects()) {
    int sign = index % 2 == 0 ? 1 : -1;
    iamfbr_->update_object_position(index, (float)sign * azimuth, 0.0f, 1.0f);
  }
}