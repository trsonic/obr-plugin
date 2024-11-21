#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "src/renderer/iamfbr_impl.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor,
                        public juce::AudioProcessorValueTreeState::Listener {
 public:
  PluginProcessor();
  ~PluginProcessor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

  std::unique_ptr<iamfbr::IamfbrImpl> iamfbr_;

  void setAudioElementType(std::string audio_element_type);
  void removeLastAudioElement();
  void removeAllAudioElements();

  void parameterChanged(const juce::String& parameterID,
                        float newValue) override;
  juce::AudioProcessorValueTreeState parameters;

  bool getBusWidthTooSmall() const { return bus_width_too_small; }

 private:
  juce::UndoManager undo_manager;
  bool bus_width_too_small = false;
  static juce::AudioProcessorValueTreeState::ParameterLayout
  createParameterLayout();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
