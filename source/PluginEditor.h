#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"
#include "extras.h"
#include "melatonin_inspector/melatonin_inspector.h"

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

  juce::Label input_cb_label, input_ambisonic_order_label,
      input_preset_select_label;
  juce::ComboBox input_type_selector, input_ambisonic_order_selector,
      input_preset_selector;

  // source list header labels
  juce::OwnedArray<juce::Label> source_list_labels;

  // source property value matrix
  juce::OwnedArray<juce::OwnedArray<juce::Label>> label_matrix_;

  std::unique_ptr<melatonin::Inspector> inspector;
  juce::TextButton inspectButton{"Inspect the UI"};

  void setup_UI();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
