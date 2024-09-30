#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);

  // Modify look and feel.
  getLookAndFeel().setDefaultSansSerifTypefaceName("Courier New");
  getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId,
                             juce::Colours::black);
  getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::white);
  getLookAndFeel().setColour(juce::TextButton::buttonColourId,
                             juce::Colours::black);
  getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId,
                             juce::Colours::blueviolet.darker());
  getLookAndFeel().setColour(juce::TextEditor::backgroundColourId,
                             juce::Colours::black);
  getLookAndFeel().setColour(juce::TextEditor::outlineColourId,
                             juce::Colours::transparentBlack);
  getLookAndFeel().setColour(juce::ScrollBar::thumbColourId,
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

  // Add log window.
  logWindow.setMultiLine(true, false);
  logWindow.setReadOnly(true);
  logWindow.setCaretVisible(false);
  //  logWindow.setInterceptsMouseClicks(false, false);
  logWindow.setScrollbarsShown(true);
  logWindow.setFont(juce::Font(14.0f));
  addAndMakeVisible(logWindow);

  // Add labels to the editor.
  addAndMakeVisible(iamfbr_number_of_audio_elements_label);
  addAndMakeVisible(iamfbr_sampling_rate_label);
  addAndMakeVisible(iamfbr_buffer_size_label);
  addAndMakeVisible(iamfbr_number_of_input_channels_label);
  addAndMakeVisible(iamfbr_number_of_output_channels_label);

  // Set size of the plugin window.
  setSize(850, 1000);

  // Start UI refresh timer.
  startTimer(100);
}

void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setFont(24.0f);
  g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
  g.drawText("IAMF Binaural Renderer", 0, 0, getWidth() / 2, 50,
             juce::Justification::centred);
}

void PluginEditor::resized() {
  int margin = 10;
  int button_width = (getWidth() - 5 * margin) / 4;
  int button_height = 50;
  add_audio_element_button.setBounds(margin, 50 + margin, button_width,
                                     button_height);
  remove_audio_element_button.setBounds(
      margin + button_width + margin, 50 + margin, button_width, button_height);

  int label_width = 400;
  int label_height = 20;
  int label_x = getWidth() / 2 + margin;
  iamfbr_buffer_size_label.setBounds(label_x, margin, label_width,
                                     label_height);
  iamfbr_sampling_rate_label.setBounds(label_x, margin + label_height * 1,
                                       label_width, label_height);
  iamfbr_number_of_input_channels_label.setBounds(
      label_x, margin + label_height * 2, label_width, label_height);
  iamfbr_number_of_output_channels_label.setBounds(
      label_x, margin + label_height * 3, label_width, label_height);
  iamfbr_number_of_audio_elements_label.setBounds(
      label_x, margin + label_height * 4, label_width, label_height);

  logWindow.setBounds(margin, margin * 2 + label_height * 5,
                      getWidth() - 2 * margin,
                      getHeight() - 3 * margin - label_height * 5);
}

void PluginEditor::timerCallback() {
  // This gets called by our timer and will update the UI
  // based on the current state of the processor / iamfbr.
  juce::String message =
      juce::String(processorRef.iamfbr_->get_audio_element_list_log_message());
  logWindow.setText(message);
  logWindow.moveCaretToEnd();

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
