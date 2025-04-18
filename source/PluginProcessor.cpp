#include "PluginProcessor.h"

#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              // discreteChannels doesn't work well with VST3
              .withInput("Input", juce::AudioChannelSet::ambisonic(7), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, &undo_manager, "PARAMETERS", createParameterLayout()) {
  // Add parameters.
  parameters.addParameterListener("audio_element_type", this);
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
  iamfbr_ = std::make_unique<obr::ObrImpl>(samplesPerBlock, sampleRate);

  auto value =
      parameters.getParameter("audio_element_type")->getCurrentValueAsText();
  setAudioElementType(value.toStdString());
}

void PluginProcessor::releaseResources() { iamfbr_.reset(); }

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;  // TODO: Verify if this is necessary.

  // Get the number of channels and samples per channel.
  auto numChannels = static_cast<size_t>(buffer.getNumChannels());
  auto numSamples = static_cast<size_t>(buffer.getNumSamples());

  // Check if the iamfbr is initialized.
  if (!iamfbr_) {
    // Clear all channels.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      buffer.clear(channel, 0, buffer.getNumSamples());
    }
    return;
  }

  // Get number of inputs and outputs of the iamfbr.
  auto numInputChannels = iamfbr_->GetNumberOfInputChannels();
  auto numOutputChannels = iamfbr_->GetNumberOfOutputChannels();

  if (numInputChannels == 0) {
    // Clear all channels.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      buffer.clear(channel, 0, buffer.getNumSamples());
    }
    return;
  }

  // Check if the bus width is too small.
  if (numInputChannels > numChannels || numOutputChannels > numChannels) {
    bus_width_too_small = true;
    // Clear all channels.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      buffer.clear(channel, 0, buffer.getNumSamples());
    }
    return;
  } else {
    bus_width_too_small = false;
  }

  // Check if the number of samples is correct.
  if (numSamples != iamfbr_->GetBufferSizePerChannel()) {
    // Clear all channels.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
      buffer.clear(channel, 0, buffer.getNumSamples());
    }
    return;
  }

  // Declare input and output buffers.
  obr::AudioBuffer input_buffer(numInputChannels, numSamples);
  obr::AudioBuffer output_buffer(numOutputChannels, numSamples);

  // Copy data from juce::AudioBuffer to iamfbr::AudioBuffer.
  for (size_t channel = 0; channel < numInputChannels; ++channel) {
    const float* source = buffer.getReadPointer(static_cast<int>(channel));
    auto& dest = input_buffer[channel];
    std::copy(source, source + numSamples, dest.begin());
  }

  iamfbr_->Process(input_buffer, &output_buffer);

  // Copy data from iamfbr::AudioBuffer to juce::AudioBuffer.
  for (size_t channel = 0; channel < numOutputChannels; ++channel) {
    const auto& source = output_buffer[channel];
    float* dest = buffer.getWritePointer(static_cast<int>(channel));
    std::copy(source.begin(), source.end(), dest);
  }

  // Clear the remaining channels.
  for (size_t channel = numOutputChannels; channel < numChannels; ++channel) {
    float* dest = buffer.getWritePointer(static_cast<int>(channel));
    std::fill(dest, dest + numSamples, 0.0f);
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
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState != nullptr)
    if (xmlState->hasTagName(parameters.state.getType()))
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void PluginProcessor::oscMessageReceived(const juce::OSCMessage& message) {
  if (message.getAddressPattern().toString() == "/quaternion" &&
      message.size() == 4) {
    qW = message[0].getFloat32();
    qX = message[1].getFloat32();
    qY = message[2].getFloat32();
    qZ = -message[3].getFloat32();

    // iamfbr_->SetHeadRotation(qW, qX, qY, qZ);
  }
}

void PluginProcessor::oscBundleReceived(const juce::OSCBundle& bundle) {
  const juce::OSCBundle::Element& elem = bundle[0];
  oscMessageReceived(elem.getMessage());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new PluginProcessor();
}

void PluginProcessor::setAudioElementType(std::string audio_element_type) {
  if (iamfbr_) {
    // Remove all audio elements.
    removeAllAudioElements();

    // Add audio element to the renderer.
    auto status = iamfbr_->AddAudioElement(
        obr::GetAudioElementTypeFromStr(audio_element_type).value());

    if (status.ok()) {
      DBG("Added audio element: " + audio_element_type);
    } else {
      DBG("Failed to add audio element: " + audio_element_type);
    }
  }
}

void PluginProcessor::removeLastAudioElement() {
  if (iamfbr_) {
    auto status = iamfbr_->RemoveLastAudioElement();
    if (status.ok()) {
      DBG("Removed last audio element.");
    } else {
      DBG("Failed to remove last audio element.");
    }
  }
}

void PluginProcessor::removeAllAudioElements() {
  if (iamfbr_) {
    auto number_of_elements = iamfbr_->GetNumberOfAudioElements();
    for (size_t i = 0; i < number_of_elements; ++i) {
      auto status = iamfbr_->RemoveLastAudioElement();
      if (status.ok()) {
        DBG("Removed audio element: " + std::to_string(i));
      } else {
        DBG("Failed to remove audio element: " + std::to_string(i));
      }
    }
  }
}

void PluginProcessor::parameterChanged(const juce::String& parameterID,
                                       float newValue) {
  if (parameterID == "audio_element_type") {
    auto value =
        parameters.getParameter("audio_element_type")->getCurrentValueAsText();
    setAudioElementType(value.toStdString());
  }
}

juce::AudioProcessorValueTreeState::ParameterLayout
PluginProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  auto available_types = obr::GetAvailableAudioElementTypesAsStr();
  juce::StringArray stringArray;
  for (const auto& type : available_types) {
    stringArray.add(type);
  }

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID{"audio_element_type", 1}, "Audio Element Type",
      stringArray, 0));

  return layout;
}

void PluginProcessor::connectOSC(bool toBeConnected) {
  if (toBeConnected) {
    if (juce::OSCReceiver::connect(12345)) {
      DBG("Connected to UDP port 12345.");
    } else {
      DBG("Error: could not connect to UDP port 12345.");
    }
    juce::OSCReceiver::addListener(this);
  } else {
    juce::OSCReceiver::disconnect();
    juce::OSCReceiver::removeListener(this);
  }
}
