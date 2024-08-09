#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);

  // Left side of the GUI (Input).
  addAndMakeVisible(input_type_selector);
  input_type_selector.addItem("Ambisonics", 1);
  input_type_selector.addItem("Channel-based", 2);
  input_type_selector.addItem("Individual sources", 3);
  input_type_selector.onChange = [this]() {
    switch (input_type_selector.getSelectedId()) {
      case 1:
        processorRef.plugin_state_.input_type = InputType::Ambisonics;
        break;
      case 2:
        processorRef.plugin_state_.input_type = InputType::Loudspeaker_feeds;
        break;
      case 3:
        processorRef.plugin_state_.input_type = InputType::Individual_sources;
        break;
      default:
        break;
    }
    setup_UI();
  };

  addAndMakeVisible(input_cb_label);
  input_cb_label.setText("Input Type", juce::dontSendNotification);
  input_cb_label.attachToComponent(&input_type_selector, true);

  addChildComponent(input_ambisonic_order_selector);
  input_ambisonic_order_selector.addItem("3OA", 1);
  input_ambisonic_order_selector.addItem("7OA", 2);
  input_ambisonic_order_selector.onChange = [this]() {
    switch (input_ambisonic_order_selector.getSelectedId()) {
      case 1:
        processorRef.plugin_state_.ambisonic_order = 3;
        break;
      case 2:
        processorRef.plugin_state_.ambisonic_order = 7;
        break;
      default:
        break;
    }
    setup_UI();
  };

  addChildComponent(input_ambisonic_order_label);
  input_ambisonic_order_label.setText("Ambisonic Order",
                                      juce::dontSendNotification);
  input_ambisonic_order_label.attachToComponent(&input_ambisonic_order_selector,
                                                true);

  addChildComponent(input_preset_selector);
  input_preset_selector.addItem("7.1.4", 1);
  input_preset_selector.addItem("5.1", 2);
  input_preset_selector.onChange = [this]() {
    // load input configuration preset
    processorRef.plugin_state_.selected_preset_id = input_preset_selector.getSelectedId();
    setup_UI();
  };

  addChildComponent(input_preset_select_label);
  input_preset_select_label.setText("Input Configuration",
                                    juce::dontSendNotification);
  input_preset_select_label.attachToComponent(&input_preset_selector, true);

  // Setup source list header labels.
  source_list_labels.add(new juce::Label());
  source_list_labels.getLast()->setText("Source ID",
                                        juce::dontSendNotification);
  source_list_labels.add(new juce::Label());
  source_list_labels.getLast()->setText("Gain", juce::dontSendNotification);
  source_list_labels.add(new juce::Label());
  source_list_labels.getLast()->setText("Azimuth", juce::dontSendNotification);
  source_list_labels.add(new juce::Label());
  source_list_labels.getLast()->setText("Elevation",
                                        juce::dontSendNotification);
  source_list_labels.add(new juce::Label());
  source_list_labels.getLast()->setText("Distance", juce::dontSendNotification);

  for (auto label : source_list_labels) {
    addChildComponent(label);
  }


  setSize(800, 600);

  // SETUP INITIAL SETTINGS
  processorRef.plugin_state_.input_type = InputType::Individual_sources;
  processorRef.plugin_state_.ambisonic_order = 7;
  processorRef.plugin_state_.selected_preset_id = 1;

  // setup individual sources
  processorRef.plugin_state_.sources.push_back({"source1", 1.0f, -30.0f, 0.0f, 1.0f});
  processorRef.plugin_state_.sources.push_back({"source2", 1.0f, 30.0f, 0.0f, 1.0f});
  processorRef.plugin_state_.sources.push_back({"source3", 1.0f, 0.0f, 0.0f, 1.0f});
  processorRef.plugin_state_.sources.push_back({"source4", 1.0f, 0.0f, 90.0f, 1.0f});
  setup_UI();
}

void PluginEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  auto area = getLocalBounds();

  g.setColour(juce::Colours::white);

  // Draw vertical, dashed line in the middle of GUI.
  g.setColour(juce::Colours::black);
  g.drawVerticalLine(getWidth() / 2, 0, getHeight());

  g.setFont(16.0f);
  //    auto helloWorld = juce::String ("Hello from ") +
  //    PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " +
  //    CMAKE_BUILD_TYPE; g.drawText (helloWorld, area.removeFromTop (150),
  //    juce::Justification::centred, false);
}

void PluginEditor::resized() {
  auto area = getLocalBounds();
  //  area.reduce(10, 10);

  int margin = 10;
  int cb_width = 200;
  int cb_height = 25;

  input_type_selector.setSize(cb_width, cb_height);
  input_type_selector.setCentrePosition(
      area.getWidth() / 2 - (cb_width / 2 + margin), cb_height / 2 + margin);

  input_ambisonic_order_selector.setSize(cb_width, cb_height);
  input_ambisonic_order_selector.setCentrePosition(
      area.getWidth() / 2 - (cb_width / 2 + margin), 60);

  input_preset_selector.setSize(cb_width, cb_height);
  input_preset_selector.setCentrePosition(
      area.getWidth() / 2 - (cb_width / 2 + margin), 60);


  // individual source stuff
  for (int i = 0; i < source_list_labels.size(); i++) {
    source_list_labels[i]->setBounds(margin + 75 * i, 50, 100, 25);
  }

  int value_width = 100;
  int value_height = 25;
}

void PluginEditor::setup_UI() {
  // trigger DSP update
  processorRef.update_DSP();

  // clear UI
  input_ambisonic_order_selector.setVisible(false);
  input_ambisonic_order_label.setVisible(false);
  input_preset_selector.setVisible(false);
  input_preset_select_label.setVisible(false);

  for (auto& label : source_list_labels) {
    label->setVisible(false);
  }

  label_matrix_.clear();

  switch (processorRef.plugin_state_.input_type) {
    case InputType::Ambisonics:
      input_type_selector.setSelectedId(1, juce::dontSendNotification);

      // show the selector for the ambisonic order
      input_ambisonic_order_selector.setVisible(true);
      input_ambisonic_order_label.setVisible(true);

      switch (processorRef.plugin_state_.ambisonic_order) {
        case 3:
          input_ambisonic_order_selector.setSelectedId(
              1, juce::dontSendNotification);
          break;
        case 7:
          input_ambisonic_order_selector.setSelectedId(
              2, juce::dontSendNotification);
          break;
        default:
          break;
      }

      break;
    case InputType::Loudspeaker_feeds:
      input_type_selector.setSelectedId(2, juce::dontSendNotification);

      // show the selector for the input presets
      input_preset_selector.setVisible(true);
      input_preset_select_label.setVisible(true);

      switch (processorRef.plugin_state_.selected_preset_id) {
        case 1:
          input_preset_selector.setSelectedId(1, juce::dontSendNotification);
          break;
        case 2:
          input_preset_selector.setSelectedId(2, juce::dontSendNotification);
          break;
        default:
          break;
      }

      break;

    case InputType::Individual_sources:
      input_type_selector.setSelectedId(3, juce::dontSendNotification);

      // show the source list
      for (auto& label : source_list_labels) {
        label->setVisible(true);
      }

      // setup the value table
      for (auto source : processorRef.plugin_state_.sources) {
        label_matrix_.add(new juce::OwnedArray<juce::Label>());
        for (int i = 0; i < source_list_labels.size(); i++) {
          label_matrix_.getLast()->add(new juce::Label());
          label_matrix_.getLast()->getLast()->setText(
              "Test", juce::dontSendNotification);

          // pull value from source and set label properties
          switch (i) {
            case 0:
              label_matrix_.getLast()->getLast()->setText(
                  source.source_id, juce::dontSendNotification);
              break;
            case 1:
              label_matrix_.getLast()->getLast()->setText(
                  juce::String(source.gain), juce::dontSendNotification);
              break;
            case 2:
              label_matrix_.getLast()->getLast()->setText(
                  juce::String(source.azimuth), juce::dontSendNotification);
              break;
            case 3:
              label_matrix_.getLast()->getLast()->setText(
                  juce::String(source.elevation), juce::dontSendNotification);
              break;
            case 4:
              label_matrix_.getLast()->getLast()->setText(
                  juce::String(source.distance), juce::dontSendNotification);
              break;
            default:
              break;
          }

          label_matrix_.getLast()->getLast()->setBounds(
              source_list_labels[i]->getX(),
              source_list_labels[i]->getY() + 25 +
                  25 * (label_matrix_.size() - 1),
              100, 25);
          addAndMakeVisible(label_matrix_.getLast()->getLast());
        }
      }

      break;

    default:
      break;
  }
}
