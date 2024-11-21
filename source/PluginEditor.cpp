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
  addAndMakeVisible(select_audio_element_button);
  select_audio_element_button.setButtonText("Select Audio Element Type");
  select_audio_element_button.onClick = [this] {
    // Get available audio element types.
    auto audio_element_types = iamfbr::GetAvailableAudioElementTypesAsStr();
    juce::PopupMenu add_audio_element_menu;
    for (size_t i = 0; i < audio_element_types.size(); ++i) {
      add_audio_element_menu.addItem((int)(i + 1), audio_element_types[i]);
    }

    add_audio_element_menu.showMenuAsync(
        juce::PopupMenu::Options(), [this](int result) {
          if (result == 0) {
            DBG("Selection dismissed.");
          } else {
            std::string selected_type =
                iamfbr::GetAvailableAudioElementTypesAsStr()
                    [static_cast<size_t>(result - 1)];

            // Add audio element to the renderer.
            auto value =
                processorRef.parameters.getParameter("audio_element_type")
                    ->getValueForText(selected_type);
            processorRef.parameters.getParameter("audio_element_type")
                ->setValueNotifyingHost(value);
          }
        });
  };

  // Add log window.
  logWindow.setMultiLine(true, false);
  logWindow.setReadOnly(true);
  logWindow.setCaretVisible(false);
  logWindow.setScrollbarsShown(true);
  logWindow.setFont(juce::Font(14.0f));
  addAndMakeVisible(logWindow);

  host_bus_width_too_small_label.setText(
      "Host bus width is too small for the number of required input/output channels.",
      juce::dontSendNotification);
  host_bus_width_too_small_label.setColour(juce::Label::textColourId,
                                           juce::Colours::red);
  addChildComponent(host_bus_width_too_small_label);

  // Add labels to the editor.
  addAndMakeVisible(iamfbr_number_of_audio_elements_label);
  addAndMakeVisible(iamfbr_sampling_rate_label);
  addAndMakeVisible(iamfbr_buffer_size_label);
  addAndMakeVisible(iamfbr_number_of_input_channels_label);
  addAndMakeVisible(iamfbr_number_of_output_channels_label);

  // Set size of the plugin window.
  setSize(800, 1000);

  // Start UI refresh timer.
  startTimer(20);
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
  int button_width = (getWidth() - 3 * margin) / 2;
  int button_height = 50;
  select_audio_element_button.setBounds(margin, 50 + margin, button_width,
                                        button_height);

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
  host_bus_width_too_small_label.setBounds(
      margin, margin * 2 + label_height * 5, getWidth() - 2 * margin,
      label_height);

  logWindow.setBounds(margin, margin * 2 + label_height * 6,
                      getWidth() - 2 * margin,
                      getHeight() - 3 * margin - label_height * 5);
}

void PluginEditor::timerCallback() {
  // This gets called by our timer and will update the UI
  // based on the current state of the processor / iamfbr.
  juce::String message =
      juce::String(processorRef.iamfbr_->GetAudioElementConfigLogMessage());
  logWindow.setText(message);
  logWindow.moveCaretToEnd();

  // Display info about iamfbr AudioElementConfig list.
  iamfbr_number_of_audio_elements_label.setText(
      "Number of audio elements: " +
          juce::String(processorRef.iamfbr_->GetNumberOfAudioElements()),
      juce::dontSendNotification);

  // Display info about iamfbr DSP state.
  iamfbr_buffer_size_label.setText(
      "Buffer size per channel: " +
          juce::String(processorRef.iamfbr_->GetBufferSizePerChannel()) +
          " samples",
      juce::dontSendNotification);
  iamfbr_sampling_rate_label.setText(
      "Sampling rate: " +
          juce::String(processorRef.iamfbr_->GetSamplingRate()) + " Hz",
      juce::dontSendNotification);
  iamfbr_number_of_input_channels_label.setText(
      "Number of input channels: " +
          juce::String(processorRef.iamfbr_->GetNumberOfInputChannels()),
      juce::dontSendNotification);
  iamfbr_number_of_output_channels_label.setText(
      "Number of output channels: " +
          juce::String(processorRef.iamfbr_->GetNumberOfOutputChannels()),
      juce::dontSendNotification);

  host_bus_width_too_small_label.setVisible(processorRef.getBusWidthTooSmall());
}
