#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);

  // Modify look and feel.
  getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId,
                             juce::Colours::black);
  getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::white);
  getLookAndFeel().setColour(juce::TextButton::buttonColourId,
                             juce::Colours::blueviolet);
  getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId,
                             juce::Colours::blueviolet.darker());

  // Set up 'Add Audio Element' button.
  addAndMakeVisible(add_audio_element_button);
  add_audio_element_button.setButtonText("Add Audio Element");
  add_audio_element_button.onClick = [this] {
    enum {
      AmbisonicsBaseID = 100,
      LoudspeakerLayoutsBaseID = 200,
      ObjectsBaseID = 300,
    };

    std::vector<int> base_ids = {AmbisonicsBaseID, LoudspeakerLayoutsBaseID,
                                 ObjectsBaseID};

    juce::PopupMenu add_audio_element_menu;
    for (size_t i = 0; i < base_ids.size(); ++i) {
      int base_id = base_ids[i];

      juce::PopupMenu sub_menu;
      for (const auto& subtype :
           processorRef.iamfbr_->get_audio_element_subtypes(
               processorRef.iamfbr_->get_audio_element_types()[i])) {
        int id = base_id + sub_menu.getNumItems();
        sub_menu.addItem(id, subtype);
      }

      add_audio_element_menu.addSubMenu(
          processorRef.iamfbr_->get_audio_element_types()[i], sub_menu);
    }

    add_audio_element_menu.showMenuAsync(
        juce::PopupMenu::Options(), [this](int result) {
          if (result == 0) {
            DBG("Selection dismissed.");
          } else if (result >= AmbisonicsBaseID &&
                     result < LoudspeakerLayoutsBaseID) {
            // Add Ambisonic scene audio element to the renderer.
            auto idx = static_cast<size_t>(result - AmbisonicsBaseID);
            processorRef.iamfbr_->add_audio_element(
                processorRef.iamfbr_->get_audio_element_types()[0],
                processorRef.iamfbr_->get_audio_element_subtypes(
                    processorRef.iamfbr_->get_audio_element_types()[0])[idx]);
          } else if (result >= LoudspeakerLayoutsBaseID &&
                     result < ObjectsBaseID) {
            // Add loudspeaker layout audio element to the renderer.
            auto idx = static_cast<size_t>(result - LoudspeakerLayoutsBaseID);
            processorRef.iamfbr_->add_audio_element(
                processorRef.iamfbr_->get_audio_element_types()[1],
                processorRef.iamfbr_->get_audio_element_subtypes(
                    processorRef.iamfbr_->get_audio_element_types()[1])[idx]);
          } else if (result >= ObjectsBaseID) {
            // Add object audio element to the renderer.
            auto idx = static_cast<size_t>(result - ObjectsBaseID);
            processorRef.iamfbr_->add_audio_element(
                processorRef.iamfbr_->get_audio_element_types()[2],
                processorRef.iamfbr_->get_audio_element_subtypes(
                    processorRef.iamfbr_->get_audio_element_types()[2])[idx]);
          }
        });
  };

  addAndMakeVisible(remove_audio_element_button);
  remove_audio_element_button.setButtonText("Remove Audio Element");
  remove_audio_element_button.onClick = [this] {
    processorRef.iamfbr_->remove_audio_element();
  };

  // Add labels to the editor.
  addAndMakeVisible(iamfbr_number_of_audio_elements_label);
  addAndMakeVisible(iamfbr_sampling_rate_label);
  addAndMakeVisible(iamfbr_buffer_size_label);
  addAndMakeVisible(iamfbr_number_of_input_channels_label);
  addAndMakeVisible(iamfbr_number_of_output_channels_label);

  // Set size of the plugin window.
  setSize(400, 600);

  // Start UI refresh timer.
  startTimer(100);
}

void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setFont(24.0f);
  g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
  g.drawText("IAMF Binaural Renderer", 0, 0, getWidth(), 50,
             juce::Justification::centred);
}

void PluginEditor::resized() {
  int margin = 10;
  int label_width = 250;
  int label_height = 20;
  int button_width = (getWidth() - 3 * margin) / 2;

  add_audio_element_button.setBounds(margin, 50 + margin, button_width, 50);

  remove_audio_element_button.setBounds(margin + button_width + margin, 50 + margin, button_width,
                                        50);

  iamfbr_number_of_audio_elements_label.setBounds(margin, 50 + 2 * margin + 50,
                                                  label_width, label_height);

  // iamfbr state labels
  //  for (int i = 0; i < source_list_labels.size(); i++) {
  //    source_list_labels[i]->setBounds(margin + 75 * i, 50, 100, 25);
  //  }

  iamfbr_buffer_size_label.setBounds(margin,
                                     getHeight() - margin - 4 * label_height,
                                     label_width, label_height);
  iamfbr_sampling_rate_label.setBounds(margin,
                                       getHeight() - margin - 3 * label_height,
                                       label_width, label_height);
  iamfbr_number_of_input_channels_label.setBounds(
      margin, getHeight() - margin - 2 * label_height, label_width,
      label_height);
  iamfbr_number_of_output_channels_label.setBounds(
      margin, getHeight() - margin - label_height, label_width, label_height);
}

void PluginEditor::timerCallback() {
  // This gets called by our timer and will update the UI
  // based on the current state of the processor / iamfbr.

  // Display info about iamfbr AudioElementConfig list.
  iamfbr_number_of_audio_elements_label.setText(
      "Number of audio elements: " +
          juce::String(processorRef.iamfbr_->get_number_of_audio_elements()),
      juce::dontSendNotification);

  // Disable 'Remove Audio Element' button if there are no audio elements.
  remove_audio_element_button.setVisible(
      processorRef.iamfbr_->get_number_of_audio_elements() > 0);

  // Display info about iamfbr DSP state.
  iamfbr_buffer_size_label.setText(
      "Buffer size per channel: " +
          juce::String(processorRef.iamfbr_->get_buffer_size_per_channel()) +
          " samples",
      juce::dontSendNotification);
  iamfbr_sampling_rate_label.setText(
      "Sampling rate: " +
          juce::String(processorRef.iamfbr_->get_sampling_rate()) + " Hz",
      juce::dontSendNotification);
  iamfbr_number_of_input_channels_label.setText(
      "Number of input channels: " +
          juce::String(processorRef.iamfbr_->get_number_of_input_channels()),
      juce::dontSendNotification);
  iamfbr_number_of_output_channels_label.setText(
      "Number of output channels: " +
          juce::String(processorRef.iamfbr_->get_number_of_output_channels()),
      juce::dontSendNotification);

  ;
}
