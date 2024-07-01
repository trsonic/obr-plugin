#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

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

    // add the input type selection box to the editor
    addAndMakeVisible(inputTypeSelectComboBox);
    inputTypeSelectComboBox.addItem("Mono", 1);
    inputTypeSelectComboBox.addItem("Stereo", 2);
    inputTypeSelectComboBox.addItem("5.1", 3);
    inputTypeSelectComboBox.addItem("7.1.4", 4);
    inputTypeSelectComboBox.addItem("3OA", 5);
    inputTypeSelectComboBox.addItem("7OA", 6);
    inputTypeSelectComboBox.onChange = [this]() {
        DBG("selected " + juce::String(inputTypeSelectComboBox.getSelectedId()));
    };
    inputTypeSelectComboBox.setSelectedId(5, juce::sendNotification);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));

    inputTypeSelectComboBox.setBounds(20,20,100,25);

}
