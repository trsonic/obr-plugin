#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible (inputTypeSelectComboBox);
    inputTypeSelectComboBox.setText("Select Input Type");
    inputTypeSelectComboBox.addItem ("Ambisonics", 1);
    inputTypeSelectComboBox.addItem ("Channel-based", 2);
    inputTypeSelectComboBox.addItem ("Single-sources", 2);
    inputTypeSelectComboBox.onChange = [this]() {
        DBG ("Selected Input: " + juce::String (inputTypeSelectComboBox.getSelectedId()) + inputTypeSelectComboBox.getText());
    };

    addAndMakeVisible(outputTypeSelectComboBox);
    outputTypeSelectComboBox.setText("Select Output Type");
    outputTypeSelectComboBox.addItem ("Binaural", 1);
    outputTypeSelectComboBox.addItem ("Ambisonics", 2);
    outputTypeSelectComboBox.onChange = [this]() {
        DBG ("Selected Output: " + juce::String (outputTypeSelectComboBox.getSelectedId()) + outputTypeSelectComboBox.getText());
    };

    // set the default configuration
    inputTypeSelectComboBox.setSelectedId(1, juce::sendNotification);
    outputTypeSelectComboBox.setSelectedId(1, juce::sendNotification);

    addAndMakeVisible (inspectButton);

    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };


    setSize (800, 600);
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();

    g.setColour (juce::Colours::white);

    // Draw vertical, dashed line in the middle of GUI.
    g.setColour (juce::Colours::black);
    g.drawVerticalLine (getWidth() / 2, 0, getHeight());

    g.setFont (16.0f);
    //    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //    g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    area.reduce (10, 10);

    inputTypeSelectComboBox.setBounds (area.removeFromLeft (200).removeFromTop (25));
    outputTypeSelectComboBox.setBounds (area.removeFromRight (200).removeFromTop (25));

//    inspectButton.setBounds (area.removeFromBottom (25));
}
