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
//    inputTypeSelectComboBox.addItem("Mono", 1);
//    inputTypeSelectComboBox.addItem("Stereo", 2);
//    inputTypeSelectComboBox.addItem("5.1", 3);
//    inputTypeSelectComboBox.addItem("7.1.4", 4);
    inputTypeSelectComboBox.addItem("3OA", 1);
    inputTypeSelectComboBox.addItem("7OA", 2);
    inputTypeSelectComboBox.onChange = [this]() {
        DBG("selected " + juce::String(inputTypeSelectComboBox.getSelectedId()));
//        if(inputTypeSelectComboBox.getSelectedId() == 1)
//        {
//            // 3OA
//            processorRef.libiamfbr_->setInputType(0);
//        }
//        else if(inputTypeSelectComboBox.getSelectedId() == 2)
//        {
//            // 7OA
//            processorRef.libiamfbr_->setInputType(1);
//        }
    };

//    inputTypeSelectComboBox.setSelectedId(1, juce::sendNotification);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
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
    auto area = getLocalBounds();
    area.reduce(10,10);

    inputTypeSelectComboBox.setBounds(area.removeFromLeft(200).removeFromTop(25));

    inspectButton.setBounds(area.removeFromRight(200).removeFromBottom(25));
}
