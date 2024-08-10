#include "PluginProcessor.h"

#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              // discreteChannels doesn't work well with VST3
              .withInput("Input", juce::AudioChannelSet::ambisonic(7), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  startTimer(100);
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
  update_DSP();
}

void PluginProcessor::releaseResources() { iamfbr_dsp_.deinitialize(); }

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;  // Verify if this is necessary

  if (plugin_state_.needs_update_DSP) return;
  if (plugin_state_.needs_update_encoding_matrix) return;

  iamfbr_dsp_.process(buffer);
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

void PluginProcessor::update_DSP() {
  auto buffer_size = static_cast<size_t>(getBlockSize());
  int sampling_rate = static_cast<int>(getSampleRate());

  if (plugin_state_.needs_update_DSP) {
    iamfbr_dsp_.update_DSP_config(buffer_size, sampling_rate, plugin_state_);
    plugin_state_.needs_update_DSP = false;
  }

  if (plugin_state_.needs_update_encoding_matrix) {
    for (size_t i = 0; i < plugin_state_.sources.size(); i++) {
      iamfbr_dsp_.update_source(
          static_cast<int>(i), plugin_state_.sources[i].gain,
          plugin_state_.sources[i].azimuth, plugin_state_.sources[i].elevation,
          plugin_state_.sources[i].distance);
    }
    plugin_state_.needs_update_encoding_matrix = false;
  }
}

void PluginProcessor::timerCallback() {
  if (timer_callbacks_to_wait > 0) {
    timer_callbacks_to_wait--;
    return;
  } else if (timer_callbacks_to_wait == 0) {
    // SETUP INITIAL SETTINGS
    plugin_state_.input_type = InputType::Individual_sources;
    plugin_state_.ambisonic_order = 7;
    plugin_state_.selected_preset_id = 1;
    plugin_state_.needs_update_DSP = true;

    // setup individual sources
    plugin_state_.sources.push_back({"source1", 1.0f, 30.0f, 0.0f, 1.0f});
    plugin_state_.sources.push_back({"source2", 1.0f, -30.0f, 0.0f, 1.0f});
    plugin_state_.sources.push_back({"source3", 1.0f, 0.0f, 0.0f, 1.0f});
    plugin_state_.sources.push_back({"source4", 1.0f, 0.0f, 0.0f, 1.0f});
    plugin_state_.needs_update_encoding_matrix = true;
    plugin_state_.needs_update_UI = true;
    timer_callbacks_to_wait = -1;
  }

  update_DSP();
}