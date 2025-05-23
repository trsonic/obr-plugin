#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor, public juce::Timer {
 public:
  explicit PluginEditor(PluginProcessor&);
  ~PluginEditor() override = default;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  void timerCallback() override;

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PluginProcessor& processorRef;

  juce::TextButton select_audio_element_button;

  juce::ToggleButton head_tracking_enabled_toggle_button;
  juce::Label head_orientation_label;

  juce::TextEditor logWindow;

  juce::Label iamfbr_number_of_audio_elements_label;
  juce::Label iamfbr_sampling_rate_label, iamfbr_buffer_size_label;
  juce::Label iamfbr_number_of_input_channels_label,
      iamfbr_number_of_output_channels_label;
  juce::Label host_bus_width_too_small_label;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
